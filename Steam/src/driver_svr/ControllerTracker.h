#pragma once
#include "../../Eigen/Geometry"
#include "../../Eigen/Core"
#include "../RVRPlugin/RVRPluginDefinitions.h"
const double EPS = 0.0000000001;
const double PI3_14 = 3.14159265;
const double PiOver2 = 1.5707963265;
const double ANGLE_EPSILON = 1.745329252e-7f;  // 1.0e-5*pi/180

template<typename T>
T safe_acos(T val)
{
	if (val >= T(1))			return T(0);
	else if (val <= T(-1))		return T(PI3_14);
	else						return std::acos(val);
};

template<typename T>
T safe_asin(T val)
{
	if (val >= T(1))				return T(PiOver2);
	else if (val <= T(-1))		return T(PiOver2) * T(3);
	else						return std::asin(val);
};
struct PoseInfo
{
	double timestamp;
	Eigen::Vector3d speed = Eigen::Vector3d::Zero();
	Eigen::Vector3d ba = Eigen::Vector3d::Zero();//不用
	Eigen::Vector3d bg = Eigen::Vector3d::Zero();//不用
	Eigen::Vector3d p = Eigen::Vector3d::Zero();;
	Eigen::Quaterniond q = Eigen::Quaterniond::Identity();

	Eigen::Vector3d curr_acc;
	Eigen::Vector3d curr_gyro;
	unsigned char confidence;
	double plane_height;
	unsigned char plane_status;
	unsigned char relocation_status;
	bool track_status;
};
class ControllerTracker
{
public:
	void getRodrigues(const Eigen::Quaterniond& _Q, Eigen::Vector3d& _W, const double _Eps);
	void setRodrigues(const Eigen::Vector3d& _W, Eigen::Quaterniond& _Q, const double _Eps);
	void kalmanFilterInit(PoseInfo& _BasePose, Eigen::Vector3d& _Vec3DeltaqInit, double& _dDeltaT, double& _dDeltaQ);
	void applyKalmanFilter(PoseInfo& _BasePose, Eigen::Vector3d& _Vec3Deltaq);
	Eigen::Matrix<float, 9, 1> m_fStatePost;
	Eigen::Matrix<float, 9, 9> m_fMeasurementMatrix;
	Eigen::Matrix<float, 9, 9> m_fErrorCovPost;
	Eigen::Matrix<float, 9, 9> m_fTransitionMatrix;
	Eigen::Matrix<float, 9, 9> m_fProcessNoiseCov;
	Eigen::Matrix<float, 9, 9> m_fMeasurementNoiseCov;
	Eigen::Matrix<float, 9, 1> m_fMeasurement;
	Eigen::Quaterniond LastOrientaion_6dof;
	bool m_bKFInitialized = false;
	double m_dPoseError = 0.1;
	double m_dVelError = 0.1;
	double m_dDeltaqError = 0.5;
	double m_dMeasurePoseError = 1;
	double m_dMeasureVelError = 0;
	double m_dMeasureDeltaqError = 1;
	void KalmanFilter(PoseInfo& hi_frq_op);
	void DoKalma(RVR::RVRControllerData &data);
	void ConverRvrPoseToPoseInfo(RVR::RVRControllerData rvr_pose_in,PoseInfo &p_info_out);
	void ConverPoseInfoToRvrPose(PoseInfo p_info_in, RVR::RVRControllerData &rvr_pose_out) ;

	void DoKalma(RVR::RVRPoseHmdData& data);
	void ConverRvrPoseToPoseInfo(RVR::RVRPoseHmdData rvr_pose_in, PoseInfo& p_info_out);
	void ConverPoseInfoToRvrPose(PoseInfo p_info_in, RVR::RVRPoseHmdData& rvr_pose_out);

	float m_fMeasureErrorHighLimit = 20;
	bool m_bUseBle = false;
};

