#pragma once

#include <stdio.h>
#include <assert.h>

enum e_track {
	e_track_unknown,
	e_track_sonoma_long,
	e_track_lagunaseca,
	e_track_count
};

enum e_car {
	e_car_unknown,
	e_car_rt2000,
	e_car_count
};

typedef struct _c_run_sample
{
public:

	double m_time;
	double m_lateral_acceleration;
	double m_longitudinal_acceleration;
	double m_longitude_position;
	double m_latitude_position;
	double m_speed;
	double m_rpm;
	unsigned int m_slip;
	unsigned int m_throttle;
	unsigned int m_brake;
	unsigned int m_clutch;
	unsigned int m_steer;
	unsigned int m_yaw;
	unsigned int m_track_percentage;
	unsigned int m_gear;

} c_run_sample;

class c_run
{
public:

	c_run(void) 
	{
		m_opened= false;
	}

	void open(void);
	void close(void);

	void trim(void);
	void offset(void);
	void write_to_file(const char * in_track_name, int in_year, int in_month, int in_day, int in_hour, int in_minute);
	void write_to_file(const char * in_file_path);
	void read_from_file(const char * in_file_path);

	void set_car(e_car in_car) { m_car = in_car; }
	void set_track(e_track in_track) { m_track = in_track; }

	bool is_open(void) { return m_opened; }

	int run_sample_count(void) { return m_run_sample_count; }
	c_run_sample * get_sample(int inIndex) { assert(inIndex >= 0 && inIndex < m_run_sample_count); return &m_run_samples[inIndex]; }
	c_run_sample * new_sample(void);

private:

	bool m_opened;

	e_car	m_car;
	e_track	m_track;

	static const int kMaxRunSamples = 100 * 60 * 60 * 1;	// one hour worth of samples

	int				m_run_sample_count;
	c_run_sample *	m_run_samples;

};
