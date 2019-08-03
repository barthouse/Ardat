#pragma once

#include <math.h>
#include <list>

#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)

double scalar_lerp(double a, double b, double lerp);

class Point2d {
public:

	Point2d() : m_x(0.0), m_y(0.0), m_normalized(false) {}
	Point2d(double inX, double inY) : m_x(inX), m_y(inY) , m_normalized(false) {}

	void Subtract(Point2d & in) { m_x -= in.m_x; m_y -= in.m_y; m_normalized = false; }
	void Add(Point2d & in) { m_x += in.m_x; m_y += in.m_y; m_normalized = false; }
	bool Normalize(void) { double magnitude = Magnitude(); if (magnitude!=0.0) { m_x /= magnitude; m_y /= magnitude; m_normalized = true; } else { m_normalized = false; } return m_normalized; }
	double Magnitude(void) { return sqrt((m_x * m_x) + (m_y * m_y)); }
	void Perpendicular(void) { double temp = m_x; m_x = -m_y; m_y = temp; }
	double Dot(const Point2d & in) { return (in.m_x * m_x) + (in.m_y * m_y); }
	void Multiply(double in) { m_x *= in; m_y *= in; m_normalized = false; }
	bool IsNormalized(void) { return m_normalized; }

	double m_x;
	double m_y;

	bool m_normalized;

};

typedef Point2d Vector2d;

typedef std::list<Point2d *> Point2dList;

class Line2d {
public:

	Line2d() : m_valid(false) , m_c(0.0) {}
	Line2d(double inAx, double inAy, double inBx, double inBy) { Set(inAx, inAy, inBx, inBy); }

	bool Set(double inAx, double inAy, double inBx, double inBy);

	bool Intersect(const Line2d & inM, Point2d & outP);

	void PointOnLineNearestPoint(const Point2d & in, Point2d & out);

	bool IsNormalized(void) { return m_n.m_normalized && m_v.m_normalized; }

	bool LinearRegression(Point2dList * inPointList);
	
	// explicit representation
	Vector2d	m_v;
	Point2d		m_u;

	// implicit representation
	Vector2d	m_n;
	double		m_c;

	bool		m_valid;

};
