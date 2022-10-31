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
