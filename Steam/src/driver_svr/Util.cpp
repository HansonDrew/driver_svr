//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include "Util.h"

#include <openvr_driver.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include "../../Eigen/Geometry"
#include "../../Eigen/Core"
using namespace vr;
extern float gHmdTimeOffset;
//-----------------------------------------------------------------------------
void set_bounds(float* texBounds, const vr::VRTextureBounds_t* bounds)
//-----------------------------------------------------------------------------
{
    if (bounds != nullptr)
    {
        texBounds[0] = bounds->uMin;
        texBounds[1] = bounds->uMax;
        texBounds[2] = bounds->vMin;
        texBounds[3] = bounds->vMax;
    }
    else {
        texBounds[0] = 0.0f;
        texBounds[1] = 1.0f;
        texBounds[2] = 0.0f;
        texBounds[3] = 1.0f;
    }
}

//-----------------------------------------------------------------------------
void HmdMatrix_QuatToMat(double w, double x, double y, double z, vr::HmdMatrix34_t *pMatrix)
//-----------------------------------------------------------------------------
{
    pMatrix->m[0][0] = (float)(1.0f - 2.0f * y * y - 2.0f * z * z);
    pMatrix->m[0][1] = (float)(2.0f * x * y - 2.0f * z * w);
    pMatrix->m[0][2] = (float)(2.0f * x * z + 2.0f * y * w);
    pMatrix->m[0][3] = (float)(0.0f);
    pMatrix->m[1][0] = (float)(2.0f * x * y + 2.0f * z * w);
    pMatrix->m[1][1] = (float)(1.0f - 2.0f * x * x - 2.0f * z * z);
    pMatrix->m[1][2] = (float)(2.0f * y * z - 2.0f * x * w);
    pMatrix->m[1][3] = (float)(0.0f);
    pMatrix->m[2][0] = (float)(2.0f * x * z - 2.0f * y * w);
    pMatrix->m[2][1] = (float)(2.0f * y * z + 2.0f * x * w);
    pMatrix->m[2][2] = (float)(1.0f - 2.0f * x * x - 2.0f * y * y);
    pMatrix->m[2][3] = (float)(0.0f);
}

//-----------------------------------------------------------------------------
int64_t nowInNs(void)
//-----------------------------------------------------------------------------
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto nowInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return nowInNs;
}

//-----------------------------------------------------------------------------
int64_t nowInUs(void)
//-----------------------------------------------------------------------------
{
    return nowInNs() / 1000;
}

//-----------------------------------------------------------------------------
int64_t nowInMs(void)
//-----------------------------------------------------------------------------
{
    return nowInUs() / 1000;
}

//-----------------------------------------------------------------------------
HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
//-----------------------------------------------------------------------------
{
    HmdQuaternion_t quat;
    quat.w = w;
    quat.w = w;
    quat.x = x;
    quat.y = y;
    quat.z = z;
    return quat;
}

//-----------------------------------------------------------------------------
float GetDistance(RVR::RVRQuaternion* q1, RVR::RVRQuaternion* q2)
//-----------------------------------------------------------------------------
{
    glm::quat _q1(q1->w, q1->x, q1->y, q1->z);
    glm::quat _q2(q2->w, q2->x, q2->y, q2->z);
    float dot = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w*q2->w;
    return fabs(dot - 1.0f);
}

//-----------------------------------------------------------------------------
void ExtractRotation(RVR::RVRQuaternion* rotation, const vr::HmdMatrix34_t *pPose)
//-----------------------------------------------------------------------------
{
    glm::mat3 rotationMatrix;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rotationMatrix[i][j] = pPose->m[i][j];
        }
    }

    glm::quat rotationQuat = glm::quat_cast(rotationMatrix);

    rotation->x = rotationQuat.x;
    rotation->y = rotationQuat.y;
    rotation->z = rotationQuat.z;
    rotation->w = rotationQuat.w;
}

//-----------------------------------------------------------------------------
void ExtractPosition(RVR::RVRVector3* position, const vr::HmdMatrix34_t *pPose)
//-----------------------------------------------------------------------------
{
    position->x = -pPose->m[0][3];
    position->y = -pPose->m[1][3];
    position->z = -pPose->m[2][3];
}

//-----------------------------------------------------------------------------
void ExtractDriverPose(vr::DriverPose_t* pose, RVR::RVRPoseHmdData* data)
//-----------------------------------------------------------------------------
{
    pose->poseIsValid = true;
    pose->result = TrackingResult_Running_OK;
    pose->deviceIsConnected = data->valid;

    pose->qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose->qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    ////Position
    pose->vecPosition[0] = data->position.x;
    pose->vecPosition[1] = data->position.y;
    pose->vecPosition[2] = data->position.z;

    //Orientation
    pose->qRotation.w = data->rotation.w;
    pose->qRotation.x = data->rotation.x;
    pose->qRotation.y = data->rotation.y;
    pose->qRotation.z = data->rotation.z;
    /*float r = sqrt(data->rotation.w*data->rotation.w + data->rotation.x*data->rotation.x +
        pose->qRotation.y* pose->qRotation.y + pose->qRotation.z*pose->qRotation.z);
    if (r < 0.0001)
        r = 0.0001;
*/
    pose->vecVelocity[0] = data->linearVelocity.x;
    pose->vecVelocity[1] = data->linearVelocity.y;
    pose->vecVelocity[2] = data->linearVelocity.z;

    pose->vecAcceleration[0] = data->linearAcceleration.x;
    pose->vecAcceleration[1] = data->linearAcceleration.y;
    pose->vecAcceleration[2] = data->linearAcceleration.z;

    pose->vecAngularVelocity[0] = data->angularVelocity.x;
    pose->vecAngularVelocity[1] = data->angularVelocity.y;
    pose->vecAngularVelocity[2] = data->angularVelocity.z;

    pose->vecAngularAcceleration[0] = 0;
    pose->vecAngularAcceleration[1] = 0;
    pose->vecAngularAcceleration[2] = 0;

    pose->poseTimeOffset = gHmdTimeOffset;
    pose->shouldApplyHeadModel = false;
    pose->willDriftInYaw = false;
}

//-----------------------------------------------------------------------------
void ExtractRVRPoseHmdData(vr::DriverPose_t* pose, RVR::RVRPoseHmdData* data)
//-----------------------------------------------------------------------------
{
    
    data->valid = pose->deviceIsConnected;
 
    ////Position
    data->position.x = pose->vecPosition[0];
    data->position.y = pose->vecPosition[1];
    data->position.z = pose->vecPosition[2];

    //Orientation
    data->rotation.w = pose->qRotation.w;
    data->rotation.x = pose->qRotation.x;
    data->rotation.y = pose->qRotation.y;
    data->rotation.z = pose->qRotation.z;
    /*float r = sqrt(data->rotation.w*data->rotation.w + data->rotation.x*data->rotation.x +
        pose->qRotation.y* pose->qRotation.y + pose->qRotation.z*pose->qRotation.z);
    if (r < 0.0001)
        r = 0.0001;
*/
    data->linearVelocity.x = pose->vecVelocity[0];
    data->linearVelocity.y = pose->vecVelocity[1];
    data->linearVelocity.z = pose->vecVelocity[2];

    data->linearAcceleration.x = pose->vecAcceleration[0];
    data->linearAcceleration.y = pose->vecAcceleration[1];
    data->linearAcceleration.z = pose->vecAcceleration[2];

    data->angularVelocity.x = pose->vecAngularVelocity[0];
    data->angularVelocity.y = pose->vecAngularVelocity[1];
    data->angularVelocity.z = pose->vecAngularVelocity[2];

    data->angularAcceleration.x = 0;
    data->angularAcceleration.y= 0;
    data->angularAcceleration.z = 0;

    
}

void ExtractRVRControllerPoseData(vr::DriverPose_t* pose, RVR::RVRControllerData* data) 
{
    int connect_state =(int) RVR::RVRControllerConnectionState::kConnected;
    if (pose->deviceIsConnected)
    {
        connect_state = (int)RVR::RVRControllerConnectionState::kConnected;
    }
    else 
    {
        connect_state = (int)RVR::RVRControllerConnectionState::kDisconnected;
    }
    data->connectionState = (RVR::RVRControllerConnectionState)connect_state;

    ////Position
    data->position.x = pose->vecPosition[0];
    data->position.y = pose->vecPosition[1];
    data->position.z = pose->vecPosition[2];

    //Orientation
    data->rotation.w = pose->qRotation.w;
    data->rotation.x = pose->qRotation.x;
    data->rotation.y = pose->qRotation.y;
    data->rotation.z = pose->qRotation.z;
    /*float r = sqrt(data->rotation.w*data->rotation.w + data->rotation.x*data->rotation.x +
        pose->qRotation.y* pose->qRotation.y + pose->qRotation.z*pose->qRotation.z);
    if (r < 0.0001)
        r = 0.0001;
*/
    data->vecVelocity.x = pose->vecVelocity[0];
    data->vecVelocity.y = pose->vecVelocity[1];
    data->vecVelocity.z = pose->vecVelocity[2];

    data->vecAcceleration.x = pose->vecAcceleration[0];
    data->vecAcceleration.y = pose->vecAcceleration[1];
    data->vecAcceleration.z = pose->vecAcceleration[2];

    data->vecAngularVelocity.x = pose->vecAngularVelocity[0];
    data->vecAngularVelocity.y = pose->vecAngularVelocity[1];
    data->vecAngularVelocity.z = pose->vecAngularVelocity[2];

	data->vecAngularAcceleration.x = 0;
	data->vecAngularAcceleration.y = 0;
	data->vecAngularAcceleration.z =0;
}
//-----------------------------------------------------------------------------
void ExtractRVRPoseHmdDataSimple(RVR::RVRPoseHmdData* poseData, const vr::HmdMatrix34_t *pPose)
//-----------------------------------------------------------------------------
{
   /* uint64_t timestamp = nowInUs();
    poseData->valid = true;
    ExtractRotation(&(poseData->rotation), pPose);
    ExtractPosition(&(poseData->position), pPose);
    poseData->poseTimeStamp = nowInUs();
    poseData->poseRecvTime = nowInUs();
    poseData->predictedTimeMs = 0.0f;*/
}
void RotateTranslateVector3(RVR::RVRQuaternion rotation, RVR::RVRVector3 vec3_in, RVR::RVRVector3& vec3_out)
{
    glm::quat _q_in(rotation.w, rotation.x, rotation.y, rotation.z);
    glm::vec3 _v_in(vec3_in.x, vec3_in.y,vec3_in.z);
    glm::vec3 _v_out = _q_in * _v_in;
    vec3_out.x = _v_out.x;
    vec3_out.y = _v_out.y;
    vec3_out.z = _v_out.z;
}

void RotateTranslateVector3ByEigen(RVR::RVRQuaternion rotation, RVR::RVRVector3 vec3_in, RVR::RVRVector3& vec3_out)
{
    Eigen::Quaterniond _q_in = Eigen::Quaterniond(rotation.w, rotation.x, rotation.y, rotation.z);
    Eigen::Vector3d _v_in = Eigen::Vector3d(vec3_in.x, vec3_in.y, vec3_in.z);
    Eigen::Vector3d  _v_out = _q_in * _v_in;
	vec3_out.x = _v_out[0];
	vec3_out.y = _v_out[1];
	vec3_out.z = _v_out[2];
}

void   ChangeRotation(RVR::RVRQuaternion& rotation, RVR::RVRQuaternion increment)
{
	RVR::RVRQuaternion out_rotation = { 0 };
	out_rotation.x = rotation.w * increment.x + rotation.x * increment.w + rotation.y * increment.z - rotation.z * increment.y;
	out_rotation.y = rotation.w * increment.y + rotation.y * increment.w + rotation.z * increment.x - rotation.x * increment.z;
	out_rotation.z = rotation.w * increment.z + rotation.z * increment.w + rotation.x * increment.y - rotation.y * increment.x;
	out_rotation.w = rotation.w * increment.w - rotation.x * increment.x - rotation.y * increment.y - rotation.z * increment.z;
	rotation = out_rotation;
}

void ChangePosition(RVR::RVRQuaternion origin_rotation, RVR::RVRVector3& origin_position, RVR::RVRVector3 increment_position)
{
	RVR::RVRVector3 translate_vec3_out = { 0 };
    RotateTranslateVector3ByEigen(origin_rotation, increment_position, translate_vec3_out);
	origin_position.x = origin_position.x + translate_vec3_out.x;
	origin_position.y = origin_position.y + translate_vec3_out.y;
	origin_position.z = origin_position.z + translate_vec3_out.z;
}
void EularAnglesToRotation(double pitch, double yaw, double roll, RVR::RVRQuaternion& rotation)// pitch yaw roll绕 x, y ,z 
{
	Eigen::AngleAxisd rollAngle(roll / 180.0 * PI, Eigen::Vector3d::UnitZ());
	Eigen::AngleAxisd yawAngle(yaw / 180.0 * PI, Eigen::Vector3d::UnitY());
	Eigen::AngleAxisd pitchAngle(pitch / 180.0 * PI, Eigen::Vector3d::UnitX());
	Eigen::Quaterniond quaternion;
	quaternion = rollAngle * yawAngle * pitchAngle;

    rotation.w = quaternion.w();
    rotation.x = quaternion.x();
    rotation.y = quaternion.y();
    rotation.z = quaternion.z();
}

void RotationToEularAngles(RVR::RVRQuaternion rotation, double& pitch, double& yaw, double& roll)// pitch yaw roll绕 x, y ,z
{
    Eigen::Quaterniond quaternion = {rotation.w,rotation.x,rotation.y,rotation.z};

    Eigen::Vector3d eulerAngle = quaternion.matrix().eulerAngles(2, 1, 0);
    //rz, ry, rx
    roll = eulerAngle[0];
    yaw = eulerAngle[1];
    pitch = eulerAngle[2];
} 

void RotationToEularAngles2(float rotation[4], double& pitch, double& yaw, double& roll)// pitch yaw roll绕 x, y ,z
{
	Eigen::Quaterniond quaternion = {rotation[0],rotation[1],rotation[2],rotation[3] };

	Eigen::Vector3d eulerAngle = quaternion.matrix().eulerAngles(2, 1, 0);
    eulerAngle = eulerAngle.transpose();
	//rz, ry, rx
	roll = eulerAngle[0];
	yaw = eulerAngle[1];
	pitch = eulerAngle[2];
	pitch = std::fmod(pitch, 3.1415926f);
	yaw = std::fmod(yaw, 3.1415926f);
	roll = std::fmod(roll, 3.1415926f);
    if (pitch<0.f)
    {
        pitch = 3.1415926f - pitch;
    }

	if (yaw < 0.f)
	{
        yaw = 3.1415926f - yaw;
	}

	if (roll < 0.f)
	{
        roll = 3.1415926f - roll;
	}


    roll = roll * 180.f / 3.1415926f;
    yaw = yaw * 180.f / 3.1415926f;
    pitch = pitch * 180.f / 3.1415926f;


    pitch = std::fmod(pitch,360);
    yaw = std::fmod(yaw, 360);
    roll = std::fmod(roll, 360);
} 
///-------------------------------------------------
// STEP 1 
// 输入四元数1和2，输出从从1到2和 从2到1的四元数
// 
// 
// 
//  @param [in]  float[4] (w,x,y,z)      为游戏渲染时实际使用的姿态
//  @param [in]  float[4]                为渲染完成后最新时刻获取的姿态
//  @param [out]  float[4]               从1转到2
//  @param [out]  float[4]               从2转到1
//-------------------------------------------------

void  GetRotationFromQuat1ToQuat2(float* quat1, float* quat2, float* rotation, float* rotation_inv)
{
	glm::quat _q1(quat1[0], quat1[1], quat1[2], quat1[3]);
	glm::quat _q2(quat2[0], quat2[1], quat2[2], quat2[3]);

	glm::quat _q1_inv = glm::inverse(_q1);
	glm::quat _q2_inv = glm::inverse(_q2);


	glm::quat _q1_t_q2 = _q1 * _q2_inv;

	rotation[0] = _q1_t_q2.w;
	rotation[1] = _q1_t_q2.x;
	rotation[2] = _q1_t_q2.y;
	rotation[3] = -1.0f * _q1_t_q2.z;

	rotation_inv[0] = rotation[0];
	rotation_inv[1] = -rotation[1];
	rotation_inv[2] = -rotation[2];
	rotation_inv[3] = -rotation[3];

}


//-------------------------------------------------
// STEP 2 
// 输入四元数，输出旋转矩阵
//  @param [in]  float[4] (w,x,y,z)
//  @param [out]  float[16] 
//-------------------------------------------------

void ConvertQuatToMatrix(float input[4], float* mat)
{
	float w = input[0];
	float x = input[1];
	float y = input[2];
	float z = input[3];



	//O
	mat[0] = 1.0f - (2.0f * y * y) - (2.0f * z * z);
	mat[1] = 2.0f * x * y - 2.0f * w * z;
	mat[2] = 2.0f * x * z + 2.0f * w * y;
	mat[3] = 0.0f;

	mat[4] = 2.0f * x * y + 2.0f * w * z;
	mat[5] = 1.0f - (2.0f * x * x) - (2.0f * z * z);
	mat[6] = 2.0f * y * z - 2.0f * w * x;
	mat[7] = 0.0f;

	mat[8] = 2.0f * x * z - 2.0f * w * y;
	mat[9] = 2.0f * y * z + 2.0f * w * x;
	mat[10] = 1.0f - (2.0f * x * x) - (2.0f * y * y);
	mat[11] = 0.0f;

	mat[12] = 0.0f;
	mat[13] = 0.0f;
	mat[14] = 0.0f;
	mat[15] = 1.0f;

}




