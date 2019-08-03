#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <map>

#include "..\common\math_util.h"

typedef std::map<double, Point2dList *> Point2dListMap;
typedef std::pair<double, Point2dList *> Point2dListMapValue;

void Process(FILE * inputFile, FILE * outputMarkerFile, FILE * outputInitializerFile)
{
	// build up list of points base on track percentage
	double track_percentage, x, y, z;

	Point2dListMap point2dListMap;

	while (1)
	{
		int fieldCount = fscanf(inputFile, "%lf %lf %lf %lf", &track_percentage, &x, &y, &z);

		if (fieldCount == 4)
		{
			Point2d * point2d = new Point2d(x, y);
			Point2dListMap::iterator i = point2dListMap.find(track_percentage);
			Point2dList * point2dList = NULL;

			if (i != point2dListMap.end())
			{
				point2dList = (*i).second;
			}
			else
			{
				point2dList = new Point2dList();

				Point2dListMapValue point2dListMapValue(track_percentage, point2dList);
				point2dListMap.insert(point2dListMapValue);
			}

			point2dList->push_back(point2d);
		}

		if (fieldCount == EOF)
			break;
	}

	Point2dListMap::iterator i = point2dListMap.begin();

	while(i != point2dListMap.end())
	{
		double track_percentage = (*i).first;
		Point2dList * point2dList = (*i).second;
		Line2d line2d;

		if(line2d.LinearRegression(point2dList))
		{
			Point2dList::iterator i = point2dList->begin();

			if (i != point2dList->end())
			{
				double min_x, min_y, max_x, max_y;
				Point2d * point = (*i);

				min_x = max_x = point->m_x;
				min_y = max_y = point->m_y;

				while(i != point2dList->end())
				{
					point = (*i);

					if (point->m_x < min_x) min_x = point->m_x;
					if (point->m_x > max_x) max_x = point->m_x;
					if (point->m_y < min_y) min_y = point->m_y;
					if (point->m_y > max_y) max_y = point->m_y;

					i++;
				}

				double c_x = (max_x + min_x) / 2;
				double c_y = (max_y + min_y) / 2;

				Point2d center(c_x, c_y);
				Point2d centerOnLine;

				line2d.PointOnLineNearestPoint(center,  centerOnLine);

				Vector2d width = line2d.m_v;
				width.Multiply(5.0);	// 5 wide

				Point2d a = centerOnLine;
				a.Add(width);

				Point2d b = centerOnLine;
				b.Subtract(width);

				fprintf(outputInitializerFile, "{ %f , %f , %f , %f , %f } ,\n", track_percentage, a.m_x, a.m_y, b.m_x, b.m_y);
				fprintf(outputMarkerFile, "\"%f\" %f %f\n\"%f\" %f %f\n\n", track_percentage, a.m_x, a.m_y, track_percentage,  b.m_x, b.m_y);

			}
		}

		i++;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 4)
	{
		printf("usage: %s <path file> <marker plot file> <marker initializer file>\n", argv[0]);
	}
	else
	{
		FILE * inputFile = fopen(argv[1], "r");

		if (inputFile != NULL)
		{
			FILE * outputPlotFile = fopen(argv[2], "w+");

			if (outputPlotFile != NULL) 
			{
				FILE * outputInitializerFile = fopen(argv[3], "w+");

				if (outputInitializerFile != NULL) 
				{
					Process(inputFile, outputPlotFile, outputInitializerFile);
				}
			}
			else
			{
				printf("unable to open '%s' for writing\n", argv[2]);
			}
		}
		else
		{
			printf("unable to open '%s' for reading\n", argv[1]);
		}
	}

	return 0;
}

