#pragma once

#include <opencv2/opencv.hpp>
using namespace cv;

class DepthTools
{
public:

	DepthTools() {}
	~DepthTools() {}

	static float dispToDepth(float disp);

	static Scalar genColorMap(float min, float max, float x);
};
