#pragma once
#include <assert.h>

class IBTLap
{
public:

	IBTLap(double inStartTime, double inDuration, int inNumber)
	{
		m_startTime = inStartTime;
		m_duration = inDuration;
		m_number = inNumber;
		m_set = true;
	}

	double GetStartTime(void) { assert(m_set); return m_startTime; }
	double GetEndTime(void) { assert(m_set); return m_startTime + m_duration; }
	double GetDuration(void) { assert(m_set); return m_duration; }
	int GetNumber(void) { assert(m_set); return m_number; }

private:

	bool m_set;
	double m_startTime;
	double m_duration;
	int m_number;

};
