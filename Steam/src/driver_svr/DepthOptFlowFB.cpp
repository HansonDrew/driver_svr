#include "DepthOptFlowFB.h"
#include "driverlog.h"
void DepthOptFlowFB::compute(Mat left, Mat right, Mat &flow)
{
	try
	{
		calcOpticalFlowFarneback(left, right, flow, 0.5, 3, 15, 3, 5, 1.1, 0);
	}
	catch (...)
	{
		DriverLog("exception");
	}
	 
	
}

void DepthOptFlowFB::show(Mat src, Mat flow, int gap)
{
	Mat dst;
	cvtColor(src, dst, COLOR_GRAY2BGR);

	int count1 = 0, count2 = 0, count3 = 3;
	for (int i = 0; i < flow.rows; ++i)
	{
		for (int j = 0; j < flow.cols; ++j)
		{
			if (i % gap == 0 && j % gap == 0)
			{
				Vec2f flow_at_point = flow.at<Vec2f>(i, j);
				float fx = flow_at_point[0];
				float fy = flow_at_point[1];
				if ((fabs(fx) > UNKNOWN_FLOW_THRESH) || (fabs(fy) > UNKNOWN_FLOW_THRESH))
				{
					count1++;
					continue;
				}
				if (fabs(fy) > 1)
				{
					count2++;
					continue;
				}
				if (fx > -2)
				{
					count3++;
					continue;
				}

				float depth = DepthTools::dispToDepth(fabs(fx));
				Scalar color = DepthTools::genColorMap(0, 10, depth);

				circle(dst, Point(j, i), 2, color);
			}
		}
	}
	std::cout << "count1:" << count1 << std::endl;
	std::cout << "count2:" << count2 << std::endl;
	std::cout << "count3:" << count3 << std::endl;

	imshow("OPTFB dst", dst);
}
float DepthOptFlowFB::GetDepth( Mat flow, int& count, int gap)
{

	int count1 = 0, count2 = 0, count3 = 3;
	count = 0;
	float all_depht = 0;
	for (int i = 0; i < flow.rows; ++i)
	{
		for (int j = 0; j < flow.cols; ++j)
		{
			if (i % gap == 0 && j % gap == 0)
			{
				Vec2f flow_at_point = flow.at<Vec2f>(i, j);
				float fx = flow_at_point[0];
				float fy = flow_at_point[1];
				if ((fabs(fx) > UNKNOWN_FLOW_THRESH) || (fabs(fy) > UNKNOWN_FLOW_THRESH))
				{
					count1++;
					continue;
				}
				if (fabs(fy) > 1)
				{
					count2++;
					continue;
				}
				if (fx > -2)
				{
					count3++;
					continue;
				}

				float depth = DepthTools::dispToDepth(fabs(fx));
				all_depht = all_depht + depth;
				count++;
			}
		}
	}
	float ret = 0;
	if (count==0)
	{
		ret = last_depth;
	}
	else 
	{
		ret = all_depht / (float)count;
		last_depth = ret;
	}
	return ret;
}
