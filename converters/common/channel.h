#pragma once

#include <vector>
#include <assert.h>

template <class t>
class channel {
public:

    channel(double step, double startTime) : m_step(step), m_startTime(startTime)
    {
        // do nothing
    }

    void add(t & data) { m_data.push_back(data); }
    t get(int i) { return m_data.operator[i]; }

private:

    double m_step;
    double m_startTime;
    std::vector<t> m_data;

};

class vector3 {
public:
    vector3(double x, double y, double z)
    {
        m_data[0] = x;
        m_data[1] = y;
        m_data[2] = z;
    }

    double operator[](int i)
    {
        assert(i >= 0 && i < 3);
        return m_data[i];
    }

private:
    double m_data[3];

};

class gps_position : public vector3 {
public:
    gps_position(double longitude, double latitude, double height) :
        vector3(longitude, latitude, height)
    {
        // do nothing
    }

    double get_longitude() { return (*this)[0]; }
    double get_latitude() { return (*this)[1]; }
    double get_height() { return (*this)[2]; }

};

class gps_channel : public channel<gps_position>
{
    gps_channel(double step, double startTime) : channel<gps_position>(step, startTime)
    {
        // do nothing
    }
};