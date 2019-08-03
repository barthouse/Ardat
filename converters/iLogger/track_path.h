#pragma once

#include <stdio.h>

class TrackPath
{
public:

	TrackPath(void);

	void Open(void);
	void Close(void);

	void Sample(double inTrackPercentage, double inX, double inY, double inZ, double *outXOffset, double *outYOffset);

private:

	void NewLap();
	void OpenPlot();
	void ClosePlot();

	FILE * m_outputPath;

	bool m_active;
	bool m_firstSample;
	int m_lapCount;

	double m_lastTrackPercentage;
	double m_nextMarker;
	double m_markerDelta;
	double m_lastX;
	double m_lastY;
	double m_lastZ;

};

