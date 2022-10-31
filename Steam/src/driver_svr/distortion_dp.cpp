 
#include "distortion_dp.h"
#include "driverlog.h"
 float DistortionFnScaleRadiusSquared (float rsq)
{
    float scale = 1.0f;
   
        scale =
             DISTORTION_C10_K_0+
             (DISTORTION_C10_K_1)*rsq+
             (DISTORTION_C10_K_2)*rsq*rsq+
             (DISTORTION_C10_K_3)*rsq*rsq*rsq+
             (DISTORTION_C10_K_4)*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_5)*rsq*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_6)*rsq*rsq*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_7)*rsq*rsq*rsq*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_8)*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_9)*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq+
             (DISTORTION_C10_K_10)*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq*rsq;

    return scale;
}

 Vector3float DistortionFnScaleRadiusSquaredChroma (float rsq)
{
    float scale = DistortionFnScaleRadiusSquared ( rsq );
	Vector3float scaleRGB;

    scaleRGB.x = scale * ( 1.0f + (CHROMATIC_ABERRATION_0)
                + rsq *  (CHROMATIC_ABERRATION_1)
                + sqrt(rsq) *  (CHROMATIC_ABERRATION_4));                      // Red

    scaleRGB.y = scale;                                                               // Green

    scaleRGB.z =  scale * ( 1.0f + (CHROMATIC_ABERRATION_2)
                + rsq *  (CHROMATIC_ABERRATION_3)
                + sqrt(rsq) *  (CHROMATIC_ABERRATION_5));                      // Blue

    return scaleRGB;
}

 void WarpTexCoordChromaMode0( const float in[2], float red[2],
		float green[2], float blue[2]) {

    float theta[2];
    for (int i = 0; i < 2; i++) {
        const float unit = in[i];
        const float ndc = 2.0f * (unit - 0.5f);
        const float pixels = ndc * HEIGHT_PIXELS * 0.5f;
        const float meters = pixels * WIDTH_METERS / WIDTH_PIXELS;
        const float tanAngle = meters / METERS_PER_TAN_ANGLE_AT_CENTER;
        theta[i] = tanAngle;
    }

    const float rsq = theta[0] * theta[0] + theta[1] * theta[1];

    const Vector3float chromaScale =
            DistortionFnScaleRadiusSquaredChroma(rsq);

    for (int i = 0; i < 2; i++) {
        red[i] = chromaScale.x * theta[i] / 2 + 0.5f;
        green[i] = chromaScale.y * theta[i] / 2 + 0.5f;
        blue[i] = chromaScale.z * theta[i] / 2 + 0.5f;
    }
}
 //pixels_offset_buffer 0 = 3.413012, 1 = -2.071015, meters_offset_buffer0 = 0.000106, 1 = -0.000064
 //pixels_offset_buffer 0=-4.990203,1=-3.400909,meters_offset_buffer0=-0.000155,1=-0.000106
 void WarpTexCoordChromaModeOpticalOffset(int eye, const float in[2], const float offset[2], float red[2], float green[2], float blue[2])
 {
	  
	 float theta[2];
	 float theta_offset[2];
	 float pixels_offset_buffer[2];
	 float meters_offset_buffer[2];
	 for (int i = 0; i < 2; i++) {
		 const float unit_offset = offset[i];
		 const float unit = in[i] + unit_offset;
		 const float ndc = 2.0f * (unit - 0.5f);
		 const float ndc_offset = 2.0f * unit_offset;
		 const float pixels = ndc * HEIGHT_PIXELS * 0.5f;
		 const float pixels_offset = ndc_offset * HEIGHT_PIXELS * 0.5f;
		 const float meters = pixels * WIDTH_METERS / WIDTH_PIXELS;
		 const float meters_offset = pixels_offset * WIDTH_METERS / WIDTH_PIXELS;
		 const float tanAngle = meters / METERS_PER_TAN_ANGLE_AT_CENTER;
		 const float tanAngle_offset = meters_offset / METERS_PER_TAN_ANGLE_AT_CENTER;
		 theta[i] = tanAngle;
		 theta_offset[i] = tanAngle_offset;
	 }

	 const float rsq = theta[0] * theta[0] + theta[1] * theta[1];

	 const Vector3float chromaScale =
		 DistortionFnScaleRadiusSquaredChroma(rsq);
	 //    float scale = calculateScale();
	 //    LOGIF("WarpTexCoordChromaModeOpticalOffset scale is %f",scale);
	 for (int i = 0; i < 2; i++) {
		 red[i] = /*scale * */(chromaScale.x * theta[i] - theta_offset[i]);
		 green[i] = /*scale * */(chromaScale.y* theta[i] - theta_offset[i]);
		 blue[i] = /*scale * */(chromaScale.z * theta[i] - theta_offset[i]);

		 red[i] = red[i]/2+0.5f;
		 green[i] = green[i] / 2 + 0.5f;
		 blue[i] = blue[i] / 2 + 0.5f;

	 }
 }

 void WarpTexCoordChromaMode0new(const float in[2], float red[2],
	 float green[2], float blue[2]) {

	 float theta[2];
	 for (int i = 0; i < 2; i++) {

		 const float ndc = 2.0f * (in[i] - 0.5f);
		 const float pixels = ndc * HEIGHT_PIXELS * 0.5f;
		 const float meters = pixels * WIDTH_METERS / WIDTH_PIXELS;
		 const float tanAngle = meters / METERS_PER_TAN_ANGLE_AT_CENTER;
		 theta[i] = tanAngle;
	 }
	 const float rsq = theta[0] * theta[0] + theta[1] * theta[1];
	 const Vector3float chromaScale =
		 DistortionFnScaleRadiusSquaredChroma(rsq);
	 for (int i = 0; i < 2; i++) {
		 red[i] = chromaScale.x * theta[i];
		 green[i] = chromaScale.y * theta[i];
		 blue[i] = chromaScale.z * theta[i];
	 }
	 for (int i = 0; i < 2; i++)
	 {
		 float meters;
		 float pixels;
		 float ndc = 1;
		 meters = red[i] * METERS_PER_TAN_ANGLE_AT_CENTER;
		 pixels = meters / WIDTH_METERS * WIDTH_PIXELS;
		 ndc = pixels / 0.5f / HEIGHT_PIXELS;
		 red[i] = ndc / 2.0f + 0.5f;
	 }
	 for (int i = 0; i < 2; i++)
	 {
		 float meters;
		 float pixels;
		 float ndc = 1;
		 meters = green[i] * METERS_PER_TAN_ANGLE_AT_CENTER;
		 pixels = meters / WIDTH_METERS * WIDTH_PIXELS;
		 ndc = pixels / 0.5f / HEIGHT_PIXELS;
		 green[i] = ndc / 2.0f + 0.5f;
	 }
	 for (int i = 0; i < 2; i++)
	 {
		 float meters;
		 float pixels;
		 float ndc = 1;
		 meters = blue[i] * METERS_PER_TAN_ANGLE_AT_CENTER;
		 pixels = meters / WIDTH_METERS * WIDTH_PIXELS;
		 ndc = pixels / 0.5f / HEIGHT_PIXELS;
		 blue[i] = ndc / 2.0f + 0.5f;
	 }
 }

 void WarpTexCoordChromaModeNdc(const float in_ndc[2], float red[2], float green[2], float blue[2]) 
 {

	 float theta[2];
	 for (int i = 0; i < 2; i++) {
		 //const float unit = in[i];
		 //const float ndc = 2.0f * (unit - 0.5f);
		 const float ndc = in_ndc[i];
		 const float pixels = ndc * HEIGHT_PIXELS * 0.5f;
		 const float meters = pixels * WIDTH_METERS / WIDTH_PIXELS;
		 const float tanAngle = meters / METERS_PER_TAN_ANGLE_AT_CENTER;
		 theta[i] = tanAngle;
	 }

	 const float rsq = theta[0] * theta[0] + theta[1] * theta[1];

	 const Vector3float chromaScale =
		 DistortionFnScaleRadiusSquaredChroma(rsq);

	 for (int i = 0; i < 2; i++) {
		 red[i] = chromaScale.x * theta[i];
		 green[i] = chromaScale.y * theta[i];
		 blue[i] = chromaScale.z * theta[i];
	 }
 }




