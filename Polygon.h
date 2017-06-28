#pragma once
#include <vector>
#include <algorithm>

struct Double2D{
	Double2D()
	{
		y = 0;
		x = 0;
	}
	Double2D(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Double2D operator + (Double2D& p0)
	{
		return Double2D((this->y + p0.y), (this->x + p0.x));
	}

	Double2D operator - (Double2D& p0)
	{
		return Double2D((this->y - p0.y), (this->x - p0.x));
	}

	Double2D operator / (float f)
	{
		float fInv = 1.0f / f;
		return Double2D(this->y * fInv, this->x * fInv);
	}

	Double2D operator * (float f)
	{
		return Double2D(this->y * f, this->x * f);
	}

	bool operator == (Double2D& p)
	{
		if (y == p.y && x == p.x)
			return true;
		else
			return false;

	}

	bool operator != (Double2D& p)
	{
		if (y == p.y && x == p.x)
			return false;
		else
			return true;

	}

	double x;
	double y;
};


class iPolygon : public std::vector<Double2D>
{
public:
	iPolygon()
		:std::vector<Double2D>()
	{
			xmin = DBL_MAX;
			xmax = -DBL_MAX;
			ymin = DBL_MAX;
			ymax = -DBL_MAX;
		}
	void push_back(Double2D& d2d)
	{
		std::vector<Double2D>::push_back(d2d);

		xmin = (std::min)(xmin, d2d.x);
		xmax = (std::max)(xmax, d2d.x);
		ymin = (std::min)(ymin, d2d.y);
		ymax = (std::max)(ymax, d2d.y);
	}
	bool IsPointIn(Double2D& pt);

	void closeRings()
	{
		bool r = (
			std::vector<Double2D>::front().x == std::vector<Double2D>::back().x
			&&std::vector<Double2D>::front().y == std::vector<Double2D>::back().y);
		if (r == false)
		{
			std::vector<Double2D>::push_back(std::vector<Double2D>::front());
		}
	}

	double xmin;
	double xmax;
	double ymin;
	double ymax;
};

