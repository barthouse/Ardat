#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <malloc.h>

#include "irt.h"

irt_samples_t::irt_samples_t(void)
{
	m_sample_count = 0;
	m_samples = NULL;
}

void irt_samples_t::load(char * inFilename)
{
	FILE * file = fopen(inFilename, "rb");

	int seek_result = fseek(file, 0, SEEK_END);
	assert(seek_result==0);

	int file_length = ftell(file);

	seek_result = fseek(file, 0, SEEK_SET);
	assert(seek_result==0);

	if (file_length >= sizeof(m_header))
	{
		int read_count = fread(&m_header, sizeof(m_header), 1, file);
		assert(read_count == 1);

		m_sample_count = (file_length - sizeof(m_header)) / sizeof(irt_sample_t);

		if (m_sample_count > 0)
		{
			m_samples = (irt_sample_t *) malloc(m_sample_count * sizeof(irt_sample_t));

			read_count = fread(m_samples, sizeof(irt_sample_t), m_sample_count, file);
			assert(read_count == m_sample_count);
		}
	}
}

int irt_samples_t::get_sample_count(void)
{
	return m_sample_count;
}

irt_sample_t * irt_samples_t::get_sample(int inIndex)
{
	assert(inIndex >= 0 && inIndex < m_sample_count);

	return &m_samples[inIndex];
}

int irt_samples_t::get_lap_count(void)
{
	int lap_count = 0;
	int back_count = 0;

	if (m_sample_count > 0)
	{
		double last_track_percentage =  m_samples[0].m_track_percentage;

		for (int sample_index = 1; sample_index < m_sample_count; sample_index++)
		{
			irt_sample_t * irt_sample = &m_samples[sample_index];

			double track_percentage = irt_sample->m_track_percentage;

			if (track_percentage < 0.1 && last_track_percentage > 0.9)
			{
				if (back_count == 0)
				{
					lap_count++;
				}
				else
				{
					back_count--;
				}
			}
			else if(irt_sample->m_track_percentage > 0.9 && last_track_percentage < 0.1)
			{
				back_count++;
			}

			last_track_percentage = track_percentage;
		}

		return lap_count;
	}

	return lap_count;
}

void irt_samples_t::plot(char *inFilename)
{
	double last_x = 0;
	double last_y = 0;
	double last_vx = 0;
	double last_vy = 0;
	double last_time = 0;

	double last_track_percentage = 0;

	double last_lap_count = 0;
	double last_lap_percentage = 0;
	double last_lap_percentage_cadence = 0.001;
	double last_next_lap_percentage = 0;

	int last_plot_index = 0;

	FILE * path_dat_file;
	double last_file_lap_count;

	for (int sample_index = 0; sample_index < m_sample_count; sample_index++)
	{
		irt_sample_t * irt_sample = &m_samples[sample_index];

		if (sample_index == 0)
		{
			last_time = irt_sample->m_time;

			last_track_percentage = irt_sample->m_track_percentage;

			last_lap_count = 0;
			last_lap_percentage = last_track_percentage;
			last_next_lap_percentage = last_lap_percentage + last_lap_percentage_cadence;

			last_plot_index = 0;

			char pathDatFilename[128];
			sprintf(pathDatFilename, "%s.path.lap%d.dat", inFilename, (int) last_lap_count);
			last_file_lap_count = last_lap_count;

			path_dat_file = fopen(pathDatFilename, "w+");
		}
		else
		{
			double time = irt_sample->m_time;
			double delta_time = time - last_time;
			double vx = irt_sample->m_velocity[0];
			double vy = irt_sample->m_velocity[1];
			double ax = vx - last_vx;
			double ay = vy - last_vy;

			double delta_x = (last_vx * delta_time) + (0.5 * (ax * delta_time));
			double delta_y = (last_vy * delta_time) + (0.5 * (ay * delta_time));

			double delta_lap_count = 0;
			double track_percentage = irt_sample->m_track_percentage;

			if (track_percentage < 0.1 && last_track_percentage > 0.9)
			{
				// new lap
				delta_lap_count = 1.0;
			}
			else if(irt_sample->m_track_percentage > 0.9 && last_track_percentage < 0.1)
			{
				// old lap
				delta_lap_count = -1.0;
			}

			double lap_count = last_lap_count + delta_lap_count;
			double lap_percentage = lap_count + track_percentage;

			if (lap_percentage >= last_next_lap_percentage)
			{
				double lerp = (last_next_lap_percentage - last_track_percentage) / (lap_percentage - last_track_percentage);

				double x = last_x + (delta_x * lerp);
				double y = last_y + (delta_y * lerp);

				fprintf(path_dat_file, "%f %f %d\n", x, y, last_plot_index++ );

				if (last_lap_count > last_file_lap_count)
				{
					fclose(path_dat_file);

					char pathDatFilename[128];
					sprintf(pathDatFilename, "%s.path.lap%d.dat", inFilename, (int) (last_lap_count));
					last_file_lap_count = last_lap_count;

					path_dat_file = fopen(pathDatFilename, "w+");
				}

				last_next_lap_percentage += last_lap_percentage_cadence;
			}

			last_time = time;
			last_vx = vx;
			last_vy = vy;

			last_x += delta_x;
			last_y += delta_y;

			last_track_percentage = track_percentage;

			last_lap_count = lap_count;
		}
	}
}
