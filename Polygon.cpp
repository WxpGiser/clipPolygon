#include "stdafx.h"
#include "Polygon.h"


bool iPolygon::IsPointIn(Double2D& pt)
{
	//不在矩形范围内直接返回 
	if ((pt.x >= xmin&&pt.x <= xmax&&pt.y >= ymin&&pt.y <= ymax) == false)
		return false;

	int i, j, iNumber;
	Double2D  p1, p2;
	Double2D	ptInfint;
	iPolygon& polygon = *this;

	int vertexCount = polygon.size();
	int nCross = 0;

	for (j = 0; j < vertexCount - 1; j++)
	{
		p1 = (polygon)[j];
		p2 = polygon[j + 1];

		//if (pt.IsPointInLine(p1,p2) )
		//	return FALSE/*TRUE*/; //该点在多边形边上

		// 求解 y=p.y 与 p1p2 的交点 
		if (p1.y == p2.y) // p1p2 与 y=p0.y平行 
		{
			if (p1.y == pt.y && (pt.x > p1.x || pt.x > p2.x))
			{
				nCross++;
			}
			continue;
		}
		if (pt.y <= std::min(p1.y, p2.y)) // 交点在p1p2延长线上 
		{
			continue;
		}
		if (pt.y >= std::max(p1.y, p2.y)) // 交点在p1p2延长线上 
		{
			continue;
		}

		// 求交点的 X 坐标 -------------------------------------------------------------- 
		double x = (double)(pt.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
		if (x > pt.x)
			nCross++; // 只统计单边交点 
	}

	//单边交点为奇数，点在多边形之内
	if (nCross % 2 == 1)
		return true;

	return false;
}
