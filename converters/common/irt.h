#pragma once

typedef struct _irt_header_t
{
	char	m_track[128];
	char	m_car[128];
} irt_header_t;

typedef struct _irt_sampe_t {
	double	m_time;
	float	m_track_percentage;
	float	m_velocity[3];
	float	m_orientation[3];
	float	m_angular_velocity[3];
	float	m_rpm;
	float	m_brake;
	float	m_clutch;
	int		m_gear;
	float	m_steer;
	float	m_throttle;
	float	m_steering_torque;
} irt_sample_t;

class irt_samples_t
{
public:

	irt_samples_t();

	void load(char * inFilename);

	int get_sample_count(void);
	irt_sample_t * get_sample(int inIndex);

	int get_lap_count(void);

	void plot(char * inFilename);

private:

	int				m_sample_count;
	irt_sample_t * 	m_samples;
	irt_header_t	m_header;

};

