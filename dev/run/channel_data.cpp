#include "channel_data.h"
#include "csv_data.h"
#include "run_data.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

e_channel c_channel_data::get_channel_from_data(unsigned char in_data)
{
	e_channel channel = e_channel_invalid;

	switch((e_channel) in_data)
	{
	case e_channel_run_information:
	case e_channel_start_stop:
	case e_channel_logger_serial_number:
	case e_channel_gps_time_of_week:
	case e_channel_acceleration_data:
	case e_channel_time_stamp:
	case e_channel_gps_position:
	case e_channel_gps_speed:
	case e_channel_beacon_pulse:
	case e_channel_gps_pulse:
	case e_channel_frequency_2:
	case e_channel_frequency_3:
	case e_channel_frequency_4:
	case e_channel_frequency_1:
	case e_channel_rpm:
	case e_channel_analog_7:
	case e_channel_analog_5:
	case e_channel_analog_6:
	case e_channel_analog_4:
	case e_channel_analog_3:
	case e_channel_analog_1:
	case e_channel_analog_2:
	case e_channel_analog_0:
	case e_channel_analog_15:
	case e_channel_analog_13:
	case e_channel_analog_14:
	case e_channel_analog_12:
	case e_channel_analog_11:
	case e_channel_analog_9:
	case e_channel_analog_10:
	case e_channel_analog_8:
	case e_channel_analog_16:
	case e_channel_analog_17:
	case e_channel_analog_18:
	case e_channel_analog_19:
	case e_channel_analog_20:
	case e_channel_analog_21:
	case e_channel_analog_22:
	case e_channel_analog_23:
	case e_channel_analog_24:
	case e_channel_analog_25:
	case e_channel_analog_26:
	case e_channel_analog_27:
	case e_channel_analog_28:
	case e_channel_analog_29:
	case e_channel_analog_30:
	case e_channel_analog_31:
	case e_channel_gps_date:
	case e_channel_gps_heading:
	case e_channel_gps_altitude:
	case e_channel_session_info:
		channel= (e_channel) in_data;
		break;
	}

	return channel;
}

int c_channel_data::get_data_length_from_channel(e_channel in_channel)
{
	unsigned int data_length = 0;

	assert(in_channel!=e_channel_invalid);

	switch(in_channel)
	{
	case e_channel_run_information: data_length = c_run_information::k_data_length; break;
	case e_channel_start_stop: data_length = c_start_stop::k_data_length; break;
	case e_channel_logger_serial_number: data_length = c_logger_serial_number::k_data_length; break;
	case e_channel_gps_time_of_week: data_length = c_gps_time_of_week::k_data_length; break;
	case e_channel_acceleration_data: data_length = c_acceleration_data::k_data_length; break;
	case e_channel_time_stamp: data_length = c_time_stamp::k_data_length; break;
	case e_channel_gps_position: data_length = c_gps_position::k_data_length; break;
	case e_channel_gps_speed: data_length = c_gps_speed::k_data_length; break;
	case e_channel_beacon_pulse: data_length = c_beacon_pulse::k_data_length; break;
	case e_channel_gps_pulse: data_length = c_gps_pulse::k_data_length; break;
	case e_channel_frequency_2:
	case e_channel_frequency_3:
	case e_channel_frequency_4:
	case e_channel_frequency_1: data_length = c_frequency_data::k_data_length; break;
	case e_channel_rpm: data_length = c_rpm_data::k_data_length; break;
	case e_channel_analog_7:
	case e_channel_analog_5:
	case e_channel_analog_6:
	case e_channel_analog_4:
	case e_channel_analog_3:
	case e_channel_analog_1:
	case e_channel_analog_2:
	case e_channel_analog_0:
	case e_channel_analog_15:
	case e_channel_analog_13:
	case e_channel_analog_14:
	case e_channel_analog_12:
	case e_channel_analog_11:
	case e_channel_analog_9:
	case e_channel_analog_10:
	case e_channel_analog_8:
	case e_channel_analog_16:
	case e_channel_analog_17:
	case e_channel_analog_18:
	case e_channel_analog_19:
	case e_channel_analog_20:
	case e_channel_analog_21:
	case e_channel_analog_22:
	case e_channel_analog_23:
	case e_channel_analog_24:
	case e_channel_analog_25:
	case e_channel_analog_26:
	case e_channel_analog_27:
	case e_channel_analog_28:
	case e_channel_analog_29:
	case e_channel_analog_30:
	case e_channel_analog_31: data_length = c_analog_data::k_data_length; break;
	case e_channel_gps_date: data_length = c_gps_date::k_data_length; break;
	case e_channel_gps_heading: data_length = c_gps_heading::k_data_length; break;
	case e_channel_gps_altitude: data_length = c_gps_altitude::k_data_length; break;
	case e_channel_session_info:data_length = c_session_info::k_data_length; break;

	}

	assert(data_length!=0);

	return data_length;
}

c_channel_data * c_channel_data::contruct(unsigned char * in_data, int in_data_length)
{
	e_channel channel = get_channel_from_data(in_data[0]);
	assert(channel!=e_channel_invalid);

	unsigned int data_length = get_data_length_from_channel(channel);
	assert(data_length==in_data_length);

	c_channel_data * channel_data = NULL;

	switch(channel)
	{
		case e_channel_run_information:	channel_data = new c_run_information(in_data, in_data_length); break;
		case e_channel_start_stop:	channel_data = new c_start_stop(in_data, in_data_length); break;
		case e_channel_logger_serial_number: channel_data = new c_logger_serial_number(in_data, in_data_length); break;
		case e_channel_gps_time_of_week: channel_data = new c_gps_time_of_week(in_data, in_data_length); break;
		case e_channel_acceleration_data: channel_data = new c_acceleration_data(in_data, in_data_length); break;
		case e_channel_time_stamp: channel_data = new c_time_stamp(in_data, in_data_length); break;
		case e_channel_gps_position: channel_data = new c_gps_position(in_data, in_data_length); break;
		case e_channel_gps_speed: channel_data = new c_gps_speed(in_data, in_data_length); break;
		case e_channel_beacon_pulse: channel_data = new c_beacon_pulse(in_data, in_data_length); break;
		case e_channel_gps_pulse: channel_data = new c_gps_pulse(in_data, in_data_length); break;
		case e_channel_frequency_2:
		case e_channel_frequency_3:
		case e_channel_frequency_4:
		case e_channel_frequency_1: channel_data = new c_frequency_data(in_data, in_data_length); break;
		case e_channel_rpm: channel_data = new c_rpm_data(in_data, in_data_length); break;
		case e_channel_analog_7:
		case e_channel_analog_5:
		case e_channel_analog_6:
		case e_channel_analog_4:
		case e_channel_analog_3:
		case e_channel_analog_1:
		case e_channel_analog_2:
		case e_channel_analog_0:
		case e_channel_analog_15:
		case e_channel_analog_13:
		case e_channel_analog_14:
		case e_channel_analog_12:
		case e_channel_analog_11:
		case e_channel_analog_9:
		case e_channel_analog_10:
		case e_channel_analog_8:
		case e_channel_analog_16:
		case e_channel_analog_17:
		case e_channel_analog_18:
		case e_channel_analog_19:
		case e_channel_analog_20:
		case e_channel_analog_21:
		case e_channel_analog_22:
		case e_channel_analog_23:
		case e_channel_analog_24:
		case e_channel_analog_25:
		case e_channel_analog_26:
		case e_channel_analog_27:
		case e_channel_analog_28:
		case e_channel_analog_29:
		case e_channel_analog_30:
		case e_channel_analog_31: channel_data = new c_analog_data(in_data, in_data_length); break;
		case e_channel_gps_date: channel_data = new c_gps_date(in_data, in_data_length); break;
		case e_channel_gps_heading: channel_data = new c_gps_heading(in_data, in_data_length); break;
		case e_channel_gps_altitude: channel_data = new c_gps_altitude(in_data, in_data_length); break;
		case e_channel_session_info: channel_data = new c_session_info(in_data, in_data_length); break;
	}

	assert(channel_data!=NULL);

	if (channel_data!=NULL)
	{
		assert(channel_data->is_valid());

		if (!channel_data->is_valid())
		{
			delete channel_data;
			channel_data = NULL;
		}
	}

	return channel_data;
}

c_channel_data::c_channel_data(void)
{
	m_valid = false;
	m_data_length = 0;
}

c_channel_data::c_channel_data(unsigned char * in_data, int in_data_length)
{
	set_data(in_data, in_data_length);
}

unsigned char c_channel_data::calculate_checksum(unsigned char * in_data, int in_data_length)
{
	unsigned char checksum = 0;

	for (int i = 0; i < in_data_length; i++)
		checksum += in_data[i];

	return checksum;
}

void c_channel_data::set_data(unsigned char * in_data, int in_data_length)
{
	if(in_data_length > 0 && in_data_length <= k_max_data_length)
	{
		e_channel channel = get_channel_from_data(in_data[0]);

		if (channel!=e_channel_invalid)
		{
			m_channel = channel;
			m_data_length = in_data_length;

			memcpy(m_data, in_data, m_data_length);

			unsigned char checksum = calculate_checksum(m_data, m_data_length - 1);

			m_valid = (checksum == in_data[m_data_length-1]);
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		assert(0);
		m_valid = false;
	}

	if (!m_valid)
	{
		m_data_length = 0;
		m_channel = e_channel_invalid;
	}
}

e_channel c_channel_data::get_channel(void)
{
	assert(m_valid);
	return m_channel; 
}

int c_channel_data::get_data_length(void) 
{
	assert(m_valid);
	return m_data_length; 
}

unsigned char * c_channel_data::get_data(void)
{
	assert(m_valid);
	return m_data; 
}

void c_channel_data::get_csv_data(c_csv_data * in_csv_data)
{
	// do nothing
}

void c_channel_data::get_run_sample_data(c_run_sample * in_run_sample)
{
	// do nothing
}


void c_channel_data::write_run_data(FILE * in_output_file)
{
	assert(m_valid);
	for(int i=0; i<m_data_length;i++)
		fputc(m_data[i], in_output_file);
}


//
// c_run_information
//

void c_run_information::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "run information");
}

void c_run_information::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_start_stop
//

void c_start_stop::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "start stop %02u %02u", m_data[1], m_data[2]);
}

void c_start_stop::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_logger_serial_number
//

void c_logger_serial_number::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "logger serial number");
}

void c_logger_serial_number::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_gps_time_of_week
//

void c_gps_time_of_week::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps time of week");
}

void c_gps_time_of_week::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_acceleration_data
//

c_acceleration_data::c_acceleration_data(double in_lateral_acceleration, double in_longitudinal_acceleration)
{
	assert(in_lateral_acceleration > -127.0 && in_lateral_acceleration < 127.0);
	assert(in_longitudinal_acceleration > -127.0 && in_longitudinal_acceleration < 127.0);

	m_lateral_acceleration = in_lateral_acceleration;
	m_longitudinal_acceleration = in_longitudinal_acceleration;

	if (m_lateral_acceleration < 0.0f)
	{
		m_lateral_data = (unsigned int) ((-m_lateral_acceleration * 256.0) + 0.5f);
		m_lateral_data |= 0x8000;
	}
	else
	{
		m_lateral_data = (unsigned int) ((m_lateral_acceleration * 256.0) + 0.5f);
	}

	if (m_longitudinal_acceleration < 0.0f)
	{
		m_longitudinal_data = (unsigned int) ((-m_longitudinal_acceleration * 256.0) + 0.5f);
		m_longitudinal_data |= 0x8000;
	}
	else
	{
		m_longitudinal_data = (unsigned int) ((m_longitudinal_acceleration * 256.0) + 0.5f);
	}

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] = (m_lateral_data >> 8) & 0xff;
	m_data[i++] = m_lateral_data & 0xff;
	m_data[i++] = (m_longitudinal_data >> 8) & 0xff;
	m_data[i++] = m_longitudinal_data & 0xff;
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);

}

void c_acceleration_data::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "acceleration %lf(%d), %lf(%d)",
		m_lateral_acceleration, m_lateral_data, m_longitudinal_acceleration, m_longitudinal_data);
}

void c_acceleration_data::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_lateral_data = (m_data[1] << 8) | m_data[2];
	m_longitudinal_data = (m_data[3] << 8) | m_data[4];

	m_lateral_acceleration = (double) (m_lateral_data & 0x7fff) / 256.0;
	m_longitudinal_acceleration = (double) (m_longitudinal_data & 0x7fff) / 256.0;

	if ((m_lateral_data & 0x8000) == 0x8000 && m_lateral_data != 0x8000) m_lateral_acceleration = -m_lateral_acceleration;
	if ((m_longitudinal_data & 0x8000) == 0x8000 && m_longitudinal_data != 0x8000) m_longitudinal_acceleration = -m_longitudinal_acceleration;
}
 
void c_acceleration_data::get_csv_data(c_csv_data * in_csv_data)
{
	in_csv_data->m_accelerations_present = true;
	in_csv_data->m_lateral_acceleration = m_lateral_acceleration;
	in_csv_data->m_longitudinal_acceleration = m_longitudinal_acceleration;
}

void c_acceleration_data::get_run_sample_data(c_run_sample * in_run_sample)
{
	// RT has positive right and we store in positive left (matching physics engine)
	in_run_sample->m_lateral_acceleration = -m_lateral_acceleration;
	in_run_sample->m_longitudinal_acceleration = m_longitudinal_acceleration;
}


//
// c_time_stamp
//

void c_time_stamp::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "time stamp %f", m_time_stamp);
}

void c_time_stamp::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_time_stamp_data = m_data[1] << 16 | m_data[2] << 8 | m_data[3];
	m_time_stamp = (double) m_time_stamp_data / 100.0f;
}

void c_time_stamp::get_csv_data(c_csv_data * in_csv_data)
{
	in_csv_data->m_time_stamp_present= true;
	in_csv_data->m_time_stamp = m_time_stamp;
}

void c_time_stamp::get_run_sample_data(c_run_sample * in_run_sample)
{
	in_run_sample->m_time = m_time_stamp;
}

c_time_stamp::c_time_stamp(double in_time_stamp)
{
	m_time_stamp = in_time_stamp;

	m_time_stamp_data = (unsigned int) ((m_time_stamp * 100.0f) + 0.5f);

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] =  (m_time_stamp_data >> 16) & 0xff;
	m_data[i++] = (m_time_stamp_data >> 8) & 0xff;
	m_data[i++] = m_time_stamp_data & 0xff;
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);
}

//
// c_gps_position
//

c_gps_position::c_gps_position(double in_longitude, double in_latitude, double in_accuracy)
{
	assert(in_longitude >= -180.0 && in_longitude <= 180.0);
	assert(in_latitude >= -90.0 && in_latitude <= 90.0);
	assert(in_accuracy >= 0.0 && in_accuracy <= (double) 0xffffffff);

	m_longitude = in_longitude; // degress
	m_latitude = in_latitude; // degrees
	m_accuracy = in_accuracy; // mm

	if (m_longitude < 0.0f)
	{
		m_longitude_data = (unsigned int) ((-m_longitude * 10000000) + 0.5f);
		m_longitude_data ^= 0xffffffff;
	}
	else
	{
		m_longitude_data = (unsigned int) ((m_longitude * 10000000) + 0.5f);
	}

	if (m_latitude < 0.0f)
	{
		m_latitude_data = (unsigned int) ((-m_latitude * 10000000) + 0.5f);
		m_latitude_data ^= 0xffffffff;
	}
	else
	{
		m_latitude_data = (unsigned int) ((m_latitude * 10000000) + 0.5f);
	}

	m_accuracy_data = (unsigned int) (m_accuracy + 0.5f);

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] = (m_longitude_data >> 24) & 0xff;
	m_data[i++] = (m_longitude_data >> 16) & 0xff;
	m_data[i++] = (m_longitude_data >> 8) & 0xff;
	m_data[i++] = m_longitude_data & 0xff;
	m_data[i++] = (m_latitude_data >> 24) & 0xff;
	m_data[i++] = (m_latitude_data >> 16) & 0xff;
	m_data[i++] = (m_latitude_data >> 8) & 0xff;
	m_data[i++] = m_latitude_data & 0xff;
	m_data[i++] = (m_accuracy_data >> 24) & 0xff;
	m_data[i++] = (m_accuracy_data >> 16) & 0xff;
	m_data[i++] = (m_accuracy_data >> 8) & 0xff;
	m_data[i++] = m_accuracy_data & 0xff;
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);

}

void c_gps_position::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps position (%lf,%lf) accuracy %lf",
		m_longitude, m_latitude, m_accuracy);
}

void c_gps_position::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_longitude_data = (m_data[1] << 24) | (m_data[2] << 16) | (m_data[3] << 8) | m_data[4];
	m_latitude_data = (m_data[5] << 24) | (m_data[6] << 16) | (m_data[7] << 8) | m_data[8];
	m_accuracy_data = (m_data[9] << 24) | (m_data[10] << 16) | (m_data[11] << 8) | m_data[12];

	if ((m_longitude_data & 0x80000000) == 0x80000000)
	{
		m_longitude = (double) (m_longitude_data ^ 0xffffffff) * 0.0000001;	// degrees
		m_longitude = -m_longitude;
	}
	else
	{
		m_longitude = (double) m_longitude_data * 0.0000001;	// degrees
	}


	if ((m_latitude_data & 0x80000000) == 0x80000000)
	{
		m_latitude = (double) (m_latitude_data ^ 0xffffffff) * 0.0000001;	// degrees
		m_latitude = -m_latitude;
	}
	else
	{
		m_latitude = (double) m_latitude_data * 0.0000001;	// degrees
	}

	m_accuracy = (double) (m_accuracy_data);

}

void c_gps_position::get_csv_data(c_csv_data * in_csv_data)
{
	in_csv_data->m_gps_positions_present = true;
	in_csv_data->m_gps_latitude_position = m_latitude;
	in_csv_data->m_gps_longitude_position = m_longitude;
	in_csv_data->m_gps_position_accuracy = m_accuracy;

	in_csv_data->m_gps_y = (m_latitude - 45.0) * 111071.04;
	in_csv_data->m_gps_x = (m_longitude + 100.0) * 78857.856;
}

void c_gps_position::get_run_sample_data(c_run_sample * in_run_sample)
{
	in_run_sample->m_latitude_position = m_latitude;
	in_run_sample->m_longitude_position = m_longitude;
}

//
// c_gps_speed
//

c_gps_speed::c_gps_speed(double in_gps_speed, double in_gps_speed_accuracy)
{
	assert(in_gps_speed >= 0.0f && in_gps_speed < 300.0f);
	assert(in_gps_speed_accuracy >= 0.0f && in_gps_speed_accuracy < 300.0f);

	m_gps_speed = in_gps_speed;			// m/s
	m_gps_speed_accuracy = in_gps_speed_accuracy;	// m/s

	m_gps_speed_data = (unsigned int) ((m_gps_speed * 100.0f) + 0.5f);
	m_gps_speed_accuracy_data = (unsigned int) ((m_gps_speed_accuracy * 100.0f) + 0.5f);

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] = (unsigned char) ((m_gps_speed_data >> 24) & 0xff);
	m_data[i++] = (unsigned char) ((m_gps_speed_data >> 16) & 0xff);
	m_data[i++] = (unsigned char) ((m_gps_speed_data >> 8) & 0xff);
	m_data[i++] = (unsigned char) (m_gps_speed_data & 0xff);
	m_data[i++] = 0; // data source
	m_data[i++] = (unsigned char) ((m_gps_speed_accuracy_data >> 16) & 0xff);
	m_data[i++] = (unsigned char) ((m_gps_speed_accuracy_data >> 8) & 0xff);
	m_data[i++] = (unsigned char) (m_gps_speed_accuracy_data & 0xff);
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);
}

void c_gps_speed::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps speed");
}

void c_gps_speed::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_gps_speed_data = (m_data[1] << 24) | (m_data[2] << 16) | (m_data[3] << 8) | m_data[4];
	m_gps_speed_accuracy_data = (m_data[6] << 16) | (m_data[7] << 8) | m_data[8];

	m_gps_speed = (double) m_gps_speed_data / 100.0;
	m_gps_speed_accuracy = (double) m_gps_speed_accuracy_data / 100.0;

}

void c_gps_speed::get_csv_data(c_csv_data * in_csv_data)
{
	in_csv_data->m_gps_speed_present = true;
	in_csv_data->m_gps_speed = m_gps_speed;
	in_csv_data->m_gps_speed_accuracy = m_gps_speed_accuracy;
}

void c_gps_speed::get_run_sample_data(c_run_sample * in_run_sample)
{
	in_run_sample->m_speed = m_gps_speed;
}


//
// c_beacon_pulse
//

void c_beacon_pulse::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "beacon pulse");
}

void c_beacon_pulse::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_gps_pulse
//

void c_gps_pulse::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps pulse");
}

void c_gps_pulse::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_frequency_data
//

void c_frequency_data::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "frequency (%u) %lf(%u)", m_input, m_frequency, m_ticks_per_cycle);
}

void c_frequency_data::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_input = channel_to_input((e_channel) m_data[0]);
	m_ticks_per_cycle = (m_data[1] << 16) | (m_data[2] << 8) | m_data[3];
	if (m_ticks_per_cycle != 0)
		m_frequency = 6000000.0f / m_ticks_per_cycle;
	else
		m_frequency = 0;
}

unsigned int c_frequency_data::channel_to_input(e_channel in_channel)
{
	switch(in_channel)
	{
	case e_channel_frequency_2: return 2;
	case e_channel_frequency_3: return 3;
	case e_channel_frequency_4: return 4;
	case e_channel_frequency_1: return 1;
	default:
		assert(0);
		return 1;
	}
}

e_channel c_frequency_data::input_to_channel(unsigned int in_input)
{
	switch(in_input)
	{
	case 2: return e_channel_frequency_2;
	case 3: return e_channel_frequency_3;
	case 4: return e_channel_frequency_4;
	case 1: return e_channel_frequency_1;
	default:
		assert(0);
		return e_channel_frequency_1;
	}
}

//
// c_rpm_data
//

c_rpm_data::c_rpm_data(double in_rpm)
{
	m_rpm = in_rpm;
	m_frequency = m_rpm / 20.0f;	// 2 pulses per revolusion converted to cycles per second

	if (m_frequency > 1.0f)
	{
		m_ticks_per_cycle = (unsigned int) ((6000000.0f / m_frequency) + 0.5f);
	}
	else
	{
		m_ticks_per_cycle = 0;
	}

	assert(m_ticks_per_cycle <= 0xffffff);

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] = (m_ticks_per_cycle >> 16) & 0xff;
	m_data[i++] = (m_ticks_per_cycle >> 8) & 0xff;
	m_data[i++] = m_ticks_per_cycle & 0xff;
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);
}

void c_rpm_data::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "rpm %f", m_rpm);
}

void c_rpm_data::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_ticks_per_cycle = (m_data[1] << 16) | (m_data[2] << 8) | m_data[3];
	if (m_ticks_per_cycle != 0)
		m_frequency = 6000000.0f / (double) m_ticks_per_cycle;
	else
		m_frequency = 0;

	m_rpm = m_frequency * 20.0f;	// 3 pulses per revolusion convert from cycles per second to rpm

}

void c_rpm_data::get_csv_data(c_csv_data * in_csv_data)
{
	in_csv_data->m_rpm_present = true;
	in_csv_data->m_rpm = m_rpm;
}

void c_rpm_data::get_run_sample_data(c_run_sample * in_run_sample)
{
	in_run_sample->m_rpm = m_rpm;
}

//
// c_analog_data
//

c_analog_data::c_analog_data(unsigned int in_input, unsigned int in_value)
{
	assert(in_value >= 0 && in_value <= 0xffff);
	assert(in_input >= 0 && in_input < 32);

	m_value = in_value;
	m_input = in_input;

	int i = 0;

	m_data[i++] = input_to_channel(m_input);
	m_data[i++] = (unsigned char) ((m_value >> 8) & 0xff);
	m_data[i++] = (unsigned char) (m_value & 0xff);
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);
}

void c_analog_data::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "analog(%u) %u", m_input, m_value);
}

void c_analog_data::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_input = channel_to_input((e_channel) m_data[0]);
	m_value = (m_data[1] << 8) | m_data[2];
}

unsigned int c_analog_data::channel_to_input(e_channel in_channel)
{
	switch(in_channel)
	{
	case e_channel_analog_7: return 7;
	case e_channel_analog_5: return 5;
	case e_channel_analog_6: return 6;
	case e_channel_analog_4: return 4;
	case e_channel_analog_3: return 3;
	case e_channel_analog_1: return 1;
	case e_channel_analog_2: return 2;
	case e_channel_analog_0: return 0;
	case e_channel_analog_15: return 15;
	case e_channel_analog_13: return 13;
	case e_channel_analog_14: return 14;
	case e_channel_analog_12: return 12;
	case e_channel_analog_11: return 11;
	case e_channel_analog_9: return 9;
	case e_channel_analog_10: return 10;
	case e_channel_analog_8: return 8;
	case e_channel_analog_16:
	case e_channel_analog_17:
	case e_channel_analog_18:
	case e_channel_analog_19:
	case e_channel_analog_20:
	case e_channel_analog_21:
	case e_channel_analog_22:
	case e_channel_analog_23:
	case e_channel_analog_24:
	case e_channel_analog_25:
	case e_channel_analog_26:
	case e_channel_analog_27:
	case e_channel_analog_28:
	case e_channel_analog_29:
	case e_channel_analog_30:
	case e_channel_analog_31: return (unsigned int) e_channel_analog_16 - (unsigned int) in_channel + 16;
	default:
		assert(0);
		return 0;
	}
}

e_channel c_analog_data::input_to_channel(unsigned int in_input)
{
	switch(in_input)
	{
	case 7: return e_channel_analog_7;
	case 5: return e_channel_analog_5;
	case 6: return e_channel_analog_6;
	case 4: return e_channel_analog_4;
	case 3: return e_channel_analog_3;
	case 1: return e_channel_analog_1;
	case 2: return e_channel_analog_2;
	case 0: return e_channel_analog_0;
	case 15: return e_channel_analog_15;
	case 13: return e_channel_analog_13;
	case 14: return e_channel_analog_14;
	case 12: return e_channel_analog_12;
	case 11: return e_channel_analog_11;
	case 9: return e_channel_analog_9;
	case 10: return e_channel_analog_10;
	case 8: return e_channel_analog_8;
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31: return (e_channel)((unsigned int) e_channel_analog_16 + in_input + 16);
	default:
		assert(0);
		return e_channel_analog_0;
	}
}

void c_analog_data::get_run_sample_data(c_run_sample * in_run_sample)
{

	switch(m_channel)
	{
	case e_channel_analog_0: in_run_sample->m_slip = m_value; break;
	case e_channel_analog_1: in_run_sample->m_throttle = m_value; break;
	case e_channel_analog_2: in_run_sample->m_brake = m_value; break;
	case e_channel_analog_3: in_run_sample->m_clutch = m_value; break;
	case e_channel_analog_4: in_run_sample->m_steer = m_value; break;
	case e_channel_analog_5: in_run_sample->m_yaw = m_value; break;
	case e_channel_analog_6: in_run_sample->m_track_percentage = m_value; break;
	case e_channel_analog_7: in_run_sample->m_gear = m_value; break;
	default:
		// do nothing
		break;
	}
}

void c_analog_data::get_csv_data(c_csv_data * in_csv_data)
{
	switch(m_channel)
	{
	case e_channel_analog_0: in_csv_data->m_slip = m_value; in_csv_data->m_slip_present = true; break;
	case e_channel_analog_1: in_csv_data->m_throttle = m_value; in_csv_data->m_throttle_present = true;  break;
	case e_channel_analog_2: in_csv_data->m_brake = m_value;  in_csv_data->m_brake_present = true; break;
	case e_channel_analog_3: in_csv_data->m_clutch = m_value;  in_csv_data->m_clutch_present = true; break;
	case e_channel_analog_4: in_csv_data->m_steer = m_value;  in_csv_data->m_steer_present = true; break;
	case e_channel_analog_5: in_csv_data->m_yaw = m_value;  in_csv_data->m_yaw_present = true; break;
	case e_channel_analog_6: in_csv_data->m_track_percentage = m_value;  in_csv_data->m_track_percentage_present = true; break;
	case e_channel_analog_7: in_csv_data->m_gear = m_value;  in_csv_data->m_gear_present = true; break;
	default:
		// do nothing
		break;
	}
}

//
// c_gps_date
//

void c_gps_date::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps date");
}

void c_gps_date::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_gps_heading
//

void c_gps_heading::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps heading");
}

void c_gps_heading::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_gps_altitude
//

void c_gps_altitude::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "gps altitude");
}

void c_gps_altitude::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);
}

//
// c_session_info
//

c_session_info::c_session_info(e_track in_track, e_car in_car)
{
	m_track = in_track;
	m_car = in_car;

	int i = 0;

	m_data[i++] = k_channel;
	m_data[i++] = m_track & 0xff;
	m_data[i++] = m_car & 0xff;
	m_data[i++] = calculate_checksum(m_data, k_data_length - 1);
	m_data_length = i;
	m_valid = true;

	assert(i==k_data_length);
}

void c_session_info::get_description(char * in_buffer, int in_buffer_length)
{
	sprintf_s(in_buffer, in_buffer_length, "track %u car %u", m_track, m_car);
}

void c_session_info::set_data(unsigned char * in_data, int in_data_length)
{
	c_channel_data::set_data(in_data, in_data_length);

	m_track = (e_track) m_data[1];
	m_car = (e_car) m_data[2];
}


//
// c_channel_data_decoder
//

c_channel_data_decoder::c_channel_data_decoder(void)
{
	m_data_position = 0;
	m_data_length = 0;
	m_data = NULL;
}

void c_channel_data_decoder::begin(const char * in_file_name)
{
	FILE * file;

	assert(m_data==NULL);
	assert(m_data_length==0);
	assert(m_data_position==0);

	m_data_length = 0;
	m_data_position = 0;
	m_data = NULL;

	if(fopen_s(&file, in_file_name, "rb")==0)
	{
		if (fseek(file, 0, SEEK_END) == 0)
		{
			int data_length = ftell(file);

			if (data_length > 8)	// make sure data stream is larger then 8 byte header
			{
				unsigned char * data = (unsigned char *) malloc(data_length);

				if (data != NULL)
				{
					fseek(file, 0, SEEK_SET);

					size_t data_read = fread(data, 1, data_length, file);

					if (data_read == data_length)
					{
						if (data[0]==0x98 && data[1]==0x1D && data[2]==0 && data[3]==0 &&
							data[4]==0xC8 && data[5]==0 && data[6]==0 && data[7]==0)
						{
							m_data_length = data_length;
							m_data = data;
							m_data_position = 8;	// start right after header
						}
					}
				}
			}
		}

		fclose(file);
	}
}

void c_channel_data_decoder::end(void)
{
	if (m_data!=NULL)
	{
		free(m_data);
		m_data = NULL;
	}

	m_data_length = 0;
}

void c_channel_data_decoder::skip_channel_data(void)
{
	if(m_data_position < m_data_length)
	{
		e_channel channel = c_channel_data::get_channel_from_data(m_data[m_data_position]);

		assert(channel!=e_channel_invalid);

		if (channel!=e_channel_invalid) 
		{
			int data_length = c_channel_data::get_data_length_from_channel(channel);

			if (m_data_position + data_length <= m_data_length)
			{
				m_data_position += data_length;
			}
		}
	}
}

e_channel c_channel_data_decoder::peek_channel_data(void)
{
	e_channel channel = e_channel_invalid;

	if(m_data_position < m_data_length)
	{
		e_channel test_channel = c_channel_data::get_channel_from_data(m_data[m_data_position]);

		assert(test_channel!=e_channel_invalid);

		if (test_channel!=e_channel_invalid) 
		{
			int data_length = c_channel_data::get_data_length_from_channel(test_channel);

			if (m_data_position + data_length <= m_data_length)
			{
				channel = test_channel;
			}
		}
	}

	return channel;
}

c_channel_data * c_channel_data_decoder::get_channel_data(void)
{
	c_channel_data * channel_data = NULL;

	if(m_data_position < m_data_length)
	{
		e_channel channel = c_channel_data::get_channel_from_data(m_data[m_data_position]);

		assert(channel!=e_channel_invalid);

		if (channel!=e_channel_invalid) 
		{
			int data_length = c_channel_data::get_data_length_from_channel(channel);

			if (m_data_position + data_length <= m_data_length)
			{
				channel_data = c_channel_data::contruct(m_data + m_data_position, data_length);

				m_data_position += data_length;
			}
		}
	}

	return channel_data;
}
