#pragma once

#include <stdio.h>
#include "run_data.h"

class c_csv_data;

enum e_channel
{
	e_channel_invalid = -1,
	e_channel_run_information = 1,
	e_channel_start_stop = 2,
	e_channel_logger_serial_number = 6,
	e_channel_gps_time_of_week = 7,
	e_channel_acceleration_data = 8,
	e_channel_time_stamp = 9,
	e_channel_gps_position = 10,
	e_channel_gps_speed = 11,
	e_channel_beacon_pulse = 12,
	e_channel_gps_pulse = 13,
	e_channel_frequency_2 = 14,
	e_channel_frequency_3 = 15,
	e_channel_frequency_4 = 16,
	e_channel_frequency_1 = 17,
	e_channel_rpm = 18,
	e_channel_analog_7 = 20,
	e_channel_analog_5 = 21,
	e_channel_analog_6 = 22,
	e_channel_analog_4 = 23,
	e_channel_analog_3 = 24,
	e_channel_analog_1 = 25,
	e_channel_analog_2 = 26,
	e_channel_analog_0 = 27,
	e_channel_analog_15 = 28,
	e_channel_analog_13 = 29,
	e_channel_analog_14 = 30,
	e_channel_analog_12 = 31,
	e_channel_analog_11 = 32,
	e_channel_analog_9 = 33,
	e_channel_analog_10 = 34,
	e_channel_analog_8 = 35,
	e_channel_analog_16 = 36,
	e_channel_analog_17 = 37,
	e_channel_analog_18 = 38,
	e_channel_analog_19 = 39,
	e_channel_analog_20 = 40,
	e_channel_analog_21 = 41,
	e_channel_analog_22 = 42,
	e_channel_analog_23 = 43,
	e_channel_analog_24 = 44,
	e_channel_analog_25 = 45,
	e_channel_analog_26 = 46,
	e_channel_analog_27 = 47,
	e_channel_analog_28 = 48,
	e_channel_analog_29 = 49,
	e_channel_analog_30 = 50,
	e_channel_analog_31 = 51,
	e_channel_gps_date = 55,
	e_channel_gps_heading = 56,
	e_channel_gps_altitude = 57,
	e_channel_session_info = 200,
	e_channel_count

};

class c_channel_data
{
public:

	c_channel_data(unsigned char * in_data, int in_data_length);

	virtual void get_description(char * in_buffer, int in_buffer_length) = 0;
	virtual bool is_valid(void) { return m_valid; }

	e_channel get_channel(void);
	int get_data_length(void);
	unsigned char * get_data(void);

	static e_channel get_channel_from_data(unsigned char in_data);
	static int get_data_length_from_channel(e_channel in_channel);
	static c_channel_data * contruct(unsigned char * in_data, int in_data_length);

	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	void write_run_data(FILE * in_output_file);

protected:

	c_channel_data(void);

	unsigned char calculate_checksum(unsigned char * in_data, int in_data_length);

	static const unsigned int k_max_data_length = 32;

	bool m_valid;
	e_channel m_channel;
	int m_data_length;
	unsigned char m_data[k_max_data_length];

};

class c_run_information : public c_channel_data
{
public:

	enum { k_data_length = 9 };

	c_run_information(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_start_stop : public c_channel_data
{
public:

	enum { k_data_length = 11 };

	c_start_stop(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_logger_serial_number : public c_channel_data
{
public:

	enum { k_data_length = 6 };

	c_logger_serial_number(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_gps_time_of_week : public c_channel_data
{
public:

	enum { k_data_length = 6 };

	c_gps_time_of_week(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_acceleration_data : public c_channel_data
{
public:

	enum { k_data_length = 6 };
	enum { k_channel = 8 };

	c_acceleration_data(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_acceleration_data(double in_lateral_acceleration, double in_longitudinal_acceleration);

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	double m_lateral_acceleration;
	double m_longitudinal_acceleration;

private:

	unsigned int m_lateral_data;
	unsigned int m_longitudinal_data;

};

class c_time_stamp : public c_channel_data
{
public:

	c_time_stamp(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_time_stamp(double in_time_stamp);

	enum { k_data_length = 5 };
	enum { k_channel = 9 };

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	double m_time_stamp;

private:

	unsigned int m_time_stamp_data;

};

class c_gps_position : public c_channel_data
{
public:

	c_gps_position(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_gps_position(double in_longitude, double in_latitude, double in_accuracy);

	enum { k_data_length = 14 };
	enum { k_channel = 10 };

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	double m_longitude;
	double m_latitude;
	double m_accuracy;
    double m_x;
    double m_y;

private:

	unsigned int m_longitude_data;
	unsigned int m_latitude_data;
	unsigned int m_accuracy_data;

};

class c_gps_speed : public c_channel_data
{
public:

	enum { k_data_length = 10 };
	enum { k_channel = 11 };

	c_gps_speed(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_gps_speed(double in_gps_speed, double in_gps_speed_accuracy);

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	double m_gps_speed;				// meters per second
	double m_gps_speed_accuracy;

private:

	unsigned int m_gps_speed_data;
	unsigned int m_gps_speed_accuracy_data;


};

class c_beacon_pulse : public c_channel_data
{
public:

	enum { k_data_length = 3 };

	c_beacon_pulse(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_gps_pulse : public c_channel_data
{
public:

	enum { k_data_length = 3 };

	c_gps_pulse(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_frequency_data : public c_channel_data
{
public:

	enum { k_data_length = 5 };

	c_frequency_data(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

	unsigned int channel_to_input(e_channel in_channel);
	e_channel input_to_channel(unsigned int in_input);

private:

	unsigned int m_input;
	unsigned int m_ticks_per_cycle;
	double m_frequency;

};

class c_rpm_data : public c_channel_data
{
public:

	enum { k_data_length = 5 };
	enum { k_channel = 18 };

	c_rpm_data(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_rpm_data(double in_rpm);

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_csv_data(c_csv_data * in_csv_data);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);

	double m_rpm;

private:

	unsigned int m_ticks_per_cycle;

	double m_frequency;

};

class c_analog_data : public c_channel_data
{
public:

	enum { k_data_length = 4 };
	enum { k_channel = 20 };

	c_analog_data(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }
	c_analog_data(unsigned int in_input, unsigned int in_value);

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);
	virtual void get_run_sample_data(c_run_sample * in_run_sample);
	virtual void get_csv_data(c_csv_data * in_csv_data);

	static unsigned int channel_to_input(e_channel in_channel);
	static e_channel input_to_channel(unsigned int in_input);

private:

	unsigned int m_value;
	unsigned int m_input;



};

class c_gps_date : public c_channel_data
{
public:

	enum { k_data_length = 10 };

	c_gps_date(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_gps_heading : public c_channel_data
{
public:

	enum { k_data_length = 10 };

	c_gps_heading(unsigned char * in_data, int in_data_length) { set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

private:

};

class c_gps_altitude : public c_channel_data
{
public:

	enum { k_data_length = 10 };

	c_gps_altitude(unsigned char * in_data, int in_data_length) {  set_data(in_data, in_data_length); }

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

    double m_z; // mm
    double m_accuracy;  // mm

private:

};

class c_session_info : public c_channel_data
{
public:

	enum { k_data_length = 4 };
	enum { k_channel = 200 };

	c_session_info(unsigned char * in_data, int in_data_length) {  set_data(in_data, in_data_length); }
	c_session_info(e_track in_track, e_car in_car);

	virtual void get_description(char * in_buffer, int in_buffer_length);
	virtual void set_data(unsigned char * in_data, int in_data_length);

	e_track	m_track;
	e_car	m_car;

};

class c_channel_data_decoder
{
public:

	c_channel_data_decoder(void);

	void begin(const char * in_file_name);
	void end(void);

	c_channel_data * get_channel_data(void);
	e_channel peek_channel_data(void);
	void skip_channel_data(void);

private:

	int m_data_length;
	int m_data_position;
	unsigned char * m_data;

};
