#pragma once
 

#define DISTORTION_C10_K_0  1.0
#define DISTORTION_C10_K_1  0.346444
#define DISTORTION_C10_K_2  0.548571
#define DISTORTION_C10_K_3  0.012183
#define DISTORTION_C10_K_4  0.321964
#define DISTORTION_C10_K_5  0.808450
#define DISTORTION_C10_K_6  1.781837
#define DISTORTION_C10_K_7  0.000025
#define DISTORTION_C10_K_8  0.725699
#define DISTORTION_C10_K_9  0.739226
#define DISTORTION_C10_K_10  0.752608

#define CHROMATIC_ABERRATION_0  -0.002894
#define CHROMATIC_ABERRATION_1  -0.007057
#define CHROMATIC_ABERRATION_2  0.006197
#define CHROMATIC_ABERRATION_3  0.014590
#define CHROMATIC_ABERRATION_4  -0.007586
#define CHROMATIC_ABERRATION_5  0.015290

#define WIDTH_PIXELS 3664
#define HEIGHT_PIXELS  1920
#define WIDTH_METERS  0.1203624f
#define HEIGHT_METERS 0.06307f
#define METERS_PER_TAN_ANGLE_AT_CENTER   0.040326f

#define HORIZONTAL_OFFSET_METERS  0.0

#define LENS_SEPARATION   0.0634 

class Vector3float 
{
public:
	float x;
	float y;
	float z;
	Vector3float() 
	{
		x = y = z = 0;
	}
	~Vector3float() {};
};

Vector3float DistortionFnScaleRadiusSquaredChroma(float rsq);
void WarpTexCoordChromaMode0(const float in[2], float red[2], float green[2], float blue[2]);
float DistortionFnScaleRadiusSquared(float rsq); 
void WarpTexCoordChromaModeOpticalOffset(int eye,const float in[2], const float offset[2], float red[2], float green[2], float blue[2]);
void WarpTexCoordChromaMode0new(const float in[2], float red[2],float green[2], float blue[2]);

void WarpTexCoordChromaModeNdc(const float in_ndc[2], float red[2], float green[2], float blue[2]);