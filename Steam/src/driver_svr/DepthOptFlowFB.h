#pragma once

#include <opencv2/opencv.hpp>
#include "DepthTools.h"

using namespace cv;

class DepthOptFlowFB
{
public:
	DepthOptFlowFB() {}
	~DepthOptFlowFB() {}
	 
	void compute(Mat left, Mat right, Mat &flow);
	void show(Mat src, Mat flow, int gap = 1);
	float GetDepth(Mat flow,int &count, int gap = 1);
private:
	float last_depth = 100;
	const int UNKNOWN_FLOW_THRESH = 1e9;

};
