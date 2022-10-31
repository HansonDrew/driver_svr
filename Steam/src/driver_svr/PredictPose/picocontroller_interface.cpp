#include "picocontroller_interface.h"
#include "PVR_Math.h"
#include <iostream>


int PredictMotion(algo_result_t input_algo_result, double predictTime ,algo_result_t &new_algo_result)
{   
	algo_result_t src = input_algo_result;
	double predictTime_in_s=predictTime*1e-9;
	double pose_time_in_s=src.pose.timestamp*1e-9;
	double delay=predictTime_in_s-pose_time_in_s;
	double angular_delay = delay;

 	PVR::Posef pose(PVR::Quatf(src.pose.rx, src.pose.ry, src.pose.rz, src.pose.rw),
					PVR::Vector3f(src.pose.x, src.pose.y, src.pose.z));

	PVR::Vector3f angularVelocity = PVR::Vector3f(src.wx, src.wy, src.wz);


	PVR::Vector3f lineVelocity = PVR::Vector3f(src.vx, src.vy, src.vz);
	PVR::Vector3f acceleration = PVR::Vector3f(src.ax, src.ay, src.az);
	float acceleration_XYZ = sqrt(pow(src.ax, 2) + pow(src.ay, 2) + pow(src.az, 2));
	float angularSpeed = angularVelocity.Length();



	// This could be tuned so that linear and angular are combined with different coefficients
	float speed = angularSpeed;

	const float slope = 0.2;		   // The rate at which the dynamic prediction interval varies
	float candidateDt = slope * speed; // TODO: Replace with smoothstep function
	

	float dynamicDt = delay;

	// Choose the candidate if it is shorter, to improve stability
 	if (candidateDt < dynamicDt)
	{
		dynamicDt = candidateDt;
	}
		 

	const float MAX_DELTA_TIME = 0.1;//1.0f / 25.0f;
	dynamicDt = dynamicDt<MAX_DELTA_TIME? dynamicDt:MAX_DELTA_TIME;



	if (angularSpeed > 0.001)  //angularSpeed > 0.001
	{
       pose.Orientation = pose.Orientation * PVR::Quatf(angularVelocity, angularSpeed * dynamicDt);
	   pose.Position=pose.Position+lineVelocity*dynamicDt;
	}
		
	new_algo_result.pose.x = pose.Position.x;
	new_algo_result.pose.y = pose.Position.y;
	new_algo_result.pose.z = pose.Position.z;
	new_algo_result.pose.rx = pose.Orientation.x;
	new_algo_result.pose.ry = pose.Orientation.y;
	new_algo_result.pose.rz = pose.Orientation.z;
	new_algo_result.pose.rw = pose.Orientation.w;
	new_algo_result.vx = input_algo_result.vx;
	new_algo_result.vy = input_algo_result.vy;
	new_algo_result.vz = input_algo_result.vz;
	new_algo_result.ax = input_algo_result.ax;
	new_algo_result.ay = input_algo_result.ay;
	new_algo_result.az = input_algo_result.az;

	new_algo_result.wx = input_algo_result.wx;
	new_algo_result.wx = input_algo_result.wy;
	new_algo_result.wx = input_algo_result.wz;

	new_algo_result.w_ax = input_algo_result.w_ax;
	new_algo_result.w_ay = input_algo_result.w_ay;
	new_algo_result.w_az = input_algo_result.w_az;
	new_algo_result.pose.timestamp=src.pose.timestamp+ (dynamicDt * 1e9);
 
	return 0;

}

void ConvertPoseToPredict(vr::DriverPose_t input_sensor, int64_t timestamp, algo_result_t& output_sensor) 
{
	output_sensor.pose.rw = input_sensor.qRotation.w;
	output_sensor.pose.rx = input_sensor.qRotation.x;
	output_sensor.pose.ry = input_sensor.qRotation.y;
	output_sensor.pose.rz = input_sensor.qRotation.z;

	output_sensor.pose.timestamp = timestamp;
	output_sensor.pose.x = input_sensor.vecPosition[0];
	output_sensor.pose.y = input_sensor.vecPosition[1];
	output_sensor.pose.z = input_sensor.vecPosition[2];

	output_sensor.vx = input_sensor.vecVelocity[0];
	output_sensor.vy = input_sensor.vecVelocity[1];
	output_sensor.vz = input_sensor.vecVelocity[2];

	output_sensor.ax = input_sensor.vecAcceleration[0];
	output_sensor.ay = input_sensor.vecAcceleration[1];
	output_sensor.az = input_sensor.vecAcceleration[2];

	output_sensor.wx = input_sensor.vecAngularVelocity[0];
	output_sensor.wy = input_sensor.vecAngularVelocity[1];
	output_sensor.wz = input_sensor.vecAngularVelocity[2];

	output_sensor.w_ax = output_sensor.w_ay = output_sensor.w_az = 0;
}

void ConvertPoseToDriver(algo_result_t input_sensor, vr::DriverPose_t& output_sensor) 
{
	output_sensor.qRotation.w = input_sensor.pose.rw;
	output_sensor.qRotation.x = input_sensor.pose.rx;
	output_sensor.qRotation.y = input_sensor.pose.ry;
	output_sensor.qRotation.z = input_sensor.pose.rz;

	output_sensor.vecPosition[0] = input_sensor.pose.x;
	output_sensor.vecPosition[1] = input_sensor.pose.y;
	output_sensor.vecPosition[2] = input_sensor.pose.z;

	output_sensor.vecVelocity[0] = input_sensor.vx;
	output_sensor.vecVelocity[1] = input_sensor.vy;
	output_sensor.vecVelocity[2] = input_sensor.vz;

	output_sensor.vecAcceleration[0] = input_sensor.ax;
	output_sensor.vecAcceleration[1] = input_sensor.ay;
	output_sensor.vecAcceleration[2] = input_sensor.az;

	output_sensor.vecAngularVelocity[0] = input_sensor.wx;
	output_sensor.vecAngularVelocity[1] = input_sensor.wy;
	output_sensor.vecAngularVelocity[2] = input_sensor.wz;

	output_sensor.vecAngularAcceleration[0] = input_sensor.w_ax;
	output_sensor.vecAngularAcceleration[1] = input_sensor.w_ay;
	output_sensor.vecAngularAcceleration[2] = input_sensor.w_az;
}

void ConvertRvrControllerPoseToPredict(RVR::RVRControllerData input_sensor, int64_t timestamp, algo_result_t& output_sensor) 
{
	output_sensor.pose.rw = input_sensor.rotation.w;
	output_sensor.pose.rx = input_sensor.rotation.x;
	output_sensor.pose.ry = input_sensor.rotation.y;
	output_sensor.pose.rz = input_sensor.rotation.z;

	output_sensor.pose.timestamp = timestamp;
	output_sensor.pose.x = input_sensor.position.x;
	output_sensor.pose.y = input_sensor.position.y;
	output_sensor.pose.z = input_sensor.position.z;

	output_sensor.vx = input_sensor.vecVelocity.x;
	output_sensor.vy = input_sensor.vecVelocity.y;
	output_sensor.vz = input_sensor.vecVelocity.z;

	output_sensor.ax = input_sensor.vecAcceleration.x;
	output_sensor.ay = input_sensor.vecAcceleration.y;
	output_sensor.az = input_sensor.vecAcceleration.z;

	output_sensor.wx = input_sensor.vecAngularVelocity.x;
	output_sensor.wy = input_sensor.vecAngularVelocity.y;
	output_sensor.wz = input_sensor.vecAngularVelocity.z;

	output_sensor.w_ax = output_sensor.w_ay = output_sensor.w_az = 0;
}
void ConvertPoseToRvrControllerPose(algo_result_t input_sensor, RVR::RVRControllerData& output_sensor) 
{
	output_sensor.rotation.w = input_sensor.pose.rw;
	output_sensor.rotation.x = input_sensor.pose.rx;
	output_sensor.rotation.y = input_sensor.pose.ry;
	output_sensor.rotation.z = input_sensor.pose.rz;

	output_sensor.position.x = input_sensor.pose.x;
	output_sensor.position.y = input_sensor.pose.y;
	output_sensor.position.z = input_sensor.pose.z;
	output_sensor.vecVelocity.x = input_sensor.vx;
	output_sensor.vecVelocity.y = input_sensor.vy;
	output_sensor.vecVelocity.z = input_sensor.vz;

	output_sensor.vecAcceleration.x = input_sensor.ax;
	output_sensor.vecAcceleration.y = input_sensor.ay;
	output_sensor.vecAcceleration.z = input_sensor.az;

	output_sensor.vecAngularVelocity.x= input_sensor.wx;
	output_sensor.vecAngularVelocity.y = input_sensor.wy;
	output_sensor.vecAngularVelocity.z = input_sensor.wz;

	output_sensor.vecAngularAcceleration.x = input_sensor.w_ax;
	output_sensor.vecAngularAcceleration.y = input_sensor.w_ay;
	output_sensor.vecAngularAcceleration.z = input_sensor.w_az;
	output_sensor.timestamp = input_sensor.pose.timestamp;
	{

	};
}



void ConvertRvrHmdPoseToPredict(RVR::RVRPoseHmdData input_sensor, int64_t timestamp, algo_result_t& output_sensor)
{
	output_sensor.pose.rw = input_sensor.rotation.w;
	output_sensor.pose.rx = input_sensor.rotation.x;
	output_sensor.pose.ry = input_sensor.rotation.y;
	output_sensor.pose.rz = input_sensor.rotation.z;

	output_sensor.pose.timestamp = timestamp;
	output_sensor.pose.x = input_sensor.position.x;
	output_sensor.pose.y = input_sensor.position.y;
	output_sensor.pose.z = input_sensor.position.z;

	output_sensor.vx = input_sensor.linearVelocity.x;
	output_sensor.vy = input_sensor.linearVelocity.y;
	output_sensor.vz = input_sensor.linearVelocity.z;

	output_sensor.ax = input_sensor.linearAcceleration.x;
	output_sensor.ay = input_sensor.linearAcceleration.y;
	output_sensor.az = input_sensor.linearAcceleration.z;

	output_sensor.wx = input_sensor.angularVelocity.x;
	output_sensor.wy = input_sensor.angularVelocity.y;
	output_sensor.wz = input_sensor.angularVelocity.z;

	output_sensor.w_ax = output_sensor.w_ay = output_sensor.w_az = 0;
}
void ConvertPoseToRvrHmdPose(algo_result_t input_sensor, RVR::RVRPoseHmdData& output_sensor)
{
	output_sensor.rotation.w = input_sensor.pose.rw;
	output_sensor.rotation.x = input_sensor.pose.rx;
	output_sensor.rotation.y = input_sensor.pose.ry;
	output_sensor.rotation.z = input_sensor.pose.rz;

	output_sensor.position.x = input_sensor.pose.x;
	output_sensor.position.y = input_sensor.pose.y;
	output_sensor.position.z = input_sensor.pose.z;
	output_sensor.linearVelocity.x = input_sensor.vx;
	output_sensor.linearVelocity.y = input_sensor.vy;
	output_sensor.linearVelocity.z = input_sensor.vz;

	output_sensor.linearAcceleration.x = input_sensor.ax;
	output_sensor.linearAcceleration.y = input_sensor.ay;
	output_sensor.linearAcceleration.z = input_sensor.az;

	output_sensor.angularVelocity.x = input_sensor.wx;
	output_sensor.angularVelocity.y = input_sensor.wy;
	output_sensor.angularVelocity.z = input_sensor.wz;

	output_sensor.angularAcceleration.x = input_sensor.w_ax;
	output_sensor.angularAcceleration.y = input_sensor.w_ay;
	output_sensor.angularAcceleration.z = input_sensor.w_az;
	output_sensor.poseTimeStamp = input_sensor.pose.timestamp;
}

int PredictMotionRvrHmd(RVR::RVRPoseHmdData input_sensor, double to_predict_ts, RVR::RVRPoseHmdData& result_sensor)
{
	algo_result_t input_data = { 0 };
	algo_result_t out_data = { 0 };

	ConvertRvrHmdPoseToPredict(input_sensor, input_sensor.poseTimeStamp, input_data);
	PredictMotion(input_data, to_predict_ts, out_data);
	ConvertPoseToRvrHmdPose(out_data, result_sensor);
	result_sensor.valid = input_sensor.valid;
	 
	return 1;
}

int PredictMotionRvrController(RVR::RVRControllerData input_sensor, double to_predict_ts, RVR::RVRControllerData& result_sensor)
{
	algo_result_t input_data = { 0 };
	algo_result_t out_data = { 0 };

	ConvertRvrControllerPoseToPredict(input_sensor, input_sensor.timestamp, input_data);
	PredictMotion(input_data, to_predict_ts, out_data);
	
	ConvertPoseToRvrControllerPose(out_data, result_sensor);
	result_sensor.connectionState = input_sensor.connectionState;

	return 1;
}