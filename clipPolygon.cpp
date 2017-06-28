// clipPolygon.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <list>

#include "Polygon.h"

static const double EPSILON = 1E-6;

static inline bool epsilonEqual(double a, double b, double eps)
{
	return (::fabs(a - b) < eps);
}

/*
*返回0代表没有相交
*返回1代表1个排除起始点的交点startClip
*返回2代表1个排除起始点的交点endClip
*返回3代表2个排除起始点的交点
*/
short LiangBarskyClip(double xmin, double xmax, double ymin, double ymax, const Double2D& start, const Double2D& end, Double2D& startClip, Double2D& endClip)
{
	bool flag = true;
	double u1 = 0, u2 = 1;
	double p[4], q[4];
	p[0] = -(end.x - start.x);
	p[1] = -p[0];
	p[2] = -(end.y - start.y);
	p[3] = -p[2];
	q[0] = start.x - xmin;
	q[1] = xmax - start.x;
	q[2] = start.y - ymin;
	q[3] = ymax - start.y;
	for (int i = 0; i<4; ++i)
	{
		double u = q[i] / p[i];
		if (p[i] < 0)
		{
			if (u > u2)
				return 0;
			u1 = std::max(u1, u);
		}
		else if (p[i] > 0)
		{
			if (u < u1)
				return 0;
			u2 = std::min(u2, u);
		}
		else if (p[i] == 0 && q[i] < 0)
		{
			return 0;
		}
	}

	startClip.x = start.x + p[1] * u1;
	startClip.y = start.y + p[3] * u1;
	endClip.x = start.x + p[1] * u2;
	endClip.y = start.y + p[3] * u2;

	if (u1 != 0 && u2 != 1)
	{
		return 3;
	}
	else if (u1 != 0)
	{
		return 1;
	}
	else if (u2 != 1)
	{
		return 2;
	}

	return 0;
}

struct pointNode
{
	pointNode()
	{
		next = nullptr;
		gotoNode = nullptr;
		bIn = false;
		bIntersectPoint = 0;
		bUse = false;
	}

	pointNode* CloneContent()
	{
		pointNode* p = new pointNode;
		p->point = this->point;
		p->bIn = this->bIn;
		p->bIntersectPoint = this->bIntersectPoint;
		p->bUse = this->bUse;
		return p;
	}
	Double2D point;
	pointNode* next;
	pointNode* gotoNode;
	short bIntersectPoint;
	bool bIn;
	bool bUse;
};

struct pointNodeList
{
	pointNodeList()
	{
		head = nullptr;
		tail = nullptr;
	}

	void AddNode(pointNode* pNode)
	{
		if (head == nullptr)
		{
			head = pNode;
			tail = pNode;
			//tail->next = nullptr;
		}
		else
		{
			tail->next = pNode;
			tail = pNode;
			//tail->next = nullptr;
		}
	}

	pointNode* head;
	pointNode* tail;
};


enum YAxisDirection
{
	YAxis_Ascend,
	YAxis_Descend
};

//        |        |
//		  |	   1   |	
//        |        |
// -------+--------+-------- ymax
//        |        |
//	  2   |		   | 4
//        |        |
// -------+--------+-------- ymin
//        |        |
//		  |    3   |	
//        |        |
//		xmin	  xmax
short IntersectPointDirection(Double2D& point, double xmin, double xmax, double ymin, double ymax)
{
	if (point.y == ymax)
	{
		return 1;
	}
	else if (point.y == ymin)
	{
		return 3;
	}
	else if (point.x == xmin)
	{
		return 2;
	}
	else if (point.x == xmax)
	{
		return 4;
	}
}

bool PointNodeCompare(pointNode* first, pointNode* second, short direction, bool ascend)
{
	double firstValue = 0;
	double secondValue = 0;
	switch (direction)
	{
	case 1:
	case 3:
		firstValue = first->point.x;
		secondValue = second->point.x;
		break;
	case 2:
	case 4:
		firstValue = first->point.y;
		secondValue = second->point.y;
		break;
	}

	if (firstValue == secondValue)
	{
		return false;
	}

	return (ascend == (firstValue > secondValue));
}

void SortPointNode(pointNodeList* list, short direction, bool ascend)
{
	if (list->head == list->tail)
		return;

	pointNodeList* firstPart = new pointNodeList;
	pointNodeList* secondPart = new pointNodeList;
	pointNode* indexNode = list->head->next;
	while (indexNode)
	{
		if (PointNodeCompare(list->head, indexNode, direction, ascend))
		{
			/*if(firstPart == nullptr)
			{
			firstPart = indexNode;
			firstPartPreNode = indexNode;
			}
			else
			{
			firstPartPreNode->next = indexNode;
			firstPartPreNode = firstPartPreNode->next;
			}*/
			firstPart->AddNode(indexNode);
		}
		else
		{
			/*if(secondPart == nullptr)
			{
			secondPart = indexNode;
			secondPartPreNode = indexNode;
			}
			else
			{
			secondPartPreNode->next = indexNode;
			secondPartPreNode = secondPartPreNode->next;
			}*/
			secondPart->AddNode(indexNode);
		}

		indexNode = indexNode->next;
	}

	//firstPartPreNode->next = head;
	//firstPartPreNode = firstPartPreNode->next;
	firstPart->AddNode(list->head);
	firstPart->tail->next = nullptr;
	if (secondPart->tail)
		secondPart->tail->next = nullptr;

	SortPointNode(firstPart, direction, ascend);
	SortPointNode(secondPart, direction, ascend);

	firstPart->tail->next = secondPart->head;
	list->head = firstPart->head;
	if (secondPart->head)
		list->tail = secondPart->tail;
	else
		list->tail = firstPart->tail;

	delete firstPart;
	delete secondPart;
}



//OGRLinearRing::isClockwise
//与IntersectPointDirection相互影响
bool IsClockwise(iPolygon& in)
{
	int nPointCount = in.size();
	int    i, v, next;
	double  dx0, dy0, dx1, dy1, crossproduct;
	int    bUseFallback = false;

	if (nPointCount < 2)
		return true;

	/* Find the lowest rightmost vertex */
	v = 0;
	for (i = 1; i < nPointCount - 1; i++)
	{
		/* => v < end */
		if (in[i].y< in[v].y ||
			(in[i].y == in[v].y &&
			in[i].x > in[v].x))
		{
			v = i;
		}
	}

	/* previous */
	next = v - 1;
	if (next < 0)
	{
		next = nPointCount - 1 - 1;
	}

	if (epsilonEqual(in[next].x, in[v].x, EPSILON) &&
		epsilonEqual(in[next].y, in[v].y, EPSILON))
	{
		/* Don't try to be too clever by retrying with a next point */
		/* This can lead to false results as in the case of #3356 */
		bUseFallback = true;
	}

	dx0 = in[next].x - in[v].x;
	dy0 = in[next].y - in[v].y;


	/* following */
	next = v + 1;
	if (next >= nPointCount - 1)
	{
		next = 0;
	}

	if (epsilonEqual(in[next].x, in[v].x, EPSILON) &&
		epsilonEqual(in[next].y, in[v].y, EPSILON))
	{
		/* Don't try to be too clever by retrying with a next point */
		/* This can lead to false results as in the case of #3356 */
		bUseFallback = true;
	}

	dx1 = in[next].x - in[v].x;
	dy1 = in[next].y - in[v].y;

	crossproduct = dx1 * dy0 - dx0 * dy1;

	if (!bUseFallback)
	{
		if (crossproduct > 0)      /* CCW */
			return false;
		else if (crossproduct < 0)  /* CW */
			return true;
	}

	/* ok, this is a degenerate case : the extent of the polygon is less than EPSILON */
	/* or 2 nearly identical points were found */
	/* Try with Green Formula as a fallback, but this is not a guarantee */
	/* as we'll probably be affected by numerical instabilities */

	double dfSum = in[0].x * (in[1].y - in[nPointCount - 1].y);

	for (i = 1; i<nPointCount - 1; i++) {
		dfSum += in[i].x * (in[i + 1].y - in[i - 1].y);
	}

	dfSum += in[nPointCount - 1].x * (in[0].y - in[nPointCount - 2].y);

	return dfSum < 0;
}

void clipPolygon(double xmin, double xmax, double ymin, double ymax, YAxisDirection yDirection, iPolygon& in, std::vector<iPolygon>& out)
{
	if ((xmin <= in.xmax && xmax >= in.xmin && ymin <= in.ymax && ymax >= in.ymin) == false)
		return;

	bool bContain = false;
	if (xmin <= in.xmin && xmax >= in.xmax && ymin <= in.ymin && ymax >= in.ymax)
	{
		bContain = true;
	}

	if (bContain == false)
	{
		in.closeRings();

		int inSize = in.size();
		bool haveIntersectPoint = false;
		pointNodeList* polygonNodeList = new pointNodeList;
		pointNode* curNode = nullptr;
		Double2D startClip, endClip;
		for (int i = 0; i < inSize - 1; ++i)
		{
			short flag = LiangBarskyClip(xmin, xmax, ymin, ymax, in[i], in[i + 1], startClip, endClip);

			curNode = new pointNode;
			curNode->point = in[i];
			//curNode->bIn = false;
			//curNode->bIntersectPoint = false;
			polygonNodeList->AddNode(curNode);
			/*if (polygonNodeList->head == nullptr)
			{
			polygonNodeList->head = curNode;
			}*/
			if (flag > 0)
			{
				haveIntersectPoint = true;
			}

			//preNode = curNode;
			switch (flag)
			{
			case 1:
			{
					  //temp.push_back(startClip);
					  curNode = new pointNode;
					  curNode->point = startClip;
					  curNode->bIn = true;
					  curNode->bIntersectPoint = IntersectPointDirection(startClip, xmin, xmax, ymin, ymax);
					  //preNode->next = curNode;
					  //preNode = curNode;
					  polygonNodeList->AddNode(curNode);
					  //intersectPoints.push_back(startClip);
					  break;
			}
			case 2:
			{
					  //temp.push_back(endClip);
					  curNode = new pointNode;
					  curNode->point = endClip;
					  curNode->bIn = false;
					  curNode->bIntersectPoint = IntersectPointDirection(endClip, xmin, xmax, ymin, ymax);
					  //preNode->next = curNode;
					  //preNode = curNode;
					  polygonNodeList->AddNode(curNode);
					  //intersectPoints.push_back(endClip);
					  break;
			}
			case 3:
			{
					  //temp.push_back(startClip);
					  //temp.push_back(endClip);
					  curNode = new pointNode;
					  curNode->point = startClip;
					  curNode->bIn = true;
					  curNode->bIntersectPoint = IntersectPointDirection(startClip, xmin, xmax, ymin, ymax);
					  //preNode->next = curNode;
					  //preNode = curNode;
					  polygonNodeList->AddNode(curNode);

					  curNode = new pointNode;
					  curNode->point = endClip;
					  curNode->bIn = false;
					  curNode->bIntersectPoint = IntersectPointDirection(endClip, xmin, xmax, ymin, ymax);
					  //preNode->next = curNode;
					  //preNode = curNode;
					  polygonNodeList->AddNode(curNode);
					  //intersectPoints.push_back(startClip);
					  //intersectPoints.push_back(endClip);
					  break;
			}
			}
			//temp.push_back(in[i+1]);
			//curNode = new pointNode;
			//curNode->point = in[i+1];
			//curNode->bIn = false;
			//curNode->bIntersectPoint = false;
			//preNode->next = curNode;
			//preNode = curNode;
			//polygonNodeList->AddNode(curNode);
		}
		curNode = new pointNode;
		curNode->point = in[inSize - 1];
		polygonNodeList->AddNode(curNode);

		bool isClockwise = IsClockwise(in);

		if (haveIntersectPoint)
		{
			pointNodeList* topList = new pointNodeList;
			pointNode* topNode = new pointNode;
			topNode->point = Double2D(xmin, ymax);
			topList->AddNode(topNode);

			pointNodeList* leftList = new pointNodeList;
			pointNode* leftNode = new pointNode;
			leftNode->point = Double2D(xmin, ymin);
			leftList->AddNode(leftNode);

			pointNodeList* bottomList = new pointNodeList;
			pointNode* bottomNode = new pointNode;
			bottomNode->point = Double2D(xmax, ymin);
			bottomList->AddNode(bottomNode);

			pointNodeList* rightList = new pointNodeList;
			pointNode* rightNode = new pointNode;
			rightNode->point = Double2D(xmax, ymax);
			rightList->AddNode(rightNode);

			/*pointNode* topPreNode = topNode;
			pointNode* leftPreNode = leftNode;
			pointNode* bottomPreNode = bottomNode;
			pointNode* rightPreNode = rightNode;*/

			pointNode* indexNode = polygonNodeList->head;
			while (indexNode)
			{
				switch (indexNode->bIntersectPoint)
				{
				case 1:
				{
						  pointNode* curNode = indexNode->CloneContent();
						  curNode->gotoNode = indexNode;
						  indexNode->gotoNode = curNode;
						  //topPreNode->next = curNode;
						  //topPreNode = curNode;
						  topList->AddNode(curNode);
						  break;
				}
				case 2:
				{
						  pointNode* curNode = indexNode->CloneContent();
						  curNode->gotoNode = indexNode;
						  indexNode->gotoNode = curNode;
						  //leftPreNode->next = curNode;
						  //leftPreNode = curNode;
						  leftList->AddNode(curNode);
						  break;
				}
				case 3:
				{
						  pointNode* curNode = indexNode->CloneContent();
						  curNode->gotoNode = indexNode;
						  indexNode->gotoNode = curNode;
						  //bottomPreNode->next = curNode;
						  //bottomPreNode = curNode;
						  bottomList->AddNode(curNode);
						  break;
				}
				case 4:
				{
						  pointNode* curNode = indexNode->CloneContent();
						  curNode->gotoNode = indexNode;
						  indexNode->gotoNode = curNode;
						  //rightPreNode->next = curNode;
						  //rightPreNode = curNode;
						  rightList->AddNode(curNode);
						  break;
				}
				}

				indexNode = indexNode->next;
			}

			SortPointNode(topList, 1, isClockwise);
			SortPointNode(leftList, 2, isClockwise);
			SortPointNode(bottomList, 3, !isClockwise);
			SortPointNode(rightList, 4, !isClockwise);

			//MessageBox(0,L"df",0,0);

			pointNodeList* clipList = new pointNodeList;
			if (isClockwise)
			{
				topList->tail->next = rightList->head;
				rightList->tail->next = bottomList->head;
				bottomList->tail->next = leftList->head;
				leftList->tail->next = topList->head;

				clipList->head = topList->head;
				clipList->tail = leftList->tail;
			}
			else
			{
				topList->tail->next = leftList->head;
				leftList->tail->next = bottomList->head;
				bottomList->tail->next = rightList->head;
				rightList->tail->next = topList->head;

				clipList->head = topList->head;
				clipList->tail = rightList->tail;
			}


			//std::vector<pointNode*> results;
			pointNode* polygonIndexNode = polygonNodeList->head;
			pointNode* clipIndexNode = clipList->head;
			//while (polygonIndexNode)
			//{
			//	/*while (clipIndexNode)
			//	{
			//	clipIndexNode = clipIndexNode->next;
			//	}*/
			//	if (polygonIndexNode->bIntersectPoint)
			//		break;
			//	polygonIndexNode = polygonIndexNode->next;
			//}
			pointNode* lastOutNode = nullptr;

			bool startPolygon = false;
			if (xmin <= polygonIndexNode->point.x && xmax >= polygonIndexNode->point.x && ymin <= polygonIndexNode->point.y && ymax >= polygonIndexNode->point.y)
			{
				startPolygon = true;
				out.push_back(iPolygon());
			}
			while (polygonIndexNode)
			{
				if (polygonIndexNode->bIntersectPoint)
				{
					if (startPolygon == false)
					{
						startPolygon = true;
						out.push_back(iPolygon());
					}
					//results.push_back(polygonIndexNode);
					out.back().push_back(Double2D(polygonIndexNode->point));
					if (polygonIndexNode->bUse == false)
					{
						polygonIndexNode->gotoNode->bUse = true;
						polygonIndexNode->bUse = true;

						if (polygonIndexNode->bIn)
						{
							polygonIndexNode = polygonIndexNode->next;
							continue;
						}
						else
						{
							lastOutNode = polygonIndexNode;
							pointNode* clipIndexNode = polygonIndexNode->gotoNode->next;
							bool gotoPolygon = false;
							bool bDone = false;
							while (clipIndexNode/*&&clipIndexNode->bUse==false*/)
							{
								//results.push_back(clipIndexNode);
								out.back().push_back(Double2D(clipIndexNode->point));
								if (clipIndexNode->bIntersectPoint)
								{
									clipIndexNode->gotoNode->bUse = true;
								}

								if (clipIndexNode->bUse)
								{
									bDone = true;
									break;
								}
								if (clipIndexNode->bIn)
								{
									polygonIndexNode = clipIndexNode->gotoNode->next;
									gotoPolygon = true;
									break;
								}
								else
								{
									clipIndexNode = clipIndexNode->next;
								}
							}
							if (gotoPolygon)
								continue;

							if (bDone)
							{
								startPolygon = false;
								//out.push_back(iPolygon());
								polygonIndexNode = lastOutNode->next;
								continue;
							}
						}
					}
					else
						break;
				}
				else
				{
					if (startPolygon)
					{
						//results.push_back(polygonIndexNode);
						out.back().push_back(Double2D(polygonIndexNode->point));
					}
					polygonIndexNode->bUse = true;
					polygonIndexNode = polygonIndexNode->next;
					if (polygonIndexNode == nullptr)
					{
						polygonIndexNode = lastOutNode->next;
						if (polygonIndexNode->bUse)
							break;
						startPolygon = false;
					}
				}
			}

			clipIndexNode = clipList->head;
			while (clipIndexNode)
			{
				pointNode* temp = clipIndexNode->next;
				delete clipIndexNode;
				clipIndexNode = temp;
				if (clipIndexNode == clipList->head)
				{
					//首尾相连的情况
					break;
				}
			}
		}
		else
		{
			//还需要考虑没交点但是完全包含的情况
			{
				//矩形在多边形内
				Double2D point(xmin, ymin);
				bool result = in.IsPointIn(point);
				if (result)
				{
					out.push_back(iPolygon());
					out.back().push_back(Double2D(xmin, ymax));
					out.back().push_back(Double2D(xmax, ymax));
					out.back().push_back(Double2D(xmax, ymin));
					out.back().push_back(Double2D(xmin, ymin));
					out.back().push_back(Double2D(xmin, ymax));
				}
			}
		}

		pointNode* polygonIndexNode = polygonNodeList->head;
		while (polygonIndexNode)
		{
			pointNode* temp = polygonIndexNode->next;
			delete polygonIndexNode;
			polygonIndexNode = temp;
		}
	}
	else
	{
		//矩形包含多边形
		out.push_back(in);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	iPolygon polygon;
	polygon.push_back(Double2D(500, 600));
	polygon.push_back(Double2D(950, 500));
	polygon.push_back(Double2D(1050, 300));
	polygon.push_back(Double2D(950, 300));
	polygon.push_back(Double2D(850, 200));
	polygon.push_back(Double2D(840, 100));
	polygon.push_back(Double2D(740, 200));
	polygon.push_back(Double2D(640, 240));
	polygon.push_back(Double2D(660, 80));
	polygon.push_back(Double2D(440, 340));
	polygon.push_back(Double2D(460, 90));
	polygon.push_back(Double2D(30, 200));
	polygon.push_back(Double2D(30, 600));
	polygon.push_back(Double2D(500, 600));


	double clipRectXmin = 50;
	double clipRectXmax = 950;
	double clipRectYmin = 120;
	double clipRectYmax = 500;

	std::vector<iPolygon> out;
	clipPolygon(clipRectXmin, clipRectXmax, clipRectYmin, clipRectYmax, YAxis_Ascend, polygon, out);

	int size = out.size();

	return 0;
}

