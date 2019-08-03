#include "csv_data.h"
#include "channel_data.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

bool g_include_timestamp = true;
bool g_include_acceleration = false;
bool g_include_position = true;
bool g_include_speed = false;
bool g_include_rpm = false;

c_csv_data::c_csv_data(void)
{
	m_time_stamp_present = false;
	m_accelerations_present = false;
	m_gps_positions_present = false;
	m_gps_speed_present = false;
	m_rpm_present = false;
	m_slip_present = false;
	m_throttle_present = false;
	m_brake_present = false;
	m_clutch_present = false;
	m_steer_present = false;
	m_yaw_present = false;
	m_track_percentage_present = false;
	m_gear_present = false;

}

void c_csv_data::write_header(FILE * in_output_file)
{
	bool need_comma = false;

	if (g_include_timestamp)
	{
		fprintf(in_output_file, "timestamp");
		need_comma = true;
	}

	if (g_include_acceleration)
	{
		if (need_comma) fprintf(in_output_file,",");
		fprintf(in_output_file,"long_accl,lat_accl");
		need_comma = true;
	}

	if (g_include_position)
	{
		if (need_comma) fprintf(in_output_file,",");
		fprintf(in_output_file,"x,y,long,lat");
		need_comma = true;
	}

	if (g_include_speed)
	{
		if (need_comma) fprintf(in_output_file,",");
		fprintf(in_output_file,"speed");
		need_comma = true;
	}

	if (g_include_rpm)
	{
		if (need_comma) fprintf(in_output_file,",");
		fprintf(in_output_file,"rpm");
		need_comma = true;
	}

	fprintf(in_output_file, "\n");
}

void c_csv_data::write(FILE * in_output_file)
{
	assert(m_time_stamp_present);

	if (g_include_timestamp)
	{
		fprintf(in_output_file, "%.2f", m_time_stamp);
	}
	
	if (g_include_acceleration)
	{
		if (m_accelerations_present)
		{
			fprintf(in_output_file, ",%.12f,%.12f", m_longitudinal_acceleration, m_lateral_acceleration);
		}
		else
		{
			fprintf(in_output_file, ",,");
		}
	}

	if (g_include_position)
	{
		if (m_gps_positions_present)
		{
			fprintf(in_output_file, ",%.12f,%.12f,%.12f,%.12f", m_gps_x, m_gps_y, m_gps_longitude_position, m_gps_latitude_position);
		}
		else
		{
			fprintf(in_output_file, ",,,");
		}
	}

	if (g_include_speed)
	{
		if (m_gps_speed_present)
		{
			fprintf(in_output_file, ",%f.12,%.12f", m_gps_speed, m_gps_speed_accuracy);
		}
		else
		{
			fprintf(in_output_file, ",,");
		}
	}

	if (g_include_rpm)
	{
		if (m_rpm_present)
		{
			fprintf(in_output_file, ",%.12f", m_rpm);
		}
		else
		{
			fprintf(in_output_file, ",");
		}
	}

	fprintf(in_output_file, "\n");
}

void c_csv_data::write_run_data(FILE * in_output_file)
{
	assert(m_time_stamp_present);

	c_time_stamp time_stamp(m_time_stamp);

	time_stamp.write_run_data(in_output_file);

	if (m_accelerations_present)
	{
		c_acceleration_data acceleration_data(m_lateral_acceleration, m_longitudinal_acceleration);

		acceleration_data.write_run_data(in_output_file);
	}

	if (m_gps_positions_present)
	{
		c_gps_position gps_position(m_gps_longitude_position, m_gps_latitude_position, m_gps_position_accuracy);

		gps_position.write_run_data(in_output_file);
	}

	if (m_gps_speed_present)
	{
		c_gps_speed gps_speed(m_gps_speed, m_gps_speed_accuracy);

		gps_speed.write_run_data(in_output_file);
	}

	if (m_rpm_present)
	{
		c_rpm_data rpm_data(m_rpm);

		rpm_data.write_run_data(in_output_file);
	}

}

//
// c_csv_data_decoder
//

c_csv_data_decoder::c_csv_data_decoder(void)
{
	m_input_file = NULL;
}

void c_csv_data_decoder::begin(const char * in_file_name)
{
	assert(m_input_file==NULL);

	if(fopen_s(&m_input_file, in_file_name, "r")!=0)
	{
		m_input_file= NULL;
	}
	else
	{
		m_header_read = false;
	}
}

void c_csv_data_decoder::end(void)
{
	if (m_input_file!=NULL)
	{
		fclose(m_input_file);
		m_input_file= NULL;
	}
}

c_csv_data * c_csv_data_decoder::get_csv_data(void)
{
	c_csv_data * csv_data = NULL;

	if (m_input_file!=NULL)
	{
		char line_buffer[1024];

		if (!m_header_read) 
		{
			fgets(line_buffer, sizeof(line_buffer), m_input_file);
			// TODO verify header
			m_header_read= true;
		}

		while(1)
		{
			const int k_max_tokens = 17;
			char * tokens[k_max_tokens];
			int token_count= 0;

			if(fgets(line_buffer, sizeof(line_buffer), m_input_file)==NULL)
				break;

			size_t line_length = strlen(line_buffer);

			assert(line_length>0);
			assert(line_buffer[line_length-1]=='\n');

			line_buffer[line_length-1] = '\0';

			char * next_token = line_buffer;

			while(token_count < k_max_tokens)
			{
				char * token = next_token;
				char * seperator = token;
				
				while(*seperator!=',' && *seperator!='\0') seperator++;

				tokens[token_count++] = token;

				if (*seperator=='\0')
					break;

				*seperator= '\0';
				next_token = seperator+1;
			}

			if(token_count!=k_max_tokens)
				continue;

			csv_data = new c_csv_data();

			assert(tokens[0][0]!='\0');

			csv_data->m_time_stamp_present= true;
			csv_data->m_time_stamp = atof(tokens[0]);

			if (tokens[1][0]!='\0' && tokens[2][0]!='\0')
			{
				csv_data->m_accelerations_present= true;
				csv_data->m_longitudinal_acceleration = atof(tokens[1]);
				csv_data->m_lateral_acceleration = atof(tokens[2]);
			}
			else
			{
				csv_data->m_accelerations_present= false;
				csv_data->m_longitudinal_acceleration = 0.0f;
				csv_data->m_lateral_acceleration = 0.0f;
			}

			if (tokens[3][0]!='\0' && tokens[4][0]!='\0' && tokens[5][0]!='\0')
			{
				csv_data->m_gps_positions_present= true;
				csv_data->m_gps_longitude_position= atof(tokens[3]);
				csv_data->m_gps_latitude_position= atof(tokens[4]);
				csv_data->m_gps_position_accuracy= atof(tokens[5]);
			}
			else
			{
				csv_data->m_gps_positions_present= false;
				csv_data->m_gps_longitude_position= 0.0f;
				csv_data->m_gps_latitude_position= 0.0f;
				csv_data->m_gps_position_accuracy= 0.0f;
			}

			if (tokens[6][0]!='\0' && tokens[7][0]!='\0')
			{
				csv_data->m_gps_speed_present= true;
				csv_data->m_gps_speed= atof(tokens[6]);
				csv_data->m_gps_speed_accuracy= atof(tokens[7]);
			}
			else
			{
				csv_data->m_gps_speed_present= false;
				csv_data->m_gps_speed= 0.0f;
				csv_data->m_gps_speed_accuracy= 0.0f;
			}

			if (tokens[8][0]!='\0')
			{
				csv_data->m_rpm_present= true;
				csv_data->m_rpm= atof(tokens[8]);
			}
			else
			{
				csv_data->m_rpm_present= false;
			}

			break;
		}
	}

	return csv_data;
}
