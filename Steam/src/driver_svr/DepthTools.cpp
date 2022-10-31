#include "DepthTools.h"

Scalar DepthTools::genColorMap(float min, float max, float x)
{
	float fx = (max - x) / (max - min);
	if (fx < 0.25)
	{
		return Scalar(255, 255 * (fx - 0) / 0.25, 0);
	}
	else if (fx < 0.5)
	{
		return Scalar(255 * (0.5 - fx) / 0.25, 255, 0);
	}
	else if (fx < 0.75)
	{
		return Scalar(0, 255, 255 * (fx - 0.5) / 0.25);
	}
	else
	{
		return Scalar(0, 255 * (1 - fx) / 0.25, 255);
	}
}

float DepthTools::dispToDepth(float disp)
{
	return 19.62 / disp;
}
