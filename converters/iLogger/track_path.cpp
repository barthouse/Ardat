#include "track_path.h"
#include "..\common\math_util.h"

#include <assert.h>

TrackPath::TrackPath(void)
{
	m_outputPath = NULL;
	m_lapCount = 0;
	m_active = false;
}

void TrackPath::Open(void)
{
	m_nextMarker = 0.0;
	m_lastTrackPercentage = 0.0;
	m_firstSample = true;
	m_markerDelta = 0.001;

	NewLap();

	m_active = true;
}

void TrackPath::NewLap(void)
{
	m_lapCount++;
	m_nextMarker = 0.0;

	ClosePlot();
	OpenPlot();
}

void TrackPath::OpenPlot(void)
{
	assert(m_outputPath==NULL);

	char filename[128];

	sprintf_s(filename, sizeof(filename), "c:\\gnuplot\\path.%d.dat", m_lapCount);

	m_outputPath = fopen(filename, "w+");
}

void TrackPath::ClosePlot(void)
{
	if (m_outputPath!=NULL)
	{
		fclose(m_outputPath);
		m_outputPath = NULL;
	}
}

void TrackPath::Close()
{
	ClosePlot();

	m_active = false;
}

void TrackPath::Sample(double inTrackPercentage, double inX, double inY, double inZ, double * outXOffset, double * outYOffset)
{
	assert(inTrackPercentage < 1.0);

	*outXOffset = 0.0;
	*outYOffset = 0.0;

	if (m_active)
	{
		if (m_firstSample)
		{
			m_firstSample = false;
		}
		else
		{
			// check for wrap around and correct saved last track percentage
			if (inTrackPercentage < 0.1 && m_lastTrackPercentage > 0.9)
			{
				m_lastTrackPercentage -= 1.0;
			}

			// have we crossed the next marker
			if (m_lastTrackPercentage < m_nextMarker && inTrackPercentage >= m_nextMarker)
			{
				double x = 0.0;
				double y = 0.0;
				double z = 0.0;

				if (m_nextMarker == 0.0)
				{
					fprintf(m_outputPath, "%f %f %f %f\n" , 1.0, inX, inY, inZ);

					// we start the path at 0,0,0

					*outXOffset = -inX;
					*outYOffset = -inY;
				}
				else
				{
					double lerp = (m_nextMarker - m_lastTrackPercentage) / (inTrackPercentage - m_lastTrackPercentage);

					x = scalar_lerp(m_lastX, inX, lerp);
					y = scalar_lerp(m_lastY, inY, lerp);
					z = scalar_lerp(m_lastZ, inZ, lerp);
				}

				fprintf(m_outputPath, "%f %f %f %f\n", m_nextMarker, x , y, z);

				m_nextMarker += m_markerDelta;

				if (m_nextMarker >= 1.0) 
				{
					NewLap();
				}
			}
		}

		m_lastTrackPercentage = inTrackPercentage;
		m_lastX = inX;
		m_lastY = inY;
		m_lastZ = inZ;
	}
}
