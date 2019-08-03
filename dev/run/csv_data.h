#pragma once

#include <stdio.h>

class c_csv_data
{
public:

	c_csv_data(void);

	void write(FILE * in_output_file);
	static void write_header(FILE * in_output_file);

	void write_run_data(FILE * in_output_file);

	bool m_time_stamp_present;
	double m_time_stamp;

	bool m_accelerations_present;
	double m_longitudinal_acceleration;
	double m_lateral_acceleration;

	bool m_gps_positions_present;
	double m_gps_longitude_position;
	double m_gps_latitude_position;
	double m_gps_position_accuracy;
	double m_gps_x;
	double m_gps_y;

	bool m_gps_speed_present;
	double m_gps_speed;
	double m_gps_speed_accuracy;

	bool m_rpm_present;
	double m_rpm;

	bool m_slip_present;
	unsigned int m_slip;

	bool m_throttle_present;
	unsigned int m_throttle;

	bool m_brake_present;
	unsigned int m_brake;

	bool m_clutch_present;
	unsigned int m_clutch;

	bool m_steer_present;
	unsigned int m_steer;

	bool m_yaw_present;
	unsigned int m_yaw;

	bool m_track_percentage_present;
	unsigned int m_track_percentage;

	bool m_gear_present;
	unsigned int m_gear;

};

class c_csv_data_decoder
{
public:

	c_csv_data_decoder(void);

	void begin(const char * in_file_name);
	void end(void);

	c_csv_data * get_csv_data(void);

private:

	FILE * m_input_file;
	bool m_header_read;

};
