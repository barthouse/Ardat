#pragma once

#include <assert.h>
#include <stdio.h>

class TrackGuide
{
public:

	TrackGuide(void);

	void Start();
	void Stop();

	void Sample(double inTrackPercentage, double inX, double inY, double inZ, double inSpeed, double * outOffsetX, double * outOffsetY);

private:

	void NewLap(void);
	void ClosePlots(void);
	void OpenPlots(void);

	bool	m_active;
	bool	m_firstSample;
	int		m_lapCount;
	int		m_markerCount;

	int		m_nextMarker;

	double m_lastTrackPercentage;
	double m_lastX;
	double m_lastY;
	double m_lastZ;

	FILE * m_crossOutput;
	FILE * m_trajectoryOutput;
	FILE * m_intersectOutput;
};
