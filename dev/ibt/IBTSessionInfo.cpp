#include "IBTSessionInfo.h"

#include <sstream>
#include <string>
#include <fstream>

IBTSessionInfo::IBTSessionInfo()
{
	strcpy_s(m_driverName, "Bart");	// driver info
	strcpy_s(m_car, "car");			// driver info
	strcpy_s(m_circuit, "circuit");	// track name
	strcpy_s(m_weather, "weather");
	strcpy_s(m_sessionName, "session name");
	strcpy_s(m_sessionDescription, "session description");
	strcpy_s(m_raceTest, "race/test");	// event type
	strcpy_s(m_extraNotes, "extra notes");
	m_timeRecorded = 0;
	m_sectorCount = 0;
}

IBTSessionInfo::~IBTSessionInfo() 
{
	// do nothing
}

char * IBTSessionInfo::Find(char * inBuffer, char * inPattern, char * outString)
{
	char * foundStr = strstr(inBuffer, inPattern);

	if (foundStr != NULL)
	{
		foundStr += strlen(inPattern);
	}

	if (outString != NULL)
	{
		int stringLength = 0;

		if (foundStr != NULL)
		{
			char * eol = strchr(foundStr, '\n');

			if (eol != NULL)
			{
				stringLength = eol - foundStr;

				if (stringLength >= kMaxStringLength) stringLength = kMaxStringLength - 1;

				// TODO: We should just be using a string class
				strncpy_s(outString, kMaxStringLength, foundStr, stringLength);

				foundStr += stringLength + 1;
			}
		}

		outString[stringLength] = '\0';
	}

	return foundStr;
}

void IBTSessionInfo::Set(const char * inFilePath, char * inSessionInfoStr)
{
	const char * fileName = inFilePath;
	const char * slash;

	while ((slash = strchr(fileName, '\\')) != NULL)
	{
		fileName = slash + 1;
	}

	strncpy_s(m_sessionName, kMaxStringLength, fileName, kMaxStringLength);
	m_sessionName[kMaxStringLength-1]= '\0';

	char * weekendInfoStr = Find(inSessionInfoStr, "WeekendInfo:", NULL);

	if (weekendInfoStr != NULL)
	{
		Find(weekendInfoStr, " TrackName: ", m_circuit);
		Find(weekendInfoStr, " EventType: ", m_raceTest);

		char sessionIdString[kMaxStringLength];
		char subSessionIdString[kMaxStringLength];

		Find(weekendInfoStr, " SessionID: ", sessionIdString);
		m_sessionId = atoi(sessionIdString);

		Find(weekendInfoStr, " SubSessionID: ", subSessionIdString);
		m_subSessionId = atoi(subSessionIdString);
	}

	char * driverInfoStr = Find(inSessionInfoStr, "DriverInfo:", NULL);

	if (driverInfoStr != NULL)
	{
		char carIdxString[kMaxStringLength];

		if (Find(driverInfoStr, " DriverCarIdx: ", carIdxString) != NULL)
		{
			char searchString[kMaxStringLength];

			strcpy_s(searchString, " - CarIdx: ");
			strcat_s(searchString, carIdxString);

			char * carInfoString = Find(driverInfoStr, searchString, NULL);

			if (carInfoString != NULL)
			{
				Find(carInfoString, "   UserName: ", m_driverName);
				Find(carInfoString, "   CarPath: ", m_car);
			}
		}
	}

	char * splitTimeInfoStr = Find(inSessionInfoStr, "SplitTimeInfo:", NULL);

	if (splitTimeInfoStr != NULL)
	{
		while(m_sectorCount < kMaxSectorCount)
		{
    		char sectorNumString[kMaxStringLength];
			char sectorPctString[kMaxStringLength];
			double sectorPct;

			sprintf_s(sectorNumString, "- SectorNum: %d", m_sectorCount);

			char * sectorNumStr = Find(splitTimeInfoStr, sectorNumString, NULL);

			if (sectorNumStr != NULL)
			{
				if (Find(sectorNumStr, " SectorStartPct:", sectorPctString) != NULL) 
				{
					if (sscanf_s(sectorPctString, " %lf", &sectorPct) == 1)
					{
						m_sectorStartPct[m_sectorCount++] = sectorPct;
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	if (m_sectorCount > 0)
	{
		strcpy_s(m_extraNotes, "sectors: ");

		for(int i=1; i<m_sectorCount; i++)
		{
			char sectorPctString[kMaxStringLength];

			sprintf_s(sectorPctString, "%0.3lf ", m_sectorStartPct[i]);
			strcat_s(m_extraNotes, sectorPctString);
		}

	}
}

char * IBTSessionInfo::GetDriverName(void) 
{ 
	return m_driverName;
}

char * IBTSessionInfo::GetCar(void) 
{
	return m_car;
}

char * IBTSessionInfo::GetCircuit(void)
{
	return m_circuit; 
}

char * IBTSessionInfo::GetWeather(void) 
{ 
	return m_weather;
}

char * IBTSessionInfo::GetSessionName(void) 
{
	return m_sessionName; 
}
char * IBTSessionInfo::
	GetSessionDescription(void) 
{
	return m_sessionDescription;
}

char * IBTSessionInfo::GetRaceTest(void) 
{
	return m_raceTest;
}

char * IBTSessionInfo::GetExtraNotes(void)
{
	return m_extraNotes;
}

time_t IBTSessionInfo::GetTimeRecorded(void)
{
	return m_timeRecorded;
}
