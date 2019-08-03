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

#include "..\iLogger\telemapp.h"
#include "..\common\irt.h"

using namespace iRacingTelem;

static bool    ctrl_c_pressed = false;
//static double  VBOX_HZ        = 20.0;
//static double  IRACING_HZ     = 60.0;
//static char   *VERSION        = "1.4";

BOOL WINAPI ctrlHandler(
    DWORD        ctrlType)
{
	ctrlType; // unused
    ctrl_c_pressed    = true;
    return TRUE;
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
	irt_header_t	irt_header;

	strcpy_s(irt_header.m_track, sizeof(irt_header.m_track), "unknown");
	strcpy_s(irt_header.m_car, sizeof(irt_header.m_car), "unknown");

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

			FILE *fp = NULL;

			unsigned int    seq = 0;	  // Session; used to name files

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

						strcpy_s(irt_header.m_track, sizeof(irt_header.m_track), wk->track);

						for (char *cp; (cp = strchr(irt_header.m_track, '\\')) != NULL ; )
							*cp = '_';

						printf("driving at track '%s'\n", irt_header.m_track);
					}
					else if(newStateData == kCarInfo)
					{
						CarInfo * carInfo = (CarInfo *) AppGetSimData(kCarInfo);

						strcpy_s(irt_header.m_car, sizeof(irt_header.m_car), carInfo->carPath);

						for (char *cp; (cp = strchr(irt_header.m_car, '\\')) != NULL ; )
							*cp = '_';

						printf("getting in car '%s'\n", irt_header.m_car);
						
					}
				}
				else if (newSample)
				{
    				sh = (SampleHeader *) AppGetSimData(kSampleHeader);
    				cd = (ChassisData *) AppGetSimData(kChassisData);
    				dl = (DrivelineData *) AppGetSimData(kDrivelineData);
    				di = (DriverInput *) AppGetSimData(kDriverInput);

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
						sprintf_s(filename, sizeof(filename), "%s_%s_%d_%d.irt", timebuf, irt_header.m_track, ses, sh->sequence);
						fopen_s(&fp, filename, "wb");

						fwrite(&irt_header, sizeof(irt_header), 1, fp);
						
						// Reset session variables
						seq = sh->sequence;

						printf("New Session; Creating log file %s\n", filename);
					}

					irt_sample_t irt_sample;

					irt_sample.m_time = sh->time;
					irt_sample.m_track_percentage = cd->trkPct;
					irt_sample.m_velocity[0] = cd->v[0];
					irt_sample.m_velocity[1] = cd->v[1];
					irt_sample.m_velocity[2] = cd->v[2];
					irt_sample.m_orientation[0] = cd->q[0];
					irt_sample.m_orientation[1] = cd->q[1];
					irt_sample.m_orientation[2] = cd->q[2];
					irt_sample.m_angular_velocity[0] = cd->w[0];
					irt_sample.m_angular_velocity[1] = cd->w[1];
					irt_sample.m_angular_velocity[2] = cd->w[2];
					irt_sample.m_rpm = dl->engRPM;
					irt_sample.m_brake = di->brake;
					irt_sample.m_clutch = di->clutch;
					irt_sample.m_gear = di->gear;
					irt_sample.m_steer = di->steer;
					irt_sample.m_throttle = di->throttle;
					irt_sample.m_steering_torque = cd->steerT;

					fwrite(&irt_sample, sizeof(irt_sample), 1, fp);

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
