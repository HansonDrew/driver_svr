//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once

#include <openvr_driver.h>
#include "RVRPluginDefinitions.h"
#define PI       3.14159265358979323846   // pi
#define DEGREES_TO_RADIANS 0.01745329251f
void set_bounds(float* texBounds, const vr::VRTextureBounds_t* bounds);
void HmdMatrix_QuatToMat(double w, double x, double y, double z, vr::HmdMatrix34_t *pMatrix);
int64_t nowInNs(void);
int64_t nowInUs(void);
int64_t nowInMs(void);
vr::HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z);
void ExtractRotation(RVR::RVRQuaternion* rotation, const vr::HmdMatrix34_t *pPose);
void ExtractPosition(RVR::RVRVector3* position, const vr::HmdMatrix34_t *pPose);
float GetDistance(RVR::RVRQuaternion* q1, RVR::RVRQuaternion* q2);
void ExtractDriverPose(vr::DriverPose_t* pose, RVR::RVRPoseHmdData* data);

void ExtractRVRPoseHmdDataSimple(RVR::RVRPoseHmdData* poseData, const vr::HmdMatrix34_t *pPose);
void ExtractRVRPoseHmdData( RVR::RVRPoseHmdData* out_data,  const vr::HmdMatrix34_t* in_pPose);
void ExtractRVRPoseHmdData(vr::DriverPose_t* pose, RVR::RVRPoseHmdData* data);
void ExtractRVRControllerPoseData(vr::DriverPose_t* pose, RVR::RVRControllerData* data);
void RotateTranslateVector3(RVR::RVRQuaternion rotation, RVR::RVRVector3 vec3_in, RVR::RVRVector3 &vec3_out);
void ChangeRotation(RVR::RVRQuaternion& rotation, RVR::RVRQuaternion increment);
void GetSubAngles(RVR::RVRQuaternion rotation1, RVR::RVRQuaternion rotation2, double& pitch, double& yaw, double& roll);
void ChangePosition(RVR::RVRQuaternion origin_rotation, RVR::RVRVector3& origin_position, RVR::RVRVector3 increment_position);

void EularAnglesToRotation(double pitch, double yaw ,double roll, RVR::RVRQuaternion& rotation);// pitch yaw roll»∆ x, y ,z 

void RotationToEularAngles( RVR::RVRQuaternion rotation, double &pitch, double &yaw, double &roll);// pitch yaw roll»∆ x, y ,z 

void RotationToEularAngles2(float rotation[4], double& pitch, double& yaw, double& roll);// pitch yaw roll»∆ x, y ,z 

void  GetRotationFromQuat1ToQuat2(float* quat1, float* quat2, float* rotation, float* rotation_inv);
void ConvertQuatToMatrix(float input[4], float* mat);
#define FLT_EQUAL(f1,f2)   (fabs(fabs(f1)-fabs(f2)) <= FLT_ZERO)
#define FLT_ZERO   (float)0.000001

#define FLT_EQUAL_2(f1,f2)   (fabs(fabs(f1)-fabs(f2)) <= FLT_ZERO_2)
#define FLT_ZERO_2   (float)0.000002


#define FLT_EQUAL_3(f1,f2)   (fabs(fabs(f1)-fabs(f2)) <= FLT_ZERO_3)
#define FLT_ZERO_3   (float)0.000051
