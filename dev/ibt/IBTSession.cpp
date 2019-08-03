#include <stdio.h>
#include <assert.h>

#include "irsdk_defines.h"

#include "MemoryMappedFile.h"
#include "IBTSession.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define M_TWO_PI (M_PI+M_PI)

#include "IBTParameter.h"
#include "IBTLap.h"
#include "IBTSessionInfo.h"

IBTSession::IBTSession()
{
	m_sessionInfoString = NULL;
	m_parameters = NULL;

	m_opened = false;
}

IBTSession::~IBTSession()
{
	assert(m_sessionInfoString == NULL);
	assert(!m_opened);
	assert(m_parameters == NULL);
}

bool IBTSession::Open(const char * inFileName)
{
	bool success = false;
	char * filePtr = NULL;
	int tickCount = 0;

	success = m_memoryMappedFile.Open(inFileName);

	//
	// load header
	//

	if (success) {

		filePtr = m_memoryMappedFile.GetBuffer();
			
		memcpy(&m_header, filePtr, sizeof(m_header));
		filePtr += sizeof(m_header);

		memcpy(&m_diskSubHeader, filePtr, sizeof(m_diskSubHeader));
		filePtr += sizeof(m_diskSubHeader);

		int varBufCount = (m_memoryMappedFile.GetLength() - m_header.varBuf[0].bufOffset) / m_header.bufLen;
		
		tickCount = min(m_diskSubHeader.sessionRecordCount, varBufCount);
	}

	//
	// load session info string

	assert(m_sessionInfoString == NULL);
		
	if (success) {
		success = (m_sessionInfoString = (char *) malloc(m_header.sessionInfoLen)) != NULL;
	}

	irsdk_varHeader *varHeaders = NULL;

	if (success) {

		filePtr = m_memoryMappedFile.GetBuffer() + m_header.sessionInfoOffset;
		memcpy(m_sessionInfoString, filePtr, m_header.sessionInfoLen);
		m_sessionInfoString[m_header.sessionInfoLen-1] = '\0';

		varHeaders = (irsdk_varHeader *) (m_memoryMappedFile.GetBuffer() + m_header.varHeaderOffset);
	}

	//
	// create IBTParameters from var headers
	//

	m_lapParameter = NULL;
	m_lapDistPctParameter = NULL;

	//
	// Count the number parameters -- we skip string parameters
	//

	m_parameterCount = 0;
	for (int i = 0; i < m_header.numVars; i++)
	{
		irsdk_varHeader * varHeader = &varHeaders[i];

		if (varHeader->count == 1)
		{
			bool set = false;

			switch(varHeader->type)
			{
			case irsdk_char:
			case irsdk_bool:
			case irsdk_int:
			case irsdk_bitField:
			case irsdk_float:
			case irsdk_double:
				set = true; break;
			default:
				assert(0);
			}

			if (set)
			{
				m_parameterCount++;
			}
		}
	}

	//
	// Additional parameter for yaw, pitch, roll, Ax, Ay, Az, Vx, Vy, Vz
	//
#if 0
	m_parameterCount+=9;
#endif

	if (success)
	{
		success = (m_parameters = new IBTParameter[m_parameterCount]) != NULL;
	}

	//
	// Assign parameters
	//

	if (success)
	{

		int parameterIndex = 0;
		for (int i = 0; i < m_header.numVars; i++)
		{
			irsdk_varHeader * varHeader = &varHeaders[i];

			if (varHeader->count == 1)
			{
				bool set = false;

				switch(varHeader->type)
				{
				case irsdk_char:
				case irsdk_bool:
				case irsdk_int:
				case irsdk_bitField:
				case irsdk_float:
				case irsdk_double:
					set = true; break;
				default:
					assert(0);
				}

				if (set)
				{
					IBTParameter * ibtParameter = &m_parameters[parameterIndex++];

					ibtParameter->Set(varHeader->name, varHeader->desc, varHeader->unit, tickCount, varHeader->offset, varHeader->type, m_memoryMappedFile.GetBuffer() + m_header.varBuf[0].bufOffset, m_header.bufLen);
					
					if (strcmp(varHeader->name, "Lap")==0) {
						m_lapParameter = ibtParameter;
					} else if(strcmp(varHeader->name, "LapDistPct")==0) {
						m_lapDistPctParameter = ibtParameter;
					}
				}
			}
		}

		//
		// LongVel parameter
		//
#if 0
		if (success)
		{
			m_parameters[parameterIndex++].Set("Yaw", "Yaw", "rad", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Pitch", "Pitch", "rad", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Roll", "Roll", "rad", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Ax", "Ax", "m/s^2", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Ay", "Ay", "m/s^2", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Az", "Az", "m/s^2", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Vx", "Ax", "m/s^2", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Vy", "Ay", "m/s^2", m_diskSubHeader.sessionRecordCount);
			m_parameters[parameterIndex++].Set("Vz", "Az", "m/s^2", m_diskSubHeader.sessionRecordCount);
		}
#endif
	}

	if (success)
	{
		m_tickCount = tickCount;
		m_startTime = m_diskSubHeader.sessionStartTime * 1000000000;
		m_tickDuration = 1000000000.0 / (double) m_header.tickRate;

		m_opened = true;
	}

	if (success)
	{
		success = CalculateLaps();
	}

	if (success)
	{
		success = CalculateVelocities();
	}

	if (!success) 
	{
		Close();
	}

	if (success)
	{
		m_info.Set(inFileName, m_sessionInfoString);
	}

	return m_opened;
}

void IBTSession::Close()
{
	m_memoryMappedFile.Close();

	if (m_sessionInfoString!=NULL)
	{
		free(m_sessionInfoString);
		m_sessionInfoString = NULL;
	}

	if (m_parameters!=NULL)
	{
		for(int i=0; i<m_parameterCount;i++) {
			m_parameters[i].Clear();
		}
		delete [] m_parameters;
		m_parameters = NULL;
	}

	m_lapParameter = NULL;
	m_lapDistPctParameter = NULL;

	m_opened = false;
}

IBTSessionInfo * IBTSession::GetInfo(void)
{ 
	assert(m_opened); 
	return &m_info; 
}

int IBTSession::GetLapCount(void)
{ 
	assert(m_opened);
	return m_laps.size();
}

IBTLap * IBTSession::GetLap(int inLapIndex) 
{ 
	assert(m_opened); 
	assert(inLapIndex < (int) m_laps.size()); 
	return &m_laps[inLapIndex]; 
}

int IBTSession::GetParameterCount(void) 
{ 
	assert(m_opened);
	return m_parameterCount;
}

IBTParameter * IBTSession::GetParameter(int inParameterIndex) 
{
	assert(m_opened); 
	assert(inParameterIndex < m_parameterCount); 
	return &m_parameters[inParameterIndex];
}

double IBTSession::GetStartTime(void)
{
	assert(m_opened);
	return m_startTime;
}

double IBTSession::GetEndTime(void) 
{ 
	assert(m_opened);
	return GetStartTime() + GetDuration();
}

double IBTSession::GetDuration(void) 
{ 
	assert(m_opened);
	return (GetTickCount() - 1) * GetTickDuration();
}

double IBTSession::GetTickDuration(void) 
{
	assert(m_opened);
	return m_tickDuration;
}

int IBTSession::GetTickCount(void) 
{ 
	assert(m_opened);
	return m_tickCount;
}

double IBTSession::GetTickTime(int inTickIndex) 
{ 
	assert(m_opened);
	assert(inTickIndex < m_tickCount);
	return GetStartTime() + (GetTickDuration() * inTickIndex); 
}

int IBTSession::GetTickIndex(double inTickTime)
{
	if (inTickTime <= GetStartTime())
	{
		return 0;
	}

	if (inTickTime >= GetEndTime())
	{
		return GetTickCount() - 1;
	}

	int tickIndex = (int) ((inTickTime - GetStartTime()) / GetTickDuration());

	assert(tickIndex >= 0 && tickIndex < GetTickCount());

	return tickIndex;
}

void IBTSession::IntegrateAngularVelocity(IBTParameter * inRate, IBTParameter * inPos)
{
	if (inRate != NULL && inPos != NULL)
	{
		double position = 0.0f;
		double tickDuration = 1.0 / (double) m_header.tickRate;
		for (int i=0; i<inRate->GetSampleCount(); i++)
		{
			inPos->SetSample(i, position);
			position += inRate->GetSample(i) * tickDuration;
			if (position > M_PI) position -= M_TWO_PI;
			if (position <= -M_PI) position += M_TWO_PI;
		}
	}
}

void IBTSession::Integrate(IBTParameter * inRate, IBTParameter * inPos)
{
	if (inRate != NULL && inPos != NULL)
	{
		double position = 0.0f;
		double tickDuration = 1.0 / (double) m_header.tickRate;
		for (int i=0; i<inRate->GetSampleCount(); i++)
		{
			inPos->SetSample(i, position);
			position += inRate->GetSample(i) * tickDuration;
		}
	}
}

bool IBTSession::CalculateVelocities(void)
{
#if 0
	IBTParameter * ibtYaw = FindParameter("Yaw");
	IBTParameter * ibtPitch = FindParameter("Pitch");
	IBTParameter * ibtRoll = FindParameter("Roll");

	IntegrateAngularVelocity(FindParameter("YawRate"), ibtYaw);
	IntegrateAngularVelocity(FindParameter("PitchRate"), ibtPitch);
	IntegrateAngularVelocity(FindParameter("RollRate"), ibtRoll);

	IBTParameter * ibtAlong = this->FindParameter("LongAccel");
	IBTParameter * ibtAlat = this->FindParameter("LatAccel");
	IBTParameter * ibtAvert = this->FindParameter("VertAccel");

	IBTParameter * ibtAx = this->FindParameter("Ax");
	IBTParameter * ibtAy = this->FindParameter("Ay");
	IBTParameter * ibtAz = this->FindParameter("Az");

	for (int i=0; i<ibtYaw->GetSampleCount(); i++)
	{
		double yaw = ibtYaw->GetSample(i);
		double pitch = ibtPitch->GetSample(i);
		double roll = ibtRoll->GetSample(i);
		double aLong = ibtAlong->GetSample(i);
		double aLat = ibtAlat->GetSample(i);
		double aVert = ibtAvert->GetSample(i);
		double cy = cos(yaw);
		double sy = sin(yaw);
		double cp = cos(pitch);
		double sp = sin(pitch);
		double cr = cos(roll);
		double sr = sin(roll);
		double ax = (aLong * (cp * cy)) + (aLat * (cp * sy)) - (aVert * sp);
		double ay = (aLong * ((cy*sp*sr) - (cr*sy))) + (aLat * ((cr*cy) + (sp*sr*sy))) + (aVert * (cp*sr));
		double az = (aLong * ((cr*cy*sp) + (sr*sy))) + (aLat * ((cr*sp*sy) - (cy*sr))) + (aVert * (cp*cr));

		ibtAx->SetSample(i, ax);
		ibtAy->SetSample(i, ay);
		ibtAz->SetSample(i, az);
	}

	Integrate(FindParameter("Ax"), FindParameter("Vx"));
	Integrate(FindParameter("Ay"), FindParameter("Vy"));
	Integrate(FindParameter("Az"), FindParameter("Vz"));
#endif

	return true;
}

bool IBTSession::CalculateLaps(void)
{
	if (m_laps.empty() && m_lapParameter != NULL && m_lapDistPctParameter != NULL) 
	{
		double lapStart = 0.0;

		for(int tickIndex = 2; tickIndex < m_tickCount; tickIndex++)
		{
			int lastSampleLapNumber = (int) m_lapParameter->GetSample(tickIndex-1);
			int lapSampleNumber = (int) m_lapParameter->GetSample(tickIndex);

			if (lastSampleLapNumber + 1 == lapSampleNumber)
			{
				double lastSampleLapDistPct = m_lapDistPctParameter->GetSample(tickIndex-1);
				double sampleLapDistPct = m_lapDistPctParameter->GetSample(tickIndex);

				if (lastSampleLapDistPct > 0.99 && sampleLapDistPct < 0.01)
				{
					sampleLapDistPct += 1.0;
							
					double lerpPct = (1.0 - lastSampleLapDistPct) / (sampleLapDistPct - lastSampleLapDistPct);
					double time = GetTickTime(tickIndex-1) + GetTickDuration() * lerpPct;

					if (lapStart != 0.0)
					{
						IBTLap lap(lapStart, time - lapStart, lastSampleLapNumber);
						m_laps.push_back(lap);
					}

					lapStart = time;
				}
			}
			else if(lapSampleNumber == 0) {
				lapStart = 0.0;
			}
		}
	}

	return true;
}

IBTParameter * IBTSession::FindParameter(char * inName)
{
	IBTParameter * parameter = NULL;

	for (int i = 0; i < m_parameterCount; i++)
	{
		if (strcmp(inName, m_parameters[i].GetName()) == 0)
		{
			parameter = &m_parameters[i];
		}
	}

	return parameter;
}

void IBTSession::AdjustMinMaxSet(char * inA, char * inB, char * inC, char * inD, char * inE, double inRound, bool inSymetric)
{
	IBTParameter * aParameter = FindParameter(inA);

	if (aParameter != NULL)	
	{
		double min = aParameter->GetMinValue();
		double max = aParameter->GetMaxValue();

		IBTParameter * bParameter = (inB != NULL ? FindParameter(inB) : NULL);
		IBTParameter * cParameter = (inC != NULL ? FindParameter(inC) : NULL);
		IBTParameter * dParameter = (inD != NULL ? FindParameter(inD) : NULL);
		IBTParameter * eParameter = (inE != NULL ? FindParameter(inE) : NULL);

		if (bParameter != NULL)
		{
			double bMin = bParameter->GetMinValue();
			double bMax = bParameter->GetMaxValue();

			if (bMin < min) min = bMin;
			if (bMax > max) max = bMax;
		}

		if (cParameter != NULL)
		{
			double cMin = cParameter->GetMinValue();
			double cMax = cParameter->GetMaxValue();

			if (cMin < min) min = cMin;
			if (cMax > max) max = cMax;
		}

		if (dParameter != NULL)
		{
			double dMin = dParameter->GetMinValue();
			double dMax = dParameter->GetMaxValue();

			if (dMin < min) min = dMin;
			if (dMax > max) max = dMax;
		}

		if (eParameter != NULL)
		{
			double eMin = eParameter->GetMinValue();
			double eMax = eParameter->GetMaxValue();

			if (eMin < min) min = eMin;
			if (eMax > max) max = eMax;
		}

		if (min < 0.0) {
			min = (double) ((int) ((min - inRound + 0.01) / inRound)) * inRound;
		} else {
			min = (double) ((int) (min / inRound)) * inRound;
		}

		if (max < 0.0) {
			max = (double) ((int) (max / inRound)) * inRound;
		} else {
			max = (double) ((int) ((max + inRound - 0.01) / inRound)) * inRound;
		}

		if (inSymetric)
		{
			if (min < 0) min = -min;
			if (min > max) max = min;
			min = -max;
		}

		aParameter->SetMinMax(min, max);
		if (bParameter) bParameter->SetMinMax(min, max);
		if (cParameter) cParameter->SetMinMax(min, max);
		if (dParameter) dParameter->SetMinMax(min, max);
		if (eParameter) eParameter->SetMinMax(min, max);

	}
}

void IBTSession::AdjustMinMax(char * inName, double inRound, bool inSymetric)
{
	AdjustMinMaxSet(inName, NULL, NULL, NULL, NULL, inRound, inSymetric);
}


void IBTSession::AdjustMinMax(void)
{

	AdjustMinMax("FuelLevel", 1.0, false);
	AdjustMinMax("FuelLevelPct", 0.1, false);
	AdjustMinMax("LatAccel", 5.0, true);
	AdjustMinMax("LongAccel", 5.0, true);
	AdjustMinMax("PitchRate", 1.0, true);
	AdjustMinMax("RollRate", 1.0, true);
	AdjustMinMax("RPM", 1000.0, false);
	AdjustMinMax("SteeringWheelAngle", 1.0, true);
	AdjustMinMax("SteeringWheelPctTorque", 1.0, false);
	AdjustMinMax("SteeringWheelTorque", 5.0, true);
	AdjustMinMax("VertAccel", 5.0, true);
	AdjustMinMax("YawRate", 1.0, true);

	AdjustMinMaxSet("LFspeed", "LRspeed", "RFspeed", "RRspeed", "Speed", 10.0, false);
	AdjustMinMaxSet("LFrideHeight","LRrideHeight", "RFrideHeight", "RRrideHeight", NULL, 0.1, false);
	AdjustMinMaxSet("LFshockDefl","LRshockDefl", "RFshockDefl", "RRshockDefl", NULL, 0.1, false);

}
