#include <stdio.h>
#include <assert.h>

#include <vector>

#include "irsdk_defines.h"
#include "MemoryMappedFile.h"

#include "IBTParameter.h"
#include "IBTLap.h"
#include "IBTSessionInfo.h"

class IBTSession
{
public:

	IBTSession();
	~IBTSession();

	bool Open(const char * inFileName);
	void Close();

	IBTSessionInfo * GetInfo(void);

	int GetLapCount(void);
	IBTLap * GetLap(int inLapIndex);

	int GetParameterCount(void);
	IBTParameter * GetParameter(int inParameterIndex);
	IBTParameter * FindParameter(char * inName);

	void AdjustMinMax();

	double GetStartTime(void);
	double GetEndTime(void);
	double GetDuration(void);

	double GetTickDuration(void);
	int GetTickCount(void);
	double GetTickTime(int inTickIndex);
	int GetTickIndex(double inTickTime);

private:

	bool CalculateLaps(void);
	bool CalculateVelocities(void);
	void IntegrateAngularVelocity(IBTParameter * inRate, IBTParameter * inPos);
	void Integrate(IBTParameter * inRate, IBTParameter * inPos);

	void AdjustMinMax(char * inName, double inRound, bool inSymetric);
	void AdjustMinMaxSet(char * inA, char * inB, char * inC, char * inD, char * inE, double inRound, bool inSymetric);
	bool m_opened;

	MemoryMappedFile m_memoryMappedFile;

	IBTSessionInfo m_info;

	int m_parameterCount;
	IBTParameter * m_parameters;
	IBTParameter * m_lapParameter;
	IBTParameter * m_lapDistPctParameter;

	std::vector<IBTLap> m_laps;

	double m_startTime;
	double m_tickDuration;
	int m_tickCount;

	irsdk_header m_header;
	irsdk_diskSubHeader m_diskSubHeader;
	char * m_sessionInfoString;
};

