// conditional expression is constant, re: if (formattedForSpreadsheet)
#pragma warning(disable:4127)


/*
* Include files
*/
#include <math.h>                    // cos, sin, etc
#include <stdio.h>                   // printf
#include <memory.h>                  // memcpy
#include <time.h>

#include <windows.h>                 // SetPriorityClass, GetCurrentProcess, Sleep, ...

#include "telemapp.h"
#include "..\common\run_data.h"

#include "track_path.h"
#include "track_guide.h"
#include "..\common\math_util.h"

#define M_PI       3.14159265358979323846

using namespace iRacingTelem;

static bool    ctrl_c_pressed = false;

//
// globals
//

FILE * g_output_file = NULL;
unsigned int g_sequence;

double g_time;
double g_x_position;	// increases east
double g_y_position;	// increases north
double g_z_position;	// increases up
double g_x_velocity;	// increases east
double g_y_velocity;	// increases north
double g_z_velocity;	// increases up
double g_lateral_acceleration;
double g_longitudinal_acceleration;
double g_speed;
double g_rpm;
double g_slip;
double g_throttle;
double g_brake;
double g_clutch;
double g_steer;
double g_yaw_rate;
double g_track_percentage;
double g_gear;
double g_output_sample_time;
double g_print_sample_time;

bool g_print_in_real_time = false;
bool g_use_track_path = false;
bool g_use_track_guide = true;

BOOL WINAPI ctrlHandler(
						DWORD        ctrlType)
{
	// Just exit, regardless of the type of control signal
	(void) ctrlType;
	ctrl_c_pressed    = true;
	return TRUE;
}

static double normalize_angle(double in)
{
	while(in > M_PI) in -= 2*M_PI;
	while(in <= -M_PI) in += 2*M_PI;

	return in;
}

static unsigned int normal_to_analog(double in)
{
	// map 0:1 to 0:65535
	return (unsigned int) (in * 65535);
}

static unsigned int gear_to_analog(int in)
{
	return ((in + 1) * 1000);
}

static unsigned int angle_to_analog(double in)
{
	// map -PI:PI to 0:1
	in = (in + M_PI) / (2.0f * M_PI);
	if (in < 0.0f) in = 0.0f;
	if (in > 1.0f) in = 1.0f;

	return normal_to_analog(in);
}

// These two functions project the x,y coordinate system onto
// a earth latitude and longitude (represented in minutes north
// and east).
//
// We use a simple flat projeection here (because the Earth's
// radius is so large). The trick is to realise that the distance
// per minute of longitude varies according to latitude whereas
// the distance per minute of latitude is pretty constant (ignoring
// the bulging effect around the equator).
//
// With the magic of Google, I determined the following for 
// 45 degrees N lat
//
// 1 minute of latitude equals 1851.8184 meters
// 1 minute of longitude equals 1314.2976 meters (only at 45 degres N lat!)
//
// Using these numbers, I'll sit the origin at the following arbitrary]
// coordinates: 100 W by 45 N (approx middle of North America)
//

static double y2lat(double y)
{
	return 45.0f + (y / 111071.04);	// flip sign to make map work right
//	return  (y / 110574);
}

static double x2long(double x)
{
	return -100.00 + (x / 78857.856);   // only valid at 45 deg N latitude
//	return (x / 111320);
}

static void matrix33FromEuler(
	float		m[3][3],
	float		yaw,
	float		pitch,
	float		roll)
{
	float cy = (float) cos(yaw);
	float cp = (float) cos(pitch);
	float cr = (float) cos(roll);
	float sy = (float) sin(yaw);
	float sp = (float) sin(pitch);
	float sr = (float) sin(roll);
	
	m[0][0] = cp*cy;
	m[0][1] = cy*sp*sr - cr*sy;
	m[0][2] = cr*cy*sp + sr*sy;
	m[1][0] = cp*sy;
	m[1][1] = cr*cy + sp*sr*sy;
	m[1][2] = cr*sp*sy - cy*sr;
	m[2][0] = -sp;
	m[2][1] = cp*sr;
	m[2][2] = cp*cr;
}

// Multiply vector by matrix transpose
static void matrix33MulTVector3(
	float		r[3],
	const float	m[3][3],
	const float	v[3])
{
	float		tmp[3];
	float		*o;

	o = (r == v) ? tmp : r;

	o[0] = (m[0][0]*v[0]) + (m[1][0]*v[1]) + (m[2][0]*v[2]);
	o[1] = (m[0][1]*v[0]) + (m[1][1]*v[1]) + (m[2][1]*v[2]);
	o[2] = (m[0][2]*v[0]) + (m[1][2]*v[1]) + (m[2][2]*v[2]);

	if (o != r) memcpy(r, o, sizeof(tmp));
}

static void vector3Add(
	float		r[3],
	const float	v0[3],
	const float	v1[3])
{
	r[0] = v0[0] + v1[0];
	r[1] = v0[1] + v1[1];
	r[2] = v0[2] + v1[2];
}

static void vector3Sub(
	float		r[3],
	const float	v0[3],
	const float	v1[3])
{
	r[0] = v0[0] - v1[0];
	r[1] = v0[1] - v1[1];
	r[2] = v0[2] - v1[2];
}

static void vector3Scale(
	float		r[3],
	float		s,
	const float	v[3])
{
	r[0] = v[0] * s;
	r[1] = v[1] * s;
	r[2] = v[2] * s;
}

static float vector3Magnitude(float v[])
{
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static void printHeading(FILE *fp)
{
	fprintf(fp, "header goes here\n");
}

void compute_accelerations(const SampleHeader * sh, ChassisData *cd, double * out_longitudinal_acceleration, double * out_lateral_acceleration)
{
	static unsigned int	sequence = static_cast<unsigned int>(-1);
	static SampleHeader	sh0;
	static ChassisData	cd0;
	float				ca[3], dt;

	if (sh)
	{
		// Compute lateral, longitudinal, and normal G loading
		// at the center of mass of the chassis.

		// We do so by computing the change in velocity from one
		// sample to the next, scaling by 1/change in time, adding
		// in gravity, and transforming this into the car's
		// coordinate system.

		// There will be discontinuities in the sample data
		// (as the driver gets in/out of the car, resets the car,
		// gets towed, etc).  We need to be careful not to attempt
		// to compute G loading from samples on either side of such
		// a discontinuity.
		if (sh->sequence != sequence ||
			(dt = (float)(sh->time - sh0.time)) <= 0.0f ||
			dt > 0.125f)
		{
			// When the sample header sequence number changes, then
			// this is the first sample since the car was dropped into
			// the world, so we have no prior sample with which to
			// compare.
			// If a data overrun occurred, then we may have missed
			// some samples and so the current and most recent
			// samples could be far apart in time.  Computing
			// G loading the way we do would be terribly inaccurate
			// in such a case.
			ca[0]		= ca[1] = ca[2] = 0.0f;

			// Squirrel away current sample and header so we
			// can compute chassis acceleration at next sample.
			sh0			= *sh;
			cd0			= *cd;
			sequence	= sh->sequence;
		}
		else
		{
			static const float	one_g	= 9.80665f;
			static const float	wg[3]	= { 0.0f, 0.0f, one_g }; // m/sec^2
			float				wa[3];
			float				m[3][3];

			// Compute acceleration (change in velocity per time) of
			// chassis in world frame from change in velocity over
			// this and prior sample.
			vector3Sub(wa, cd->v, cd0.v);			// m/sec
			vector3Scale(wa, 1.0f / dt, wa);		// m/sec^2

			// Add in gravity.
			vector3Add(wa, wa, wg);					// m/sec^2

			// Form 3x3 rotation matrix from car relative
			// coordinates to world coordinates from the
			// yaw, pitch and roll of the chassis.
			matrix33FromEuler(m, cd->q[0], cd->q[1], cd->q[2]);

			// Rotate chassis acceleration vector from world frame
			// into car's frame (note that to do this we're
			// multiplying by the matrix transpose).
			matrix33MulTVector3(ca, m, wa);

			// Scale to convert from m/sec^2 to G's.
			vector3Scale(ca, 1.0f / one_g, ca);		// G's

			// Squirrel away current sample and header so we
			// can compute chassis acceleration at next sample.
			sh0 = *sh;
			cd0 = *cd;
		}
	}
	else
	{
		ca[0] = 0.0f;
		ca[1] = 0.0f;
	}

	// The car's reference frame has +x forwards, +y to the
	// left, and +z up.  So longitudinal is ca[0], lateral
	// is ca[1], and normal is ca[2].
//	printf("%c%f%c%f%c%f",
//			S, ca[0], S, ca[1], S, ca[2]);

	*out_longitudinal_acceleration = ca[0];
	*out_lateral_acceleration = ca[1];

}

double normal_wrap_lerp(double a, double b, double lerp)
{
	if (a > b)
	{
		double result = scalar_lerp(a, b + 1.0f, lerp);
		if (result >= 1.0f) result -= 1.0f;
		return result;
	}
	else
	{
		return scalar_lerp(a, b, lerp);
	}
}

/*
***************************************************************************
***************************************************************************
*/
int main(int argc, char *argv[])
{
	argc; // unused
	argv; // unused

	SetConsoleCtrlHandler(ctrlHandler, TRUE);
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	unsigned int    ses = 0;	  // Session; used to name files
	char track_path[128];
	e_car car = e_car_unknown;
	e_track track = e_track_unknown;
    struct tm timeinfo = { 0 };
	strcpy_s(track_path, sizeof(track_path), "unknown");
	
	TrackPath		trackPath;
	TrackGuide		trackGuide;

	c_run run;

	while (AppBegin("iRacing.com Simulator") && !ctrl_c_pressed)
	{
		printf("Waiting to connect to iRacing Simulator.\n");

		while(!AppCheckIfSimActiveQ() && !ctrl_c_pressed)
		{
			Sleep(1000);
		}
		ses++;		// increment session number

		if (!ctrl_c_pressed)
		{
			const eSimDataType desired[] =
			{
				kSampleHeader,
				kChassisData,
				kDrivelineData,
				kDriverInput,
				kCurrentWeekend,
				kCarInfo
			};

			(void) AppRequestDataItems(sizeof(desired)/sizeof(desired[0]), desired);
			(void) AppRequestDataAtPhysicsRate(true);
			(void) AppEnableSampling(true);

			const int timeOutMs = 100;
			eSimDataType newStateData;
			bool newSample;

			while((newSample = AppWaitForNewSample(&newStateData, timeOutMs)) == true ||
				newStateData != kNoStateInfo ||
				AppCheckIfSimActiveQ())
			{
				if (ctrl_c_pressed) break;

				if (newStateData != kNoStateInfo)
				{
					if (newStateData == kCurrentWeekend) 
					{
						// assign the track data
						CurrentWeekend *wk = (CurrentWeekend *)AppGetSimData(kCurrentWeekend);

						printf("at track '%s'\n",  wk->track);

						if (strcmp(wk->track, "sonoma\\long")==0)
						{
							track = e_track_sonoma_long;
						}
						else
						{
							track = e_track_unknown;

							printf("Unrecognized track\n");
						}

						strcpy_s(track_path, sizeof(track_path), wk->track);
						for (char *cp; (cp = strchr(track_path, '\\')) != NULL ; )
							*cp = '_';
					}
					else if(newStateData == kCarInfo)
					{
						CarInfo * carInfo = (CarInfo *) AppGetSimData(kCarInfo);

						car = e_car_unknown;

						printf("getting in car '%s'\n", carInfo->carPath);

						if (strcmp(carInfo->carPath, "rt2000")==0)
						{
							car = e_car_rt2000;
						}
						else
						{
							car = e_car_unknown;

							printf("Unrecognized car\n");
						}
						
					}
				}
				else if (newSample)
				{
					SampleHeader *sample_header = (SampleHeader *) AppGetSimData(kSampleHeader);
					ChassisData  *chassis_data = (ChassisData *) AppGetSimData(kChassisData);
					DrivelineData *driveline_data = (DrivelineData *) AppGetSimData(kDrivelineData);
					DriverInput   *driver_input = (DriverInput *) AppGetSimData(kDriverInput);

					if (g_output_file==NULL || sample_header->sequence != g_sequence)
					{
						//
						// close any existing log files
						//

						if (g_use_track_path)
						{
							trackPath.Close();
						}

						if (g_use_track_guide)
						{
							trackGuide.Stop();
						}

						if (g_output_file != NULL) 
							fclose(g_output_file);

						if (run.is_open()) {
//							run.trim();
//							run.offset();
							run.write_to_file(track_path, timeinfo.tm_year - 100, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
							run.close();
						}

						//
						// Capture start time for new log file
						//

						time_t rawtime;
						char timebuf[64];

						time(&rawtime);
						localtime_s(&timeinfo, &rawtime);

						strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", &timeinfo);

						// Open new file for writing
						char filename[256];
						sprintf_s(filename, sizeof(filename), "%s_%s_%d_%d.vbo", timebuf, track_path, ses, sample_header->sequence);
						
						fopen_s(&g_output_file, filename, "w");

						printHeading(g_output_file);

						// open marker file

						if (g_use_track_path)
						{
							trackPath.Open();
						}

						if (g_use_track_guide)
						{
							trackGuide.Start();
						}

						// Reset session variables
						g_sequence = sample_header->sequence;
						g_time = sample_header->time;
						g_x_position = 0.0f;
						g_y_position = 0.0f;
						g_z_position = 0.0f;
						g_x_velocity = 0.0f;
						g_y_velocity = 0.0f;
						g_z_velocity = 0.0f;

						// wait one second before emitting samples
						g_output_sample_time = (double) ((int) (1.0f + g_time));
						g_print_sample_time = g_output_sample_time;

						printf("New Session; log file %s\n", filename);

						run.open();
						run.set_car(car);
						run.set_track(track);
					}

					double sample_time = sample_header->time;
					double speed      = vector3Magnitude(chassis_data->v);

					double rpm      = (driveline_data == NULL) ? 0.0 : driveline_data->engRPM;
					double throttle = (driver_input == NULL) ? 0.0 : driver_input->throttle;
					double brake    = (driver_input == NULL) ? 0.0 : driver_input->brake;
					double clutch    = (driver_input == NULL) ? 0.0 : driver_input->clutch;
					double steer    = (driver_input == NULL) ? 0.0 : driver_input->steer;
					int gear     = (driver_input == NULL) ? 0 : driver_input->gear;

					double longitudinal_acceleration, lateral_acceleration;
					compute_accelerations(sample_header, chassis_data, &longitudinal_acceleration, &lateral_acceleration);

					double heading = 0.0f;
					double dot= 0.0f;
					double slip= 0.0f;

					if (speed > 1.0f)
					{
						heading = chassis_data->q[0];
						dot = atan2(chassis_data->v[1], chassis_data->v[0]);	// direction of travel in radians
						slip = normalize_angle(heading - dot);
					}

					double yaw_rate = normalize_angle(chassis_data->w[2]);
					double track_percentage = chassis_data->trkPct;

					double delta_time = sample_time - g_time;

					double x_velocity = chassis_data->v[0];
					double y_velocity = chassis_data->v[1];
					double z_velocity = chassis_data->v[2];

					double x_acceleration = (x_velocity - g_x_velocity);
					double y_acceleration = (y_velocity - g_y_velocity);
					double z_acceleration = (z_velocity - g_z_velocity);

					double x_position = g_x_position + (x_velocity * delta_time) + (0.5 * (x_acceleration * delta_time));
					double y_position = g_y_position + (y_velocity * delta_time) + (0.5 * (y_acceleration * delta_time));
					double z_position = g_z_position + (z_velocity * delta_time) + (0.5 * (z_acceleration * delta_time));

					if (g_use_track_path)
					{
						double x_offset, y_offset;
						trackPath.Sample(track_percentage, x_position, y_position, z_position, &x_offset, &y_offset);
						x_position += x_offset;
						y_position += y_offset;
					}

					if (g_use_track_guide)
					{
						double x_offset, y_offset;
						trackGuide.Sample(track_percentage, x_position, y_position, z_position, speed, &x_offset, &y_offset);
						x_position += x_offset;
						y_position += y_offset;
					}

					if (g_print_in_real_time && sample_time >= g_print_sample_time)
					{
						double delta_x_velocity = (x_velocity * delta_time);
						double delta_x_acceleration = (0.5 * (x_acceleration * delta_time));
						double delta_y_velocity = (y_velocity * delta_time);
						double delta_y_acceleration = (0.5 * (y_acceleration * delta_time));

						printf("(%f,%f) (%f,%f)\n",
							delta_x_velocity, delta_y_velocity, delta_x_acceleration, delta_y_acceleration);

						g_print_sample_time = sample_time + 1.5f;
					}

					if (sample_time >= g_output_sample_time)
					{
						if (g_time <= g_output_sample_time && delta_time > 0.0f)
						{
							c_run_sample * run_sample = run.new_sample();

							double lerp = (g_output_sample_time - g_time) / delta_time;

							run_sample->m_time = g_output_sample_time;
							run_sample->m_lateral_acceleration = scalar_lerp(g_lateral_acceleration, lateral_acceleration, lerp);
							run_sample->m_longitudinal_acceleration = scalar_lerp(g_longitudinal_acceleration, longitudinal_acceleration, lerp);
							run_sample->m_longitude_position = x2long(scalar_lerp(g_x_position, x_position, lerp));
							run_sample->m_latitude_position = y2lat(scalar_lerp(g_y_position, y_position, lerp));
							run_sample->m_speed = scalar_lerp(g_speed, speed, lerp);
							run_sample->m_rpm = scalar_lerp(g_rpm, rpm, lerp);
							run_sample->m_slip = angle_to_analog(scalar_lerp(g_slip, slip, lerp));
							run_sample->m_throttle = normal_to_analog(scalar_lerp(g_throttle, throttle, lerp));
							run_sample->m_brake = normal_to_analog(scalar_lerp(g_brake, brake, lerp));
							run_sample->m_clutch = normal_to_analog(scalar_lerp(g_clutch, clutch, lerp));
							run_sample->m_steer = angle_to_analog(scalar_lerp(g_steer, steer, lerp));
							run_sample->m_yaw = angle_to_analog(scalar_lerp(g_yaw_rate, yaw_rate, lerp));
							run_sample->m_track_percentage =normal_to_analog(normal_wrap_lerp(g_track_percentage, track_percentage, lerp));
							run_sample->m_gear = gear_to_analog(gear);

							fprintf(g_output_file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%u,%u,%u,%u,%u,%u,%u,%u\n",
								run_sample->m_time,
								run_sample->m_lateral_acceleration,
								run_sample->m_longitudinal_acceleration,
								run_sample->m_longitude_position,
								run_sample->m_latitude_position,
								1000.0f,
								run_sample->m_speed, 
								0.1f,
								run_sample->m_rpm,
								run_sample->m_slip,
								run_sample->m_throttle,
								run_sample->m_brake,
								run_sample->m_clutch,
								run_sample->m_steer,
								run_sample->m_yaw,
								run_sample->m_track_percentage,
								run_sample->m_gear);
								
						}

						g_output_sample_time = (double) ((int) (sample_time * 100) + 1) / 100.0f;
					}

					// update global state

					g_time = sample_time;
					g_lateral_acceleration = lateral_acceleration;
					g_longitudinal_acceleration = longitudinal_acceleration;
					g_x_position = x_position;
					g_y_position = y_position;
					g_z_position = z_position;
					g_x_velocity = x_velocity;
					g_y_velocity = y_velocity;
					g_z_velocity = z_velocity;
					g_speed = speed;
					g_rpm = rpm;
					g_slip = slip;
					g_throttle = throttle;
					g_brake = brake;
					g_clutch = clutch;
					g_steer = steer;
					g_yaw_rate = yaw_rate;
					g_track_percentage = track_percentage;
					g_gear = gear;

					AppClearSample();

					if (AppCheckIfSimDataOverrunQ())
					{
						printf("*OVERRUN*\n");
						AppClearSimDataOverrun();
					}
				}
			}

			if (g_output_file != NULL) 
			{
				fclose(g_output_file);	// close the existing file if opened
				g_output_file = NULL;
			}

			if (g_use_track_path)
			{
				trackPath.Close();
			}

			if (g_use_track_guide)
			{
				trackGuide.Stop();
			}

			if (run.is_open())
			{
//				run.trim();
//				run.offset();
				run.write_to_file(track_path, timeinfo.tm_year - 100, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
				run.close();
			}

			printf("\n");
			printf("Simulator has deactivated.\n");

		}

		AppEnd();
	}
	return 0;
}
