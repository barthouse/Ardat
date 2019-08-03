#define _CRT_SECURE_NO_WARNINGS

#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "run.h"

run_samples_t::run_samples_t(void)
{
	m_lap_count = 0;
	m_lap_samples = NULL;

	m_sample_count = 0;
	m_samples = NULL;
}

void run_samples_t::run_sample_lerp(run_sample_t * in_a, run_sample_t * in_b, double in_lerp, run_sample_t * out_c)
{
	irt_sample_t * a_irt_sample = &in_a->m_irt_sample;
	irt_sample_t * b_irt_sample = &in_b->m_irt_sample;
	irt_sample_t * c_irt_sample = &out_c->m_irt_sample;

	c_irt_sample->m_time = scalar_lerp(a_irt_sample->m_time, b_irt_sample->m_time, in_lerp);

	for (int axis = 0; axis < 3; axis++)
	{
		c_irt_sample->m_velocity[axis] = (float) scalar_lerp(a_irt_sample->m_velocity[axis], b_irt_sample->m_velocity[axis], in_lerp);
		c_irt_sample->m_angular_velocity[axis] = (float) scalar_lerp(a_irt_sample->m_angular_velocity[axis], b_irt_sample->m_angular_velocity[axis], in_lerp);
		c_irt_sample->m_orientation[axis] = (float) scalar_lerp(a_irt_sample->m_orientation[axis], b_irt_sample->m_orientation[axis], in_lerp);
		out_c->m_position[axis] = scalar_lerp(in_a->m_position[axis], in_b->m_position[axis], in_lerp);
	}

	c_irt_sample->m_rpm = (float) scalar_lerp(a_irt_sample->m_rpm, b_irt_sample->m_rpm, in_lerp);
	c_irt_sample->m_throttle = (float) scalar_lerp(a_irt_sample->m_throttle, b_irt_sample->m_throttle, in_lerp);
	c_irt_sample->m_brake = (float) scalar_lerp(a_irt_sample->m_brake, b_irt_sample->m_brake, in_lerp);
	c_irt_sample->m_clutch = (float) scalar_lerp(a_irt_sample->m_clutch, b_irt_sample->m_clutch, in_lerp);
	c_irt_sample->m_steer = (float) scalar_lerp(a_irt_sample->m_steer, b_irt_sample->m_steer, in_lerp);
	c_irt_sample->m_rpm = (float) scalar_lerp(a_irt_sample->m_rpm, b_irt_sample->m_rpm, in_lerp);
	c_irt_sample->m_gear = (in_lerp <= .5 ? a_irt_sample->m_gear : b_irt_sample->m_gear);
	c_irt_sample->m_steering_torque = (float) scalar_lerp(a_irt_sample->m_steering_torque, b_irt_sample->m_steering_torque, in_lerp);
}


void run_samples_t::load(irt_samples_t * in_irt_samples)
{
	m_lap_count = in_irt_samples->get_lap_count();

	if (m_lap_count > 0)
	{
		m_lap_samples = (run_sample_t **) malloc(m_lap_count * sizeof(run_sample_t *));
		assert(m_lap_samples!=NULL);

		// we will create new samples at start of each new lap
		int irt_sample_count = in_irt_samples->get_sample_count();
		m_sample_count = irt_sample_count + m_lap_count;
		m_samples = (run_sample_t *) malloc(m_sample_count * sizeof(run_sample_t));

		assert(in_irt_samples->get_sample_count() > 1);
		irt_sample_t * last_irt_sample = NULL;
		run_sample_t * last_run_sample = NULL;

		double last_track_percentage = 0;
		int lap_count = 0;
		int back_count = 0;

		int run_sample_index = 0;

		for (int irt_sample_index = 0; irt_sample_index < irt_sample_count; irt_sample_index++)
		{
			irt_sample_t * irt_sample = in_irt_samples->get_sample(irt_sample_index);
			run_sample_t * run_sample = &m_samples[run_sample_index++];
			assert(run_sample_index <= m_sample_count);

			run_sample->m_lap_sample = false;

			// copy irt_sample into run_sample
			run_sample->m_irt_sample = *irt_sample;

			// set position
			if (last_irt_sample != NULL)
			{
				for (int axis = 0; axis < 3; axis++)
				{
					double delta_time = irt_sample->m_time - last_irt_sample->m_time;
					double delta_velocity = irt_sample->m_velocity[axis] - last_irt_sample->m_velocity[axis];

					run_sample->m_position[axis] = last_run_sample->m_position[axis] + 
												  (last_irt_sample->m_velocity[axis] * delta_time) +
												  (0.5 * delta_velocity * delta_time);
				}
			}
			else
			{
				for (int axis = 0; axis < 3; axis++)
				{
					run_sample->m_position[axis] = 0.0f;
				}
			}

			// set offset to zero
			for (int axis = 0; axis < 3; axis++)
			{
				run_sample->m_offset[axis] = 0.0f;
			}

			if (last_irt_sample != NULL)
			{
				if (irt_sample->m_track_percentage < 0.1 && last_irt_sample->m_track_percentage > 0.9)
				{
					if (back_count == 0)
					{
						// lap sample
						run_sample_t * lap_run_sample = &m_samples[run_sample_index++];
						assert(run_sample_index <= m_sample_count);

						lap_run_sample->m_lap_sample = true;

						m_lap_samples[lap_count] = lap_run_sample;

						double delta_percentage = irt_sample->m_track_percentage + (1.0 - last_irt_sample->m_track_percentage);
						double lerp = (1.0 - last_irt_sample->m_track_percentage) / delta_percentage;

						run_sample_lerp(last_run_sample, run_sample, lerp, lap_run_sample);

						// lerp above should have calculated something very close to zero ... we set it to zero
						lap_run_sample->m_irt_sample.m_track_percentage = 0.0f;

						for (int axis = 0; axis < 3; axis++)
						{
							lap_run_sample->m_offset[axis] = 0.0f;
						}

						lap_count++;
					}
					else
					{
						back_count--;
					}
				}
				else if(irt_sample->m_track_percentage > 0.9 && last_irt_sample->m_track_percentage < 0.1)
				{
					back_count++;
				}
			}

			last_irt_sample = irt_sample;
			last_run_sample = run_sample;
		}

		assert(run_sample_index == m_sample_count);

	}
}

// correct the samples using start finish where plot should be crossing same point lap after lap
void run_samples_t::correct_using_start_finish(void)
{
	if (m_lap_count > 2)
	{
		// offset before first start finish so that it hits start finish at 0,0,0
		// use the first lap delta since we don't know what it is exactly
		{
			run_sample_t * first_sample = &m_samples[0];
			run_sample_t * lap_start_sample = m_lap_samples[0];
			run_sample_t * lap_end_sample = m_lap_samples[1];

			double lap_delta[3];
			double lap_offset[3];

			for(int axis = 0; axis < 3; axis++)
			{
				lap_delta[axis] =  lap_start_sample->m_position[axis] - lap_end_sample->m_position[axis];
				lap_offset[axis] = - lap_start_sample->m_position[axis];
			}

			run_sample_t * lap_sample = first_sample;
			while(lap_sample < lap_start_sample)
			{
				for(int axis = 0; axis < 3; axis++)
				{
					lap_sample->m_offset[axis] = lap_offset[axis] + (lap_delta[axis] * lap_sample->m_irt_sample.m_track_percentage);
				}

				lap_sample++;
			}
		}

		// correct full laps
		for (int lap_index = 0; lap_index < m_lap_count - 1; lap_index++)
		{
			run_sample_t * lap_start_sample = m_lap_samples[lap_index];
			run_sample_t * lap_end_sample = m_lap_samples[lap_index+1];

			double lap_delta[3];
			double lap_offset[3];

			for(int axis = 0; axis < 3; axis++)
			{
				lap_delta[axis] =  lap_start_sample->m_position[axis] - lap_end_sample->m_position[axis];
				lap_offset[axis] = - lap_start_sample->m_position[axis];
			}

			run_sample_t * lap_sample = lap_start_sample;
			while(lap_sample < lap_end_sample)
			{
				for(int axis = 0; axis < 3; axis++)
				{
					lap_sample->m_offset[axis] = lap_offset[axis] + (lap_delta[axis] * lap_sample->m_irt_sample.m_track_percentage);
				}

				lap_sample++;
			}
		}

		// correct last partial lap
		{
			run_sample_t * lap_start_sample = m_lap_samples[m_lap_count-2];
			run_sample_t * lap_end_sample = m_lap_samples[m_lap_count-1];
			run_sample_t * last_sample = &m_samples[m_sample_count - 1];

			double lap_delta[3];
			double lap_offset[3];

			for(int axis = 0; axis < 3; axis++)
			{
				lap_delta[axis] = lap_start_sample->m_position[axis] - lap_end_sample->m_position[axis];
				lap_offset[axis] = - lap_end_sample->m_position[axis];
			}

			run_sample_t * lap_sample = lap_end_sample;
			while(lap_sample <= last_sample)
			{
				for(int axis = 0; axis < 3; axis++)
				{
					lap_sample->m_offset[axis] = lap_offset[axis] + (lap_delta[axis] * lap_sample->m_irt_sample.m_track_percentage);
				}

				lap_sample++;
			}
		}

		// now offset all positions
		{
			run_sample_t * first_sample = &m_samples[0];
			run_sample_t * last_sample = &m_samples[m_sample_count - 1];

			run_sample_t * lap_sample = first_sample;
			while(lap_sample <= last_sample)
			{
				for(int axis = 0; axis < 3; axis++)
				{
					lap_sample->m_position[axis] += lap_sample->m_offset[axis];
				}

				lap_sample++;
			}
		}

	}
}

void run_samples_t::plot(char * inFilename, double in_plot_cadence)
{
	if (m_sample_count > 0)
	{
		int lap_count = 0;

		char datPath[128];
		sprintf(datPath, "%s.path.lap%d.dat", inFilename, lap_count);

		FILE * file = fopen(datPath, "w+");
		assert(file!=NULL);

		double plot_track_percentage = m_samples[0].m_irt_sample.m_track_percentage + in_plot_cadence;

		run_sample_t * last_run_sample = NULL;

		for (int run_sample_index = 0; run_sample_index < m_sample_count; run_sample_index++)
		{
			run_sample_t * run_sample = &m_samples[run_sample_index];

			if (run_sample->m_lap_sample)
			{
				// always print out the end of lap point

				fprintf(file, "%f %f %f\n", run_sample->m_position[0], run_sample->m_position[1], run_sample->m_position[2]);

				lap_count++;

				fclose(file);

				sprintf(datPath, "%s.path.lap%d.dat", inFilename, lap_count);

				FILE * file = fopen(datPath, "w+");
				assert(file!=NULL);

				fprintf(file, "%f %f %f\n", run_sample->m_position[0], run_sample->m_position[1], run_sample->m_position[2]);

				assert(run_sample->m_irt_sample.m_track_percentage == 0.0);
				plot_track_percentage = in_plot_cadence;
			}
			else if (run_sample->m_irt_sample.m_track_percentage >= plot_track_percentage)
			{
				assert(last_run_sample != NULL);

				double delta_percentage = run_sample->m_irt_sample.m_track_percentage - last_run_sample->m_irt_sample.m_track_percentage;
				double lerp = (plot_track_percentage - last_run_sample->m_irt_sample.m_track_percentage) / delta_percentage;

				run_sample_t plot_sample;
				run_sample_lerp(last_run_sample, run_sample, lerp, &plot_sample);

				fprintf(file, "%f %f %f\n", plot_sample.m_position[0], plot_sample.m_position[1], plot_sample.m_position[2]);

				plot_track_percentage += in_plot_cadence;
			}


			last_run_sample = run_sample;
		}
	}
}

