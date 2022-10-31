#include "ControllerTracker.h"
#include "config_reader.h"
#include"stringtool.h"
extern ConfigReader gConfigReader;
 
void ControllerTracker::getRodrigues(const Eigen::Quaterniond& _Q, Eigen::Vector3d& _W, const double _Eps)
{
	const double thh = safe_acos(_Q.w()), th = thh + thh;
	if (th < _Eps || _Q.w() > 1.0) {
		_W = Eigen::Vector3d::Zero();
	}
	else {
		double m = th / (std::sin(thh) + _Eps);
		_W << _Q.x() * m, _Q.y()* m, _Q.z()* m;
	}
}

void ControllerTracker::setRodrigues(const Eigen::Vector3d& _W, Eigen::Quaterniond& _Q, const double _Eps)
{
	const double th2 = _W.squaredNorm(), th = std::sqrt(th2);
	if (th < _Eps) {
		const double s = 1.0f / std::sqrt(th2 + 4.0f);
		_Q = Eigen::Quaterniond(s + s, s * _W(0), s * _W(1), s * _W(2));
	}
	else {
		const double thh = th * 0.5f;
		const double s = std::sin(thh) / (th + _Eps);
		_Q = Eigen::Quaterniond(std::cos(thh), s * _W(0), s * _W(1), s * _W(2));
	}
	_Q.normalize();
}




void ControllerTracker::kalmanFilterInit(PoseInfo& _BasePose, Eigen::Vector3d& _Vec3DeltaqInit,
	double& _dDeltaT, double& _dDeltaQ)
{
	LastOrientaion_6dof = _BasePose.q;
	m_fStatePost << _BasePose.p.x(), _BasePose.speed.x(),
		_BasePose.p.y(), _BasePose.speed.y(),
		_BasePose.p.z(), _BasePose.speed.z(),
		_Vec3DeltaqInit.x(), _Vec3DeltaqInit.y(),
		_Vec3DeltaqInit.z();

	m_fMeasurementMatrix.setIdentity();
	m_fErrorCovPost.setZero();
	m_fMeasurementNoiseCov.setZero();
	m_fTransitionMatrix.setIdentity();
	m_fTransitionMatrix(0, 1) = _dDeltaT;
	m_fTransitionMatrix(2, 3) = _dDeltaT;
	m_fTransitionMatrix(4, 5) = _dDeltaT;
	m_fTransitionMatrix(6, 6) = _dDeltaQ;
	m_fTransitionMatrix(7, 7) = _dDeltaQ;
	m_fTransitionMatrix(8, 8) = _dDeltaQ;

	m_fProcessNoiseCov.setZero();
	m_fProcessNoiseCov(0, 0) = m_dPoseError;
	m_fProcessNoiseCov(1, 1) = m_dVelError;
	m_fProcessNoiseCov(2, 2) = m_dPoseError;
	m_fProcessNoiseCov(3, 3) = m_dVelError;
	m_fProcessNoiseCov(4, 4) = m_dPoseError;
	m_fProcessNoiseCov(5, 5) = m_dVelError;
	m_fProcessNoiseCov(6, 6) = m_dDeltaqError;
	m_fProcessNoiseCov(7, 7) = m_dDeltaqError;
	m_fProcessNoiseCov(8, 8) = m_dDeltaqError;

	m_bKFInitialized = true;
}

void ControllerTracker::applyKalmanFilter(PoseInfo& _BasePose, Eigen::Vector3d& _Vec3Deltaq)
{
	m_fMeasurementNoiseCov(0, 0) = m_dMeasurePoseError;
	// m_fMeasurementNoiseCov(1,1) = m_dMeasureVelError;
	m_fMeasurementNoiseCov(2, 2) = m_dMeasurePoseError;
	// m_fMeasurementNoiseCov(3,3) = m_dMeasureVelError;
	m_fMeasurementNoiseCov(4, 4) = m_dMeasurePoseError;
	// m_fMeasurementNoiseCov(5,5) = m_dMeasureVelError;
	m_fMeasurementNoiseCov(6, 6) = m_dMeasureDeltaqError;
	m_fMeasurementNoiseCov(7, 7) = m_dMeasureDeltaqError;
	m_fMeasurementNoiseCov(8, 8) = m_dMeasureDeltaqError;

	m_fMeasurement << _BasePose.p.x(), _BasePose.speed.x(), _BasePose.p.y(),
		_BasePose.speed.y(), _BasePose.p.z(), _BasePose.speed.z(),
		_Vec3Deltaq.x(), _Vec3Deltaq.y(), _Vec3Deltaq.z();

	Eigen::MatrixXf fStatePre = m_fTransitionMatrix * m_fStatePost;
	Eigen::MatrixXf fErrorCovPre = m_fTransitionMatrix * m_fErrorCovPost * m_fTransitionMatrix.transpose() + m_fProcessNoiseCov;
	Eigen::MatrixXf fHpk = m_fMeasurementMatrix * fErrorCovPre;
	Eigen::MatrixXf fHtR = fHpk * m_fMeasurementMatrix.transpose() + m_fMeasurementNoiseCov;
	Eigen::MatrixXf fGaintran = fHtR.inverse() * fHpk;
	Eigen::MatrixXf fZkHxk = m_fMeasurement - m_fMeasurementMatrix * fStatePre;
	Eigen::MatrixXf fGain = fGaintran.transpose();
	m_fStatePost = fStatePre + fGain * fZkHxk;
	m_fErrorCovPost = fErrorCovPre - fGain * fHpk;


}

void ControllerTracker::KalmanFilter(PoseInfo& hi_frq_op)
{
	PoseInfo KF_pose = hi_frq_op;
	//计算两帧姿态四元数之差：
	Eigen::Quaterniond DeltaQ_6dof = hi_frq_op.q * LastOrientaion_6dof.inverse();
	if (DeltaQ_6dof.w() < 0)
	{
		DeltaQ_6dof.w() = -DeltaQ_6dof.w();
		DeltaQ_6dof.x() = -DeltaQ_6dof.x();
		DeltaQ_6dof.y() = -DeltaQ_6dof.y();
		DeltaQ_6dof.z() = -DeltaQ_6dof.z();
	}
	Eigen::Vector3d Vec3Deltaq;
	getRodrigues(DeltaQ_6dof, Vec3Deltaq, ANGLE_EPSILON);
	// kalman 初始化检查
	if (!m_bKFInitialized)
	{
		double dDeltaT = 1.0f/gConfigReader.GetFps();// 0.002;// _Imu.timestamp - m_LastImu.timestamp;
		if (m_bUseBle)
		{
			dDeltaT = 0.005;
		}
		double dDeltaQ = 1.0;
		kalmanFilterInit(KF_pose, Vec3Deltaq, dDeltaT, dDeltaQ);
	}
	if (m_bKFInitialized)
	{
		m_dMeasurePoseError =0.5;
		m_dMeasureVelError = 0.5;
		m_dMeasureDeltaqError = 0.5;
		double DeltaT = 1.0f / gConfigReader.GetFps();;

		applyKalmanFilter(KF_pose, Vec3Deltaq);

		Eigen::Vector3d  Vec3Deltaq_result;
		Vec3Deltaq_result.x() = m_fStatePost(6, 0);
		Vec3Deltaq_result.y() = m_fStatePost(7, 0);
		Vec3Deltaq_result.z() = m_fStatePost(8, 0);
		// 得到滤波后的 delta 四元数
		Eigen::Quaterniond q_add;
		setRodrigues(Vec3Deltaq_result, q_add, ANGLE_EPSILON);
		q_add.normalize();
		// 通过更新后的delta四元数更新需要输出的四元数				
		hi_frq_op.q = q_add * LastOrientaion_6dof;
	}
	LastOrientaion_6dof = hi_frq_op.q;
}

void  ControllerTracker::DoKalma(RVR::RVRControllerData& data) 
{
	PoseInfo cal_data = {0};
	ConverRvrPoseToPoseInfo(data, cal_data);
	KalmanFilter(cal_data);
	ConverPoseInfoToRvrPose(cal_data,data);
}
void  ControllerTracker::ConverRvrPoseToPoseInfo(RVR::RVRControllerData rvr_pose_in, PoseInfo& p_info_out) 
{
	p_info_out.p=Eigen::Vector3d( rvr_pose_in.position.x, rvr_pose_in.position.y, rvr_pose_in.position.z);
	p_info_out.q = Eigen::Quaternion<double>(rvr_pose_in.rotation.w, rvr_pose_in.rotation.x, rvr_pose_in.rotation.y, rvr_pose_in.rotation.z);
	p_info_out.speed= Eigen::Vector3d(rvr_pose_in.vecVelocity.x, rvr_pose_in.vecVelocity.y, rvr_pose_in.vecVelocity.z);
}
void  ControllerTracker::ConverPoseInfoToRvrPose(PoseInfo p_info_in, RVR::RVRControllerData &rvr_pose_out) 
{
	rvr_pose_out.position.x = p_info_in.p[0];
	rvr_pose_out.position.y = p_info_in.p[1];
	rvr_pose_out.position.z = p_info_in.p[2];

	rvr_pose_out.vecVelocity.x = p_info_in.speed[0];
	rvr_pose_out.vecVelocity.y = p_info_in.speed[1];
	rvr_pose_out.vecVelocity.z = p_info_in.speed[2];

	rvr_pose_out.rotation.w = p_info_in.q.w();
	rvr_pose_out.rotation.x = p_info_in.q.x();
	rvr_pose_out.rotation.y = p_info_in.q.y();
	rvr_pose_out.rotation.z = p_info_in.q.z();

}



void  ControllerTracker::DoKalma(RVR::RVRPoseHmdData& data)
{
	PoseInfo cal_data = { 0 };
	ConverRvrPoseToPoseInfo(data, cal_data);
	KalmanFilter(cal_data);
	ConverPoseInfoToRvrPose(cal_data, data);
}
void  ControllerTracker::ConverRvrPoseToPoseInfo(RVR::RVRPoseHmdData rvr_pose_in, PoseInfo& p_info_out)
{
	p_info_out.p = Eigen::Vector3d(rvr_pose_in.position.x, rvr_pose_in.position.y, rvr_pose_in.position.z);
	p_info_out.q = Eigen::Quaternion<double>(rvr_pose_in.rotation.w, rvr_pose_in.rotation.x, rvr_pose_in.rotation.y, rvr_pose_in.rotation.z);
	p_info_out.speed = Eigen::Vector3d(rvr_pose_in.linearVelocity.x, rvr_pose_in.linearVelocity.y, rvr_pose_in.linearVelocity.z);
}
void  ControllerTracker::ConverPoseInfoToRvrPose(PoseInfo p_info_in, RVR::RVRPoseHmdData& rvr_pose_out)
{
	rvr_pose_out.position.x = p_info_in.p[0];
	rvr_pose_out.position.y = p_info_in.p[1];
	rvr_pose_out.position.z = p_info_in.p[2];

	rvr_pose_out.linearVelocity.x = p_info_in.speed[0];
	rvr_pose_out.linearVelocity.y = p_info_in.speed[1];
	rvr_pose_out.linearVelocity.z = p_info_in.speed[2];

	rvr_pose_out.rotation.w = p_info_in.q.w();
	rvr_pose_out.rotation.x = p_info_in.q.x();
	rvr_pose_out.rotation.y = p_info_in.q.y();
	rvr_pose_out.rotation.z = p_info_in.q.z();

}