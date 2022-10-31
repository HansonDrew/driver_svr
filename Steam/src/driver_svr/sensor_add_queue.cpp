#include "sensor_add_queue.h"
#include "driver_define.h"
#include "PredictPose/picocontroller_interface.h"
#include "Util.h"
#include "config_reader.h"
#include "smooth_sensor.h"
#include "RVRLogger.h"
#include "RVRUtils.h"
#include <vector>
using namespace RVR;
extern ConfigReader gConfigReader;

extern bool gDashboardActivated ;
extern bool gOverlayShow ;
void sensor_add_queue::SetPose(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose) 
{
	new_receive_ts_ = RVR::nowInNs();
	if (hmd_pose.poseTimeStamp==0)
	{
		return;
	}
	if (hmd_pose.valid==false)
	{
		return;
	}
	if ((hmd_pose.poseTimeStamp-last_hmd_pose_.poseTimeStamp)<3000000)
	{
		return;
	}
	pose_lock_.lock();
	new_receive_sdk_ts_ = hmd_pose.poseTimeStamp;
	while (gConfigReader.GetMaxSensorStoreValue()<=hmd_pose_queue_.size())
	{

		
		hmd_pose_queue_.pop();
		controller_pose_queue_[(int)ControllerIndex::kLeftController].pop();
		controller_pose_queue_[(int)ControllerIndex::kRightController].pop();
		
		
	}
	hmd_pose_queue_.push(hmd_pose);
	controller_pose_queue_[(int)ControllerIndex::kLeftController].push(left_controller_pose);
	controller_pose_queue_[(int)ControllerIndex::kRightController].push(right_controller_pose);

	pose_lock_.unlock();
}
void sensor_add_queue::SmoothSensor() 
{
	int sensor_size = hmd_pose_queue_.size();
	double sensor_in_x[10] = { 0 };
	double sensor_out_x[10] = { 0 };
	double sensor_in_y[10] = { 0 };
	double sensor_out_y[10] = { 0 };
	double sensor_in_z[10] = { 0 };
	double sensor_out_z[10] = { 0 };


	double sensor_in_rw[10] = { 0 };
	double sensor_out_rw[10] = { 0 };
	double sensor_in_rx[10] = { 0 };
	double sensor_out_rx[10] = { 0 };
	double sensor_in_ry[10] = { 0 };
	double sensor_out_ry[10] = { 0 };
	double sensor_in_rz[10] = { 0 };
	double sensor_out_rz[10] = { 0 };

	RVR::RVRPoseHmdData hmd[10] = { 0 };
	for (int i= sensor_size-1;i>=0;i--)
	{
		hmd [i]=hmd_pose_queue_.front();
		
		sensor_in_x[i] = hmd[i].position.x;
		sensor_in_y[i] = hmd[i].position.y;
		sensor_in_z[i] = hmd[i].position.z;

	//	sensor_in_rw[i] = hmd[i].rotation.w;
	//	sensor_in_rx[i] = hmd[i].rotation.x;
	//	sensor_in_ry[i] = hmd[i].rotation.y;
	//	sensor_in_rz[i] = hmd[i].rotation.z;
	    hmd_pose_queue_.pop();
	}
	/////*linearSmooth3(sensor_in_x,sensor_out_x,sensor_size);
	////linearSmooth3(sensor_in_y, sensor_out_y, sensor_size);
	////linearSmooth3(sensor_in_z, sensor_out_z, sensor_size);*/
	//linearSmooth3(sensor_in_x, sensor_out_x, sensor_size);
	//linearSmooth3(sensor_in_y, sensor_out_y, sensor_size);
	//linearSmooth3(sensor_in_z, sensor_out_z, sensor_size);

	
	for (int i = 0; i < sensor_size; i++)
	{
		hmd[i].position.x = sensor_out_x[i];
		hmd[i].position.y = sensor_out_y[i];
		hmd[i].position.z = sensor_out_z[i];

		
		hmd_pose_queue_.push(hmd[i]);
		if (i == (sensor_size - 1))
		{
			last_hmd_pose_ = hmd[i];
		}
	}
 
	RVR::RVRControllerData left[10] = { 0 };
	for (int i =  0; i <sensor_size; i++)
	{
		left[i] = controller_pose_queue_[0].front();
		sensor_in_x[i] = left[i].position.x;
		sensor_in_y[i] = left[i].position.y;
		sensor_in_z[i] = left[i].position.z;

		
		controller_pose_queue_[0].pop();
	}
	linearSmooth5(sensor_in_x, sensor_out_x, sensor_size);
	linearSmooth5(sensor_in_y, sensor_out_y, sensor_size);
	linearSmooth5(sensor_in_z, sensor_out_z, sensor_size);

	for (int i = 0; i < sensor_size; i++)
	{
		left[i].position.x = sensor_out_x[i];
		left[i].position.y = sensor_out_y[i];
		left[i].position.z = sensor_out_z[i];

		
		controller_pose_queue_[0].push(left[i]);
		if (i==(sensor_size-1))
		{
			last_left_controller_pose_ = left[i];
		}
	}


	RVR::RVRControllerData right[10] = { 0 };
	for (int i = 0; i < sensor_size; i++)
	{
		right[i] = controller_pose_queue_[1].front();
		sensor_in_x[i] = right[i].position.x;
		sensor_in_y[i] = right[i].position.y;
		sensor_in_z[i] = right[i].position.z;

	
		controller_pose_queue_[1].pop();
	}
	linearSmooth5(sensor_in_x, sensor_out_x, sensor_size);
	linearSmooth5(sensor_in_y, sensor_out_y, sensor_size);
	linearSmooth5(sensor_in_z, sensor_out_z, sensor_size);

	for (int i = 0; i < sensor_size; i++)
	{
		right[i].position.x = sensor_out_x[i];
		right[i].position.y = sensor_out_y[i];
		right[i].position.z = sensor_out_z[i];
		
	
		controller_pose_queue_[1].push(right[i]);
		if (i == (sensor_size - 1))
		{
			last_right_controller_pose_ = right[i];
		}
	}
}
void sensor_add_queue::GetPoseWithSmooth(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose) 
{
	pose_lock_.lock();

	SmoothSensor();
	if (hmd_pose_queue_.size() > 0)
	{
		//get value in smoothsensor ,get new sensor
		//last_hmd_pose_ = hmd_pose = hmd_pose_queue_.front();
		//last_left_controller_pose_ = controller_pose_queue_[(int)ControllerIndex::kLeftController].front();
		//last_right_controller_pose_ = controller_pose_queue_[(int)ControllerIndex::kRightController].front();
		if (hmd_pose_queue_.size() >= gConfigReader.GetMaxSensorStoreValue())
		{
			hmd_pose_queue_.pop();
			controller_pose_queue_[(int)ControllerIndex::kLeftController].pop();
			controller_pose_queue_[(int)ControllerIndex::kRightController].pop();
		}

		last_get_time_ = RVR::nowInNs();
	}
	
	pose_lock_.unlock();
}
int sensor_add_queue::GetPoseWithTimeStore( RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose)
{
	 if ((RVR::nowInNs()- new_receive_ts_)>2000000000)
	 {
		 return 0;
	 }
	 
	pose_lock_.lock();
	int queue_size = hmd_pose_queue_.size();
	if (queue_size<=0)
	{
		pose_lock_.unlock();
		int64_t present_space = RVR::nowInNs() - last_present_ts_;
		int64_t predict_to = last_hmd_pose_.poseTimeStamp + present_space;
		PredictMotionRvrHmd(last_hmd_pose_, predict_to, hmd_pose);
		PredictMotionRvrController(last_left_controller_pose_, predict_to, left_controller_pose);
		PredictMotionRvrController(last_right_controller_pose_, predict_to, right_controller_pose);
		RVR_LOG_A("not find strore,predict flag=%d,use	 this sdk_t=%lld sub=%lld", 1, hmd_pose.poseTimeStamp, predict_to - hmd_pose.poseTimeStamp);
		RVR_LOG_A("predict in p %f,%f,%f q %f,%f,%f,%f v %f,%f,%f, av %f,%f,%f ts=%lld out p %f,%f,%f q %f,%f,%f,%f ts=%lld",
			last_hmd_pose_.position.x, last_hmd_pose_.position.y, last_hmd_pose_.position.z,
			last_hmd_pose_.rotation.w, last_hmd_pose_.rotation.x, last_hmd_pose_.rotation.y, last_hmd_pose_.rotation.z,
			last_hmd_pose_.linearVelocity.x, last_hmd_pose_.linearVelocity.y, last_hmd_pose_.linearVelocity.z,
			last_hmd_pose_.angularVelocity.x, last_hmd_pose_.angularVelocity.y, last_hmd_pose_.angularVelocity.z, last_hmd_pose_.poseTimeStamp,
			hmd_pose.position.x, hmd_pose.position.y, hmd_pose.position.z,
			hmd_pose.rotation.w, hmd_pose.rotation.x, hmd_pose.rotation.y, hmd_pose.rotation.z, hmd_pose.poseTimeStamp);
		 
	}
	else
	{
		hmd_pose = hmd_pose_queue_.front();
		left_controller_pose = controller_pose_queue_[(int)ControllerIndex::kLeftController].front();
		right_controller_pose= controller_pose_queue_[(int)ControllerIndex::kRightController].front();

		hmd_pose_queue_.pop();
		controller_pose_queue_[(int)ControllerIndex::kLeftController].pop();
		controller_pose_queue_[(int)ControllerIndex::kRightController].pop();
		pose_lock_.unlock();
		RVR_LOG_A("GET  p %f,%f,%f q %f,%f,%f,%f v %f,%f,%f, av %f,%f,%f ts=%lld ",
			hmd_pose.position.x, hmd_pose.position.y, hmd_pose.position.z,
			hmd_pose.rotation.w, hmd_pose.rotation.x, hmd_pose.rotation.y, hmd_pose.rotation.z,
			hmd_pose.linearVelocity.x, hmd_pose.linearVelocity.y, hmd_pose.linearVelocity.z,
			hmd_pose.angularVelocity.x, hmd_pose.angularVelocity.y, hmd_pose.angularVelocity.z, hmd_pose.poseTimeStamp);
	}
	 
	last_present_ts_ = RVR::nowInNs();
	last_hmd_pose_ = hmd_pose;
	last_left_controller_pose_ = left_controller_pose;
	last_right_controller_pose_ = right_controller_pose;
	
	 
	return 1;
}
void sensor_add_queue::GetPose(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose) 
{
	pose_lock_.lock();
	

	if (hmd_pose_queue_.size()>0)
	{
		last_hmd_pose_=hmd_pose = hmd_pose_queue_.front();
		last_left_controller_pose_= left_controller_pose = controller_pose_queue_[(int)ControllerIndex::kLeftController].front();
		last_right_controller_pose_= right_controller_pose = controller_pose_queue_[(int)ControllerIndex::kRightController].front();
		if (hmd_pose_queue_.size()>=gConfigReader.GetMaxSensorStoreValue())
		{
			hmd_pose_queue_.pop();
			controller_pose_queue_[(int)ControllerIndex::kLeftController].pop();
			controller_pose_queue_[(int)ControllerIndex::kRightController].pop();
		}
	
		last_get_time_ = RVR::nowInNs();
	}
	else
	{
		hmd_pose = last_hmd_pose_;
		left_controller_pose = last_left_controller_pose_;
		right_controller_pose = last_right_controller_pose_;
		
		if (last_hmd_pose_.valid!=false)
		{
			 
			GetPredictHmdPose(hmd_pose);
		}
		 

		if (last_left_controller_pose_.connectionState==RVR::RVRControllerConnectionState::kConnected)
		{
			 
			GetPredictControllerPose(left_controller_pose,(int)ControllerIndex::kLeftController);
		} 
		 
		
		if (last_right_controller_pose_.connectionState == RVR::RVRControllerConnectionState::kConnected)
		{
			 
			GetPredictControllerPose(right_controller_pose, (int)ControllerIndex::kRightController);
		}
		
	}
	
	pose_lock_.unlock();
}


void sensor_add_queue::GetPoseWithTimestamp(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose, int64_t last_present_ts) 
{
	if (last_present_ts_==0)
	{
		pose_lock_.lock();
		while (hmd_pose_queue_.size()>0)
		{
			hmd_pose = hmd_pose_queue_.front();
			hmd_pose_queue_.pop();

			left_controller_pose = controller_pose_queue_[0].front();
			controller_pose_queue_[0].pop();

			right_controller_pose = controller_pose_queue_[1].front();
			controller_pose_queue_[1].pop();
		}
		pose_lock_.unlock();
		
	}
	else
	{
		int64_t now_ts = RVR::nowInNs();//
		bool get_loop = true;
		while (get_loop)
		{
			int queue_size = 0;

			pose_lock_.lock();

			queue_size = hmd_pose_queue_.size();
			if (queue_size <= 0)
			{

				pose_lock_.unlock();
				Sleep(1);
				continue;
			}
			else
			{
				hmd_pose = hmd_pose_queue_.front();
				hmd_pose_queue_.pop();

				left_controller_pose = controller_pose_queue_[0].front();
				controller_pose_queue_[0].pop();

				right_controller_pose = controller_pose_queue_[1].front();
				controller_pose_queue_[1].pop();

				if ((hmd_pose.poseTimeStamp - last_use_sdk_ts_)  > (now_ts - last_present_ts_))
				{
					///get
					get_loop = false;
					RVR_LOG_A("hmd get ts=%lld ,last_sdk_t=%lld ,subsdk=%lld,pc_t=%lld,last_pst=%lld,subpc=%lld,sub sdk-pc=%lld",
						hmd_pose.poseTimeStamp, last_use_sdk_ts_, hmd_pose.poseTimeStamp - last_use_sdk_ts_,
						now_ts, last_present_ts_, now_ts - last_present_ts_, hmd_pose.poseTimeStamp - last_use_sdk_ts_ - (now_ts - last_present_ts_));

				}
				else
				{
					RVR_LOG_A("hmd ts=%lld ,last_sdk_t=%lld ,subsdk=%lld,pc_t=%lld,last_pst=%lld,subpc=%lld,sub sdk-pc=%lld",
						hmd_pose.poseTimeStamp, last_use_sdk_ts_, hmd_pose.poseTimeStamp - last_use_sdk_ts_,
						now_ts, last_present_ts_, now_ts - last_present_ts_, hmd_pose.poseTimeStamp - last_use_sdk_ts_ - (now_ts - last_present_ts_));
				}

			}


			pose_lock_.unlock();

		}

	}


	last_use_sdk_ts_ = hmd_pose.poseTimeStamp;

	last_present_ts_ = RVR::nowInNs();

}
void sensor_add_queue::GetPredictHmdPose(RVR::RVRPoseHmdData& hmd_pose) 
{
	int64_t timestamp= RVR::nowInNs();
	int64_t predict_to_time = last_hmd_pose_.poseTimeStamp + timestamp - last_get_time_;
	PredictMotionRvrHmd(last_hmd_pose_, predict_to_time, hmd_pose);
}
void sensor_add_queue::GetPredictControllerPose(RVR::RVRControllerData& controller_pose, int controller_index)
{
	int64_t timestamp = RVR::nowInNs();
	int64_t predict_to_time = 0;
	if (controller_index==(int)ControllerIndex::kLeftController)
	{
		predict_to_time=last_left_controller_pose_.timestamp + timestamp - last_get_time_;
		PredictMotionRvrController(last_left_controller_pose_,predict_to_time,controller_pose);
	} 
	else if (controller_index == (int)ControllerIndex::kRightController)
	{
		predict_to_time = last_right_controller_pose_.timestamp + timestamp - last_get_time_;
		PredictMotionRvrController(last_right_controller_pose_, predict_to_time, controller_pose);
	}	
}