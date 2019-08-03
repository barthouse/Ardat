#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <assert.h>

#include "irsdk_defines.h"

#include "IBTSession.h"

#include "channel_data.h"

/*

	We want to be able to convert from an iRacing IBT telemetry save file to an iRacing RUN tile.

	The IBT format represents a stream of telemetry data.  This data is continuous in time but
	may contain discontinuities in the location of the car (the car can instantly teleport to
	the pits upon a crash/reset).

	The RUN format represents a continuous stream of telemetry data.

	The following telemetry data from the IBT will be translated and included in the RUN format (initial implementation):
	  - time
	  - latitude/longitude/altitude
	  - gps speed
	  - long/lateral/vertical acceleration

	  Follow up work will include:
	  - pitch/roll/yaw rate
	  - yaw/pitch/roll
	  - time -> add usage of high resolution event timer
	  - driver marker
	  - steering wheel angle
	  - throttle
	  - break
	  - clutch
	  - gear
	  - rpm
	  - lap distance (?)
	  - lap distance percentage (?)
	  - x/y/z velocity
	  - steering wheel torque
	  - steering wheel torque percentage
	  - RR/RF/LR/LF wheel speed
	  - RR/RF/LR/LF tire pressure
	  - RR/RF/LR/LF inner/middle/outer tire temp
	  - RR/RF/LR/LF shock deflection
	  - RR/RF/LR/LF ride height



*/

class IbtToRunConverter
{
public:

	void ProcessFile(const char * inIbtFileName, const char * inRunFileName);

private:

	bool FindParameters(void);

	FILE * m_file;
	IBTSession	m_session;

	IBTParameter * m_sessionTimeParameter;

	IBTParameter * m_latitudeParameter;
	IBTParameter * m_longitudeParameter;
	IBTParameter * m_altitudeParameter;

	IBTParameter * m_speedParameter;

	IBTParameter * m_longitudnalAccelerationParameter;
	IBTParameter * m_lateralAccelerationParameter;
	IBTParameter * m_verticalAccelerationParameter;

	IBTParameter * m_rpmParameter;
};

bool IbtToRunConverter::FindParameters(void)
{
	if((m_sessionTimeParameter = m_session.FindParameter("SessionTime")) == NULL) return false;

	if((m_latitudeParameter = m_session.FindParameter("Lat")) == NULL) return false;
	if((m_longitudeParameter = m_session.FindParameter("Lon")) == NULL) return false;
	if((m_altitudeParameter = m_session.FindParameter("Alt")) == NULL) return false;

	if((m_speedParameter = m_session.FindParameter("Speed")) == NULL) return false;
	
	if((m_longitudnalAccelerationParameter = m_session.FindParameter("LongAccel")) == NULL) return false;
	if((m_lateralAccelerationParameter = m_session.FindParameter("LatAccel")) == NULL) return false;
	if((m_verticalAccelerationParameter = m_session.FindParameter("VertAccel")) == NULL) return false;

	if((m_rpmParameter = m_session.FindParameter("RPM")) == NULL) return false;

	return true;
}

void IbtToRunConverter::ProcessFile(const char * inIbtFileName, const char * inRunFileName)
{
	if (!m_session.Open(inIbtFileName))
	{
		printf("Unable to open ibt file '%s'\n", inIbtFileName);
		return;
	}

	if (FindParameters())
	{
		FILE * output_file;

		errno_t error_number = fopen_s(&output_file, inRunFileName, "wb");
		assert(error_number==0);
		assert(output_file!=NULL);

		// write header

		fputc(0x98, output_file);
		fputc(0x1D, output_file);
		fputc(0x00, output_file);
		fputc(0x00, output_file);
		fputc(0xC8, output_file);
		fputc(0x00, output_file);
		fputc(0x00, output_file);
		fputc(0x00, output_file);

		int tickCount = m_session.GetTickCount();
		assert(tickCount != 0);

		double time = m_session.GetStartTime();
		double tickDuration = m_session.GetTickDuration();

		double oneG = 9.80665;	// 1G = 9.80665 m/s^2

		for (int tickIndex = 0; tickIndex < tickCount; tickIndex++)
		{
			double timeSeconds = time / (1000 * 1000 * 1000);
			c_time_stamp time_stamp(timeSeconds);
			time_stamp.write_run_data(output_file);

			// Accelerations in IBT are in m/s^2.  We must convert to G's.
			double lateralAcceleration = m_lateralAccelerationParameter->GetSample(tickIndex) / oneG;
			double longitudinalAcceleration = m_longitudnalAccelerationParameter->GetSample(tickIndex) / oneG;

			// need to invert lat, physics engine calculates positive left and RT expect positive right
			c_acceleration_data acceleration_data(-lateralAcceleration, longitudinalAcceleration);
			acceleration_data.write_run_data(output_file);

			double longitude = m_longitudeParameter->GetSample(tickIndex);
			double latitude = m_latitudeParameter->GetSample(tickIndex);

			c_gps_position gps_position(longitude, latitude, 1000.0f);
			gps_position.write_run_data(output_file);

			double speed = m_speedParameter->GetSample(tickIndex);
			c_gps_speed gps_speed(speed, 0.1f);
			gps_speed.write_run_data(output_file);

			double rpm = m_rpmParameter->GetSample(tickIndex);
			c_rpm_data rpm_data(rpm);
			rpm_data.write_run_data(output_file);

#if 0
			c_analog_data analog_slip(0, run_sample->m_slip);
			analog_slip.write_run_data(output_file);

			c_analog_data analog_throttle(1, run_sample->m_throttle);
			analog_throttle.write_run_data(output_file);

			c_analog_data analog_brake(2, run_sample->m_brake);
			analog_brake.write_run_data(output_file);

			c_analog_data analog_clutch(3, run_sample->m_clutch);
			analog_clutch.write_run_data(output_file);

			c_analog_data analog_steer(4, run_sample->m_steer);
			analog_steer.write_run_data(output_file);

			c_analog_data analog_yaw(5, run_sample->m_yaw);
			analog_yaw.write_run_data(output_file);

			c_analog_data analog_track_percentage(6, run_sample->m_track_percentage);
			analog_track_percentage.write_run_data(output_file);

			c_analog_data analog_gear(7, run_sample->m_gear);
			analog_gear.write_run_data(output_file);
#endif

			time += tickDuration;
		}

		fclose(output_file);

		printf("done\n");
	}

	m_session.Close();


}

int main(int argc, char* argv[])
{
	IbtToRunConverter converter;

	converter.ProcessFile("c:\\iRacing\\data\\bart\\test.ibt", "c:\\iRacing\\data\\bart\\test.run");

	return 0;
}

