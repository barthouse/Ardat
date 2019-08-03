#pragma once
#include <string.h>

class IBTSessionInfo
{
public:

	IBTSessionInfo();
	~IBTSessionInfo();

	void Set(const char * inFileName, char * inSessionInfoStr);

	char * GetDriverName(void);
	char * GetCar(void);
	char * GetCircuit(void);
	char * GetWeather(void);
	char * GetSessionName(void);
	char * GetSessionDescription(void);
	char * GetRaceTest(void);
	char * GetExtraNotes(void);

	time_t GetTimeRecorded(void);

private:

	char * Find(char * inBuffer, char * inPattern, char * outScalar);

	static const int kMaxStringLength = 128;

	char m_driverName[kMaxStringLength];
	char m_car[kMaxStringLength];
	char m_circuit[kMaxStringLength];
	char m_weather[kMaxStringLength];
	char m_sessionName[kMaxStringLength];
	char m_sessionDescription[kMaxStringLength];
	char m_raceTest[kMaxStringLength];
	char m_extraNotes[kMaxStringLength];

	int m_sessionId;
	int m_subSessionId;

	time_t m_timeRecorded;

	static const int kMaxSectorCount = 10;

	int m_sectorCount;
	double m_sectorStartPct[kMaxSectorCount];

};
