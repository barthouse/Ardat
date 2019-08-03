#include "run_data.h"
#include "channel_data.h"

#include <assert.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <direct.h>

void c_run::open(void)
{
	assert(!m_opened);

	m_run_samples = (c_run_sample *) malloc(sizeof(c_run_sample) * kMaxRunSamples);
	assert(m_run_samples!=NULL);
	m_run_sample_count = 0;

	m_car = e_car_unknown;
	m_track = e_track_unknown;

	m_opened = true;

}

void c_run::close(void)
{
	printf("closing ...");

	assert(m_opened);
	free(m_run_samples);
	m_run_samples = NULL;
	m_run_sample_count = 0;

	m_opened = false;

	printf("done\n");
}

c_run_sample * c_run::new_sample(void)
{
	c_run_sample * run_sample = NULL;

	if (m_run_sample_count < kMaxRunSamples)
	{
		run_sample = &m_run_samples[m_run_sample_count++];
	}

	return run_sample;
}

void c_run::write_to_file(const char * in_track_name, int in_year, int in_month, int in_day, int in_hour, int in_minute)
{
	// make path to file
	char path[256];

	sprintf_s(path, sizeof(path), ".\\%s", in_track_name);
	_mkdir(path);

	sprintf_s(path, sizeof(path), ".\\%s\\%02d_%02d", in_track_name, in_year, in_month);
	_mkdir(path);

	sprintf_s(path, sizeof(path), ".\\%s\\%02d_%02d\\%02d", in_track_name, in_year, in_month, in_day);
	_mkdir(path);

	// Open new file for writing
	char file_path[256];
	sprintf_s(file_path, sizeof(file_path), ".\\%s\\%02d_%02d\\%02d\\%02u_%02u.run", in_track_name, in_year, in_month, in_day, in_hour, in_minute);

	write_to_file(file_path);
}

void c_run::write_to_file(const char * in_file_path)
{
	FILE * output_file;

	printf("outputting to file %s ...", in_file_path);

	errno_t error_number = fopen_s(&output_file, in_file_path, "wb");
	assert(error_number==0);
	assert(output_file!=NULL);

	// write header

	fputc(0x98, output_file);
	fputc(0x1D, output_file);
	fputc(0x00, output_file);
	fputc(0x00, output_file);
	fputc(0xC8, output_file);
	fputc(0x00, output_file);
	fputc(0x00, output_file);
	fputc(0x00, output_file);

	// write session info
	c_session_info session_info(m_track, m_car);
	session_info.write_run_data(output_file);

	// write samples

	int run_sample_index = 0;

	while(run_sample_index < m_run_sample_count)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index++];

		c_time_stamp time_stamp(run_sample->m_time);
		time_stamp.write_run_data(output_file);

		// need to invert lat, physics engine calculates positive left and RT expect positive right
		c_acceleration_data acceleration_data(-run_sample->m_lateral_acceleration, run_sample->m_longitudinal_acceleration);
		acceleration_data.write_run_data(output_file);

		c_gps_position gps_position(run_sample->m_longitude_position, run_sample->m_latitude_position, 1000.0f);
		gps_position.write_run_data(output_file);

		c_gps_speed gps_speed(run_sample->m_speed, 0.1f);
		gps_speed.write_run_data(output_file);

		c_rpm_data rpm_data(run_sample->m_rpm);
		rpm_data.write_run_data(output_file);

		c_analog_data analog_slip(0, run_sample->m_slip);
		analog_slip.write_run_data(output_file);

		c_analog_data analog_throttle(1, run_sample->m_throttle);
		analog_throttle.write_run_data(output_file);

		c_analog_data analog_brake(2, run_sample->m_brake);
		analog_brake.write_run_data(output_file);

		c_analog_data analog_clutch(3, run_sample->m_clutch);
		analog_clutch.write_run_data(output_file);

		c_analog_data analog_steer(4, run_sample->m_steer);
		analog_steer.write_run_data(output_file);

		c_analog_data analog_yaw(5, run_sample->m_yaw);
		analog_yaw.write_run_data(output_file);

		c_analog_data analog_track_percentage(6, run_sample->m_track_percentage);
		analog_track_percentage.write_run_data(output_file);

		c_analog_data analog_gear(7, run_sample->m_gear);
		analog_gear.write_run_data(output_file);
	}

	fclose(output_file);

	printf("done\n");

}

#if 0
void c_run::offset(void)
{
	const int k_max_samples = 20;
	double sample_x_position[k_max_samples];
	double sample_y_position[k_max_samples];
	int sample_count = 0;

	double last_track_percentage = -1.0f;
	double last_x_position = 0.0f;
	double last_y_position = 0.0f;


	int run_sample_index = 0;

	while(run_sample_index < m_run_sample_count && sample_count<k_max_samples)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index++];

		// look for two samples that bound start/finish
		double x_position;
		double y_position;
		double track_percentage;

		x_position = run_sample->m_longitude_position;
		y_position = run_sample->m_latitude_position;
		track_percentage = (double) run_sample->m_track_percentage / 65535.0;

		if (track_percentage < 0.5f && last_track_percentage > 0.5f)
		{
			// looks like we found two samples that cross start finish

			double lerp = 1.0f - last_track_percentage;
			double lerp_x_position = last_x_position + (x_position - last_x_position) * lerp;
			double lerp_y_position = last_y_position + (y_position - last_y_position) * lerp;

			printf("(%f,%f,%f) - (%f,%f,%f) : %f (%f,%f)\n",
				last_x_position, last_y_position, last_track_percentage,
				x_position, y_position, track_percentage,
				lerp, lerp_x_position, lerp_y_position);

			sample_x_position[sample_count] = lerp_x_position;
			sample_y_position[sample_count] = lerp_y_position;
			sample_count++;
		}

		last_track_percentage = track_percentage;
		last_x_position = x_position;
		last_y_position = y_position;

	}

	double x_offset = 0;
	double y_offset = 0;

	if (sample_count > 0)
	{
		for (int i=0;i<sample_count;i++)
		{
			printf("[%i] %f %f\n", i, sample_x_position[i], sample_y_position[i]);
			x_offset += sample_x_position[i];
			y_offset += sample_y_position[i];
		}

		x_offset /= sample_count;
		y_offset /= sample_count;

		x_offset = -100.0f - x_offset;
		y_offset = 45.0f - y_offset;

	}

	// offset samples

	printf("offseting ... x_offset = %f, y_offset = %f\n", x_offset, y_offset);

	run_sample_index = 0;
	while(run_sample_index < m_run_sample_count)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index++];

		run_sample->m_longitude_position += x_offset;
		run_sample->m_latitude_position += y_offset;
	}
}
#endif

void c_run::trim(void)
{
	printf("trimming ... not yet implemented\n");
}

void c_run::read_from_file(const char * in_file_path)
{
	printf("reading from %s ...", in_file_path);

	c_channel_data_decoder data_decoder;

	data_decoder.begin(in_file_path);

	// skip over any channel data until we hit a time stamp
	while(1)
	{
		e_channel channel = data_decoder.peek_channel_data();

		if (channel == e_channel_time_stamp || channel == e_channel_invalid)
			break;

		data_decoder.skip_channel_data();
	}

	// process time stamp data
	while(1)
	{
		// time stamp
		if (data_decoder.peek_channel_data() != e_channel_time_stamp)
			break;

		c_time_stamp * time_stamp = (c_time_stamp *) data_decoder.get_channel_data();

		if (time_stamp->m_time_stamp == 2.0)
		{
			printf("here\n");
		}

		// acceleration
		if (data_decoder.peek_channel_data() != e_channel_acceleration_data)
			break;

		c_acceleration_data * acceleration_data = (c_acceleration_data *) data_decoder.get_channel_data();

		// gps position
		if (data_decoder.peek_channel_data() != e_channel_gps_position)
			break;

		c_gps_position * gps_position = (c_gps_position *) data_decoder.get_channel_data();

		// gps speed
		if (data_decoder.peek_channel_data() != e_channel_gps_speed)
			break;

		c_gps_speed * gps_speed = (c_gps_speed *) data_decoder.get_channel_data();

		// rpm
		if (data_decoder.peek_channel_data() != e_channel_rpm)
			break;

		c_rpm_data  * rpm_data = (c_rpm_data *) data_decoder.get_channel_data();

		// analog data

		c_analog_data * analog_data[8];
		int analog_index = 0;
		while(analog_index < 8)
		{
			if (data_decoder.peek_channel_data() != (e_channel) c_analog_data::input_to_channel(analog_index))
				break;

			analog_data[analog_index++] = (c_analog_data *) data_decoder.get_channel_data();
		}

		if (analog_index != 8)
			break;

		// add run data

		c_run_sample * run_sample = new_sample();

		time_stamp->get_run_sample_data(run_sample);
		delete time_stamp;

		acceleration_data->get_run_sample_data(run_sample);
		delete acceleration_data;

		gps_position->get_run_sample_data(run_sample);
		delete gps_position;

		gps_speed->get_run_sample_data(run_sample);
		delete gps_speed;

		rpm_data->get_run_sample_data(run_sample);
		delete rpm_data;

		for (analog_index = 0; analog_index < 8; analog_index++)
		{
			analog_data[analog_index]->get_run_sample_data(run_sample);
			delete analog_data[analog_index];
		}

	}

	data_decoder.end();

	printf("done\n");
	
}

//
// Based on what track it is ... pick a percentage of track location to set to use and set
// the zero reference for both x and y seperately.
//

void c_run::offset(void)
{
	double x_zero_offset_percentage = 0.5;
	double y_zero_offset_percentage = 0.5;

	double x_offset = 0.0;
	double y_offset = 0.0;

	switch(m_track)
	{
	case e_track_sonoma_long: x_zero_offset_percentage = 0.84; y_zero_offset_percentage = 0.085; break;
	case e_track_lagunaseca: x_zero_offset_percentage = 0.04; y_zero_offset_percentage = 0.25; break;
	default: x_zero_offset_percentage = 0.5; y_zero_offset_percentage = 0.5; break;
	}

	double prev_track_percentage = 0.0;
	double prev_latitude_position = 0.0;
	double prev_longitude_position = 0.0;

	// find the x_offset using the x_zero_offset_percentage
	for(int run_sample_index = 0; run_sample_index < m_run_sample_count; run_sample_index++)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index];

		double track_percentage = (double) run_sample->m_track_percentage / 65535.0;
		double longitude_position = run_sample->m_longitude_position;

		if (run_sample_index > 0)
		{
			if (track_percentage >= x_zero_offset_percentage && prev_track_percentage < x_zero_offset_percentage )
			{
				double lerp = x_zero_offset_percentage - prev_track_percentage;
				double lerp_x_position = prev_longitude_position + (longitude_position - prev_longitude_position) * lerp;

				x_offset = -lerp_x_position;
				break;

			}
		}

		prev_track_percentage = track_percentage;
		prev_longitude_position = longitude_position;
	}


	// find the y_offset using the y_zero_offset_percentage
	for(int run_sample_index = 0; run_sample_index < m_run_sample_count; run_sample_index++)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index];

		double track_percentage = (double) run_sample->m_track_percentage / 65535.0;
		double latitude_position = run_sample->m_latitude_position;

		if (run_sample_index > 0)
		{
			if (track_percentage >= y_zero_offset_percentage && prev_track_percentage < y_zero_offset_percentage )
			{
				double lerp = y_zero_offset_percentage - prev_track_percentage;
				double lerp_y_position = prev_latitude_position + (latitude_position - prev_latitude_position) * lerp;

				y_offset = -lerp_y_position;
				break;
			}
		}

		prev_track_percentage = track_percentage;
		prev_latitude_position = latitude_position;
	}

	for(int run_sample_index = 0; run_sample_index < m_run_sample_count; run_sample_index++)
	{
		c_run_sample * run_sample = &m_run_samples[run_sample_index];

		double track_percentage = (double) run_sample->m_track_percentage / 65535.0;
		double longitude_position = run_sample->m_longitude_position;
		double latitude_position = run_sample->m_latitude_position;

		if (run_sample_index > 0)
		{
			if (track_percentage >= x_zero_offset_percentage && prev_track_percentage < x_zero_offset_percentage )
			{
				double lerp = x_zero_offset_percentage - prev_track_percentage;
				double lerp_x_position = prev_longitude_position + (longitude_position - prev_longitude_position) * lerp;

				printf("x offset %f ->", x_offset);

				x_offset = -lerp_x_position;

				printf("%f\n", x_offset);
			}
		}

		if (run_sample_index > 0)
		{
			if (track_percentage >= y_zero_offset_percentage && prev_track_percentage < y_zero_offset_percentage )
			{
				double lerp = y_zero_offset_percentage - prev_track_percentage;
				double lerp_y_position = prev_latitude_position + (latitude_position - prev_latitude_position) * lerp;

				y_offset = -lerp_y_position;
			}
		}

		run_sample->m_longitude_position += x_offset;
		run_sample->m_latitude_position += y_offset;

		prev_track_percentage = track_percentage;
		prev_latitude_position = latitude_position;
		prev_longitude_position = longitude_position;
	}

}
