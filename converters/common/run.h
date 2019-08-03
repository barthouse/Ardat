#pragma once

#include "irt.h"
#include "math_util.h"

typedef struct _run_sample
{
	bool					m_lap_sample;
	double					m_offset[3];
	double					m_position[3];
	irt_sample_t			m_irt_sample;
} run_sample_t;

class run_samples_t
{
public:

	run_samples_t(void);

	void load(irt_samples_t * in_irt_samples);
	void plot(char * inFilename, double inPlotCadence);

	void correct_using_start_finish(void);

private:

	void run_sample_lerp(run_sample_t * in_a, run_sample_t * in_b, double in_lerp, run_sample_t * out_c);

	int				m_sample_count;
	run_sample_t *	m_samples;

	int				m_lap_count;
	run_sample_t **	m_lap_samples;
};