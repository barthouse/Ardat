#include <stdio.h>

#include "IBTSession.h"

class RacePlot
{
public:

	RacePlot();
	~RacePlot();

	void Open(const char * inInputFileName);
	void Close(void);

	void WriteLapData(const char * inOutputFileName);

private:

	void PrintParameters(void);
	void OpenOutput(const char * inOutputFileName);
	void WriteLap(int inIndex);

	IBTSession	m_session;
	FILE *		m_output;

	double		m_lonOffset;
	double		m_latOffset;
};

RacePlot::RacePlot()
{
	m_output = NULL;
	m_lonOffset = 0.0;
	m_latOffset = 0.0;
}

RacePlot::~RacePlot()
{
	assert(m_output==NULL);
}

void RacePlot::WriteLap(int inLapIndex)
{
	IBTLap * lap = m_session.GetLap(inLapIndex);
	assert(lap != NULL);

	IBTParameter * lonParameter = m_session.FindParameter("Lon");
	assert(lonParameter != NULL);

	IBTParameter * latParameter = m_session.FindParameter("Lat");
	assert(latParameter != NULL);

	int startTickIndex = m_session.GetTickIndex(lap->GetStartTime());
	int endTickIndex = m_session.GetTickIndex(lap->GetEndTime());

	for (int tickIndex = startTickIndex; tickIndex < endTickIndex; tickIndex++)
	{
		double lon = lonParameter->GetSample(tickIndex);
		double lat = latParameter->GetSample(tickIndex);

		fprintf(m_output, "%lf %lf\n", (lon - m_lonOffset) * 1000.0, (lat - m_latOffset) * 1000.0);
	}

}

void RacePlot::Open(const char * inInputFileName)
{
	if(!m_session.Open(inInputFileName))
	{
		printf("Failed to open %s\n", inInputFileName);
		exit(1);
	}

	//
	// Calculate lon/lat offsets
	//

	IBTParameter * lonParameter = m_session.FindParameter("Lon");
	assert(lonParameter != NULL);

	IBTParameter * latParameter = m_session.FindParameter("Lat");
	assert(latParameter != NULL);

	int tickCount = m_session.GetTickCount();
	assert(tickCount != 0);

	m_lonOffset = lonParameter->GetSample(0);
	m_latOffset = latParameter->GetSample(0);

	for (int tickIndex = 0; tickIndex < tickCount; tickIndex++)
	{
		double lon = lonParameter->GetSample(tickIndex);
		double lat = latParameter->GetSample(tickIndex);

		m_lonOffset = min(m_lonOffset, lon);
		m_latOffset = min(m_latOffset, lat);
	}
}

void RacePlot::Close(void)
{
	m_session.Close();
}

void RacePlot::OpenOutput(const char * inOutputFileName)
{
	fopen_s(&m_output, inOutputFileName, "w+");

	if (m_output == NULL)
	{
		printf("Failed to open %s\n", inOutputFileName);
		exit(1);
	}
}

void RacePlot::PrintParameters(void)
{
	int parameterCount = m_session.GetParameterCount();
	for (int parameterIndex = 0; parameterIndex < parameterCount; parameterIndex++)
	{
		printf("%d: %s\n", parameterIndex, m_session.GetParameter(parameterIndex)->GetName());
	}
}

void RacePlot::WriteLapData(const char * inOutputFileName)
{
	OpenOutput(inOutputFileName);

	int lapCount = m_session.GetLapCount();
	for (int lapIndex = 0; lapIndex < lapCount; lapIndex++)
	{
		WriteLap(lapIndex);
		fprintf(m_output, "\n\n");
	}

	fclose(m_output);
	m_output = NULL;
}

int main(int argc, char* argv[])
{
	RacePlot racePlot;

	racePlot.Open("c:\\test\\test.ibt");
	racePlot.WriteLapData("c:\\test\\test.dat");
	racePlot.Close();

	return 0;
}
