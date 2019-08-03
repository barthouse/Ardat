#include "math_util.h"

double scalar_lerp(double a, double b, double lerp)
{
	double delta = b - a;
	return a + (lerp * delta);
}

bool Line2d::Set(double inAx, double inAy, double inBx, double inBy)
{
	m_u.m_x = inAx;
	m_u.m_y = inAy;

	m_v.m_x = inBx - inAx;
	m_v.m_y = inBy - inAy;

	if (m_v.Normalize())
	{	
		m_n = m_v;
		m_n.Perpendicular();
		
		m_c = -m_n.Dot(m_u);

		m_valid = true;
	}
	else
	{
		m_valid = false;
	}

	return m_valid;
}

bool Line2d::Intersect(const Line2d & inM, Point2d & outP)
{
	bool intersect = false;

	if (m_valid && inM.m_valid)
	{
		double d = m_n.Dot(inM.m_v);

		if (d != 0.0)
		{
			Vector2d temp = inM.m_v;
			double n = m_n.Dot(inM.m_u);
			n += m_c;
			double t = - (n / d);

			temp.Multiply(t);

			outP = inM.m_u;
			outP.Add(temp);
			
			intersect = true;
		}
	}

	return intersect;
}

void Line2d::PointOnLineNearestPoint(const Point2d & in, Point2d & out)
{
	double q = m_c + (m_n.Dot(in));

	if (!IsNormalized())
	{
		q /= m_n.Magnitude();
	}

	Point2d temp = m_n;
	temp.Multiply(q);

	out = in;
	out.Subtract(temp);
}


bool Line2d::LinearRegression(Point2dList * inLineList)
{
	double sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0;
	double n = 0;

	double minx, miny, maxx, maxy;

	Point2dList::iterator i = inLineList->begin();

	if (i != inLineList->end())
	{
		Point2d * point = (*i);

		minx = maxx = point->m_x;
		miny = maxy = point->m_y;

		while(i != inLineList->end())
		{
			Point2d * point = (*i);

			sx += point->m_x;
			sy += point->m_y;
			sxx += point->m_x * point->m_x;
			syy += point->m_y * point->m_y;
			sxy += point->m_y * point->m_x;
			n += 1.0f;

			if (point->m_x < minx) minx = point->m_x;
			if (point->m_x > minx) maxx = point->m_x;

			if (point->m_y < miny) miny = point->m_y;
			if (point->m_y > miny) maxy = point->m_y;

			i++;
		}
	}

	double delta_x = maxx - minx;
	double delta_y = maxy - miny;

	if (delta_x > delta_y)
	{
		double beta_divisor = (n * sxx) - (sx * sx);

		if (beta_divisor != 0.0f)
		{
			double beta = ((n * sxy) - (sx * sy)) / beta_divisor;
			double alpha = (sy / n) - ((beta * sx) / n);

			// y = alpha + (beta * x)
			//     x = 0, y = alpha
			//     x = 1, y = alpha + beta

			Set(0, alpha, 1, (alpha + beta));
		}
		else
		{
			m_valid = false;
		}
	}
	else
	{
		double beta_divisor = (n * syy) - (sy * sy);

		if (beta_divisor != 0.0f)
		{
			double beta = ((n * sxy) - (sx * sy)) / beta_divisor;
			double alpha = (sx / n) - ((beta * sy) / n);

			// x = alpha + (beta * y)
			//     y = 0, x = alpha
			//     y = 1, x = alpha + beta

			Set(alpha, 0, (alpha + beta), 1);
		}
		else
		{
			m_valid = false;
		}
	}

	return m_valid;
}
