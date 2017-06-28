#include "stdafx.h"
#include "Polygon.h"


bool iPolygon::IsPointIn(Double2D& pt)
{
	//���ھ��η�Χ��ֱ�ӷ��� 
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
		//	return FALSE/*TRUE*/; //�õ��ڶ���α���

		// ��� y=p.y �� p1p2 �Ľ��� 
		if (p1.y == p2.y) // p1p2 �� y=p0.yƽ�� 
		{
			if (p1.y == pt.y && (pt.x > p1.x || pt.x > p2.x))
			{
				nCross++;
			}
			continue;
		}
		if (pt.y <= std::min(p1.y, p2.y)) // ������p1p2�ӳ����� 
		{
			continue;
		}
		if (pt.y >= std::max(p1.y, p2.y)) // ������p1p2�ӳ����� 
		{
			continue;
		}

		// �󽻵�� X ���� -------------------------------------------------------------- 
		double x = (double)(pt.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
		if (x > pt.x)
			nCross++; // ֻͳ�Ƶ��߽��� 
	}

	//���߽���Ϊ���������ڶ����֮��
	if (nCross % 2 == 1)
		return true;

	return false;
}
