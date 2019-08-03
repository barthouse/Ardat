#if 0

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

using namespace iRacingTelem;

static bool    ctrl_c_pressed = false;
static double  VBOX_HZ        = 20.0;
static double  IRACING_HZ     = 60.0;
static char   *VERSION        = "1.4";

BOOL WINAPI ctrlHandler(
    DWORD        ctrlType)
{
    // Just exit, regardless of the type of control signal
    (void) ctrlType;
    ctrl_c_pressed    = true;
    return TRUE;
}

static float VEL(float v)
{
    return v * 2.237f;                // M/s to MPH
}

// Normalise angles between 0 and 360
static double ANGLE_NORM(double deg)
{ 
    if (deg < 0.0)
        deg = 360.0 + deg;

    if (deg > 360.0)
        deg = deg - 360.0;

    return deg;
}

static double ANGLE_DIFF(double deg1, double deg2)
{ 
    return ANGLE_NORM(deg1+180.0-deg2)-180.0;
}

static double ANGLE(double a)
{ 
    double deg = (a * 180.0 / 3.14159);    // rad to deg
    return deg;
}



static float SPEED(float v[])
{
    return VEL(sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
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
    double lat0 = 45.00*60.00;	// 45 deg N; in minutes
    return lat0 + y/1851.8184;	// flip sign to make map work right
}

static double x2long(double x)
{
    double long0 = -100.00*60.00; // 100 deg W; minutes
    return long0 - x/1314.2976;	  // only valid at 45 deg N latitude
}

static void printHeading(FILE *fp)
{
    fprintf(fp, "File created on iRacing Telemetry Converter\n");
    fprintf(fp, "\n");
    fprintf(fp, "[header]\n");
    fprintf(fp, "satellites\n");
    fprintf(fp, "time\n");
    fprintf(fp, "latitude\n");
    fprintf(fp, "longitude\n");
    fprintf(fp, "velocity mph\n");
    fprintf(fp, "heading\n");
    fprintf(fp, "height\n");
    fprintf(fp, "yaw rate deg/s\n");
    fprintf(fp, "Slip Angle\n");
    fprintf(fp, "RPM\n");
    fprintf(fp, "Throttle\n");
    fprintf(fp, "Brake\n");
    fprintf(fp, "Steer\n");
    fprintf(fp, "Gear\n");
    fprintf(fp, "GR\n");		// Gear Ratio
    fprintf(fp, "trkPct\n");		// Track percentage
    fprintf(fp, "\n");
    fprintf(fp, "[channel units]\n");
    fprintf(fp, "Degrees\n");		// Slip Angle (drift angle)
    fprintf(fp, "RPM\n");		// RPM
    fprintf(fp, "Percent\n");		// Throttle
    fprintf(fp, "Percent\n");		// Brake
    fprintf(fp, "Degrees\n");		// Steering Angle
    fprintf(fp, "Gear\n");		// Gear
    fprintf(fp, "GR\n");	        // Gear Ratio
    fprintf(fp, "Percent\n");		// Track Percentage
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "[comments]\n");
    fprintf(fp, "Log Rate (Hz) : %.0f\n", VBOX_HZ);
    fprintf(fp, "Software Version : 1.0.0\n");
    fprintf(fp, "UnitType: iRacing Telemetry\n");
    fprintf(fp, "Track Offset (feet):0.00;0.00\n");
    fprintf(fp, "Track Offset (feet):0.00;0.00\n");
    fprintf(fp, "\n");
    fprintf(fp, "[column names]\n");
    fprintf(fp, "sats time lat long velocity heading height yaw-calc slip RPM Throttle Brake Steer Gear\n");
    fprintf(fp, "[data]\n");
}

static void getSample(SampleHeader  **sh, 
		      ChassisData   **cd, 
		      DrivelineData **dl,
		      DriverInput   **di)
		      
{
    *sh = (SampleHeader *) AppGetSimData(kSampleHeader);
    *cd = (ChassisData *) AppGetSimData(kChassisData);
    *dl = (DrivelineData *) AppGetSimData(kDrivelineData);
    *di = (DriverInput *) AppGetSimData(kDriverInput);
}

static void
set_timestr(char   *time_str, 
	    int     len,
            double  t)
{
    int    h = (int)(t/3600.0);
    int    m = (int)((t - 3600.0*h)/60.0);
    double s = t - 3600.0*(double)h - 60.0*(double)m;
    sprintf_s(time_str, len, "%02d%02d%05.2f", h, m, s);
}

/*
 ***************************************************************************
 ***************************************************************************
 */
int main(int argc, char *argv[])
{
    if (argc == 2)
        VBOX_HZ = atof(argv[1]);

    if ((VBOX_HZ > 60.0) || (VBOX_HZ < 1.0)) 
    {
        printf("Usage: %s [Output Rate]\n", argv[0]);
	printf("  where the Output Rate is between 1 and 60 (inclusive)\n");
	exit(1);
    }

    SetConsoleCtrlHandler(ctrlHandler, TRUE);
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    unsigned int    ses = 0;	  // Session; used to name files
    char track[128];
    strcpy_s(track, sizeof(track), "unknown");
    printf("VBOX Data Logger Version %s\n", VERSION);
    printf("    Output Rate is %.0f Hz\n\n", VBOX_HZ);
    printf("Press ^C to exit\n\n");
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
		kCurrentWeekend
            };

            (void) AppRequestDataItems(sizeof(desired)/sizeof(desired[0]), desired);
            (void) AppRequestDataAtPhysicsRate(false);
            (void) AppEnableSampling(true);

            const int timeOutMs = 100;
            eSimDataType newStateData;
            bool newSample;

	    FILE *fp = NULL;

	    unsigned int    seq = 0;	  // Session; used to name files
	    double t0=0;  // previous sample time
	    double x=0;   // increases eaast
	    double y=0;   // increases north
	    double h=0;   // increases upwards

            while((newSample = AppWaitForNewSample(&newStateData, timeOutMs)) == true ||
                  newStateData != kNoStateInfo ||
                  AppCheckIfSimActiveQ())
            {
		SampleHeader *sh;
		ChassisData  *cd;
		DrivelineData *dl;
		DriverInput   *di;

                if (ctrl_c_pressed) break;

		if (newStateData != kNoStateInfo)
		{
		    if (newStateData == kCurrentWeekend) 
		    {
		        // assign the track data
		        CurrentWeekend *wk = (CurrentWeekend *)AppGetSimData(kCurrentWeekend);
                        strcpy_s(track, sizeof(track), wk->track);
                        for (char *cp; (cp = strchr(track, '\\')) != NULL ; )
			    *cp = '_';
		    }
		}
		else if (newSample)
                {
	            getSample(&sh, &cd, &dl, &di);

		    // Test if the sequence number changed; if so, then we'll
		    // open a new file with the sequence number as the session number.
		    if (sh->sequence != seq)
		    {
		        time_t rawtime;
			struct tm timeinfo;
			char timebuf[64];

			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);

			strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", &timeinfo);

		        if (fp != NULL) 
			    fclose(fp);	// close the existing file if opened

			// Open new file for writing
			char filename[256];
			sprintf_s(filename, sizeof(filename), "%s_%s_%d_%d.vbo", timebuf, track, ses, sh->sequence);
			fopen_s(&fp, filename, "w");
                        printHeading(fp);
			
			// Reset session variables
			seq = sh->sequence;
	                t0 = 0;
	                x  = 0;
	                y  = 0;
	                h  = 100.0f;

                        printf("New Session; Creating log file %s\n", filename);
		    }

		    // Update current position
		    double t        = sh->time;
	            double dt       = 1.0/IRACING_HZ;         // MBW: should get by looking at delta time on samples
		    double dx       = -1.0*cd->v[0]*dt;	// force x to icrease to the right
		    double dy       = -1.0*cd->v[1]*dt; // force y to increase upwards
		    double hdg      = ANGLE_NORM(ANGLE(-cd->q[0]) - 90);	// q[0] is yaw (0 points westward)
		    double yawR     = ANGLE_NORM(ANGLE(cd->w[2]));              // w[2] is angular vel for z (up)
		    double dot      = ANGLE_NORM(ANGLE(-atan2(-dx, dy)));	// direction of travel
		    double slip     = ANGLE_DIFF(dot, hdg); 		// slip is negative when oversteering on righthander
                    double vel      = SPEED(cd->v);
		    double rpm      = (dl == NULL) ? 0.0 : dl->engRPM;
		    double throttle = (di == NULL) ? 0.0 : di->throttle;
		    double brake    = (di == NULL) ? 0.0 : di->brake;
		    double steer    = (di == NULL) ? 0.0 : ANGLE(di->steer);	// Neg is to the right; pos is to left
		    double gear     = (di == NULL) ? 0.0 : di->gear;
		    double gearRatio = (rpm < 500) ? 0.0 : 1000.0*vel/rpm;	// Dead engine registers 300 rpm
		    double trkPct    = cd->trkPct;				// Track percentage; 0 at S/F line

		    x += dx;
		    y += dy;
		    h += (float)(cd->v[2]*dt);	// multiply by dt to convert velocity to distance

		    long double intpart;
		    if (modf(t*VBOX_HZ, &intpart) < 1.0/IRACING_HZ)
		    {
			char time_str[64];
			set_timestr(time_str, sizeof(time_str), t);
                        // sats, time, lat, long, vel, hdg, height, yaw-calc, Slip Angle, RPM, throttle, brake, steer, gear, gearRatio, trkPct
                        fprintf(fp, "%d %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", 
                                    8, time_str, y2lat(y), x2long(x), vel, hdg, h, yawR, slip, rpm, throttle, brake, steer, gear, 
				    gearRatio, trkPct);
			//printf("dx = %f, dy = %f, slip = %f, hdg = %f, dot = %f\n", dx, dy, slip, hdg, dot);
			//printf("steer = %f\n", steer);
		    }

		    t0 = t;
		    AppClearSample();

                    if (AppCheckIfSimDataOverrunQ())
                    {
                        printf("*OVERRUN*\n");
                        AppClearSimDataOverrun();
                    }
                }
            }
	    if (fp != NULL) 
	        fclose(fp);	// close the existing file if opened

            printf("\n");
            printf("Simulator has deactivated.\n");

        }

        AppEnd();
    }
    return 0;
}

#endif
