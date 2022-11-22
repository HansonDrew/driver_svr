#include "sensor_passer.h"
#include "driver_pico.h"
#include "RVRLogger.h"
#include "PredictPose/picocontroller_interface.h"
#include "sensor_add_queue.h"
#include "config_reader.h"
#include "RVRLogger.h"
extern ConfigReader gConfigReader;
extern PicoVRDriver g_svrDriver; 
SensorPasser* SensorPasser::sensor_passer_instance_;
SensorPasser::GC SensorPasser::sensor_passer_gc_;
extern bool gDashboardActivated;
extern bool gOverlayShow;
SensorPasser::SensorPasser()
{
	new_sensor_notify_ = CreateEvent(NULL, FALSE, FALSE, NULL);
}

SensorPasser::~SensorPasser()
{

}
void SensorPasser::SetNewSensorNotifyEvent() 
{
	SetEvent(new_sensor_notify_);
	//RVR_LOG_A("set event");
}
int64_t last_recv_time = -1;


void SensorPasser::AddNewSensorWithPredict(int64_t present_sub_time)
{
	RVR::RVRPoseHmdData hmd_pose = { 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };
	 
	int64_t predict_to_time = 0;
	
	GetHmdSensor(hmd_pose);
	GetControllerSensor(left_controller_pose, ControllerIndex::kLeftController);
	GetControllerSensor(right_controller_pose, ControllerIndex::kRightController);


	if (hmd_pose.poseTimeStamp<=last_sdk_time_) 
	{
		predict_to_time = last_sdk_time_ + present_sub_time;
	}
	hmd_mutex_.lock();
	if (predict_to_time > 0)
	{
		RVR::RVRPoseHmdData input_sensor = hmd_pose_;

		PredictMotionRvrHmd(input_sensor, predict_to_time, hmd_pose_);
		RVR_LOG_A("predict in p_x %f,%f,%f v %f.%f,%f out %f ,%f ,%f",
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.linearVelocity.x, input_sensor.linearVelocity.y, input_sensor.linearVelocity.z,
			hmd_pose_.position.x, hmd_pose_.position.y, hmd_pose_.position.z);
		 
	}
	hmd_pose = hmd_pose_;
	hmd_mutex_.unlock();
	
	//	g_svrDriver.AddHmdPose(&hmd_pose_);

	
	controller_mutex_[0].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[0];

		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[0]);
		RVR_LOG_A("predict  left in r %f,%f,%f,%f p %f,%f,%f v %f,%f,%f  av %f,%f,%f, out  r %f,%f,%f,%f , p %f ,%f ,%f in_t %lld  to_t %lld",
			input_sensor.rotation.w, input_sensor.rotation.x, input_sensor.rotation.y, input_sensor.rotation.z,
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.vecVelocity.x, input_sensor.vecVelocity.y, input_sensor.vecVelocity.z,
			input_sensor.vecAngularVelocity.x, input_sensor.vecAngularVelocity.y, input_sensor.vecAngularVelocity.z,
			controller_pose_[0].rotation.w, controller_pose_[0].rotation.x, controller_pose_[0].rotation.y, controller_pose_[0].rotation.z,
			controller_pose_[0].position.x, controller_pose_[0].position.y, controller_pose_[0].position.z,
			input_sensor.timestamp, predict_to_time);
	}

	controller_mutex_[0].unlock();
	//g_svrDriver.AddControllerPose(0, &controller_pose_[0]);
	left_controller_pose = controller_pose_[0];
	controller_mutex_[1].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[1];

		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[1]);
	}
	//g_svrDriver.AddControllerPose(1, &controller_pose_[1]);
	right_controller_pose = controller_pose_[1];
	controller_mutex_[1].unlock();
	

	last_sdk_time_ = hmd_pose.poseTimeStamp;
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);
	//	
}
void SensorPasser::AddNewSensorWithPredictAndKalmanFilter(int64_t present_sub_time)
{
	RVR::RVRPoseHmdData hmd_pose = { 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };

	int64_t predict_to_time = 0;

	GetHmdSensor(hmd_pose);
	GetControllerSensor(left_controller_pose, ControllerIndex::kLeftController);
	GetControllerSensor(right_controller_pose, ControllerIndex::kRightController);


	if (hmd_pose.poseTimeStamp <= last_sdk_time_)
	{
		predict_to_time = last_sdk_time_ + present_sub_time;
	}
	hmd_mutex_.lock();
	if (predict_to_time > 0)
	{
		RVR::RVRPoseHmdData input_sensor = hmd_pose_;

		PredictMotionRvrHmd(input_sensor, predict_to_time, hmd_pose_);
		RVR_LOG_A("predict in p_x %f,%f,%f v %f.%f,%f out %f ,%f ,%f",
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.linearVelocity.x, input_sensor.linearVelocity.y, input_sensor.linearVelocity.z,
			hmd_pose_.position.x, hmd_pose_.position.y, hmd_pose_.position.z);

	}
	hmd_pose = hmd_pose_;
	hmd_mutex_.unlock();

	//	g_svrDriver.AddHmdPose(&hmd_pose_);


	controller_mutex_[0].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[0];

		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[0]);
		RVR_LOG_A("predict  left in r %f,%f,%f,%f p %f,%f,%f v %f,%f,%f  av %f,%f,%f, out  r %f,%f,%f,%f , p %f ,%f ,%f in_t %lld  to_t %lld",
			input_sensor.rotation.w, input_sensor.rotation.x, input_sensor.rotation.y, input_sensor.rotation.z,
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.vecVelocity.x, input_sensor.vecVelocity.y, input_sensor.vecVelocity.z,
			input_sensor.vecAngularVelocity.x, input_sensor.vecAngularVelocity.y, input_sensor.vecAngularVelocity.z,
			controller_pose_[0].rotation.w, controller_pose_[0].rotation.x, controller_pose_[0].rotation.y, controller_pose_[0].rotation.z,
			controller_pose_[0].position.x, controller_pose_[0].position.y, controller_pose_[0].position.z,
			input_sensor.timestamp, predict_to_time);
	}

	controller_mutex_[0].unlock();
	//g_svrDriver.AddControllerPose(0, &controller_pose_[0]);
	left_controller_pose = controller_pose_[0];
	controller_mutex_[1].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[1];

		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[1]);
	}
	//g_svrDriver.AddControllerPose(1, &controller_pose_[1]);
	right_controller_pose = controller_pose_[1];
	controller_mutex_[1].unlock();
	LeftKalman.DoKalma(left_controller_pose);
	RightKalman.DoKalma(right_controller_pose);
	///  
	last_sdk_time_ = hmd_pose.poseTimeStamp;
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);
	//	


}

bool SensorPasser::AddSensorByTimestamp(int64_t last_present_time)
{
	RVR::RVRPoseHmdData hmd_pose = { 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };
	 
	g_svrDriver.sensor_queue_.GetPoseWithTimestamp(hmd_pose, left_controller_pose, right_controller_pose,last_present_time);
	 
	last_get_sdk_time_ = hmd_pose.poseTimeStamp;
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);
	//	

	return true;
}
void SensorPasser::AddSensorByNotifyEventWithSmooth(int64_t present_sub_time)
{
	RVR::RVRPoseHmdData hmd_pose = { 0 }; 
	RVR::RVRControllerData left_controller_pose = { 0 }; 
	RVR::RVRControllerData right_controller_pose = { 0 };
	DWORD ret= WaitForSingleObject(new_sensor_notify_, 6);
	int64_t predict_to_time = 0;
	if (ret== WAIT_TIMEOUT)
	{
		//超时，预测一个。进入平滑
	    predict_to_time = last_sdk_time_ + present_sub_time+5000;
	
	}
	 
	hmd_mutex_.lock();
	int64_t  sdk_time_space = hmd_pose_.poseTimeStamp - last_sdk_time_;
	
	//RVR_LOG_A("present tsp=%lld sdk tsp=%lld recv tsp=%lld", sub_time, hmd_pose_.poseTimeStamp - last_sdk_time, hmd_pose_.poseRecvTime - last_recv_time);
	last_recv_time = hmd_pose_.poseRecvTime;
	 	 
	if (predict_to_time >0)
	{
		RVR::RVRPoseHmdData input_sensor = hmd_pose_; 
		
		PredictMotionRvrHmd(input_sensor, predict_to_time,hmd_pose_);
		RVR_LOG_A("predict in p_x %f,%f,%f v %f.%f,%f out %f ,%f ,%f", 
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.linearVelocity.x, input_sensor.linearVelocity.y, input_sensor.linearVelocity.z,
			hmd_pose_.position.x, hmd_pose_.position.y, hmd_pose_.position.z);
	}
	
	
//	g_svrDriver.AddHmdPose(&hmd_pose_);
	hmd_pose = hmd_pose_;
	hmd_mutex_.unlock();
	controller_mutex_[0].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[0];
		 
		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[0]);
		RVR_LOG_A("predict  left in r %f,%f,%f,%f p %f,%f,%f v %f,%f,%f  av %f,%f,%f, out  r %f,%f,%f,%f , p %f ,%f ,%f in_t %lld  to_t %lld",
			input_sensor.rotation.w, input_sensor.rotation.x, input_sensor.rotation.y, input_sensor.rotation.z,
			input_sensor.position.x, input_sensor.position.y, input_sensor.position.z,
			input_sensor.vecVelocity.x, input_sensor.vecVelocity.y, input_sensor.vecVelocity.z,
			input_sensor.vecAngularVelocity.x, input_sensor.vecAngularVelocity.y, input_sensor.vecAngularVelocity.z,
			controller_pose_[0].rotation.w, controller_pose_[0].rotation.x, controller_pose_[0].rotation.y, controller_pose_[0].rotation.z,
			controller_pose_[0].position.x, controller_pose_[0].position.y, controller_pose_[0].position.z,
			input_sensor.timestamp, predict_to_time);
	}
	
	controller_mutex_[0].unlock();
	//g_svrDriver.AddControllerPose(0, &controller_pose_[0]);
	left_controller_pose = controller_pose_[0];
	controller_mutex_[1].lock();
	if (predict_to_time > 0)
	{
		RVR::RVRControllerData input_sensor = controller_pose_[1];
	 
		PredictMotionRvrController(input_sensor, predict_to_time, controller_pose_[1]);
	}
	//g_svrDriver.AddControllerPose(1, &controller_pose_[1]);
	right_controller_pose= controller_pose_[1];
	controller_mutex_[1].unlock();
	int64_t start_t = nowInNs();
    g_svrDriver.sensor_queue_.SetPose(hmd_pose, left_controller_pose, right_controller_pose);

	g_svrDriver.sensor_queue_.GetPoseWithSmooth(hmd_pose, left_controller_pose, right_controller_pose);
	
	int64_t end_t = nowInNs();
	RVR_LOG_A("smooth cost=%lld",end_t-start_t);

	last_sdk_time_ = hmd_pose_.poseTimeStamp;
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);
//	
 
	 
}
void SensorPasser::AddSensorByNotifyEvent(int64_t present_time,  int wait_time)
{
	timeBeginPeriod(1);
	RVR::RVRPoseHmdData  hmd_pose={ 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };
	
	DWORD ret = WaitForSingleObject(new_sensor_notify_, wait_time);

	all_mutex_.lock();
	hmd_pose = hmd_pose_;
	left_controller_pose = controller_pose_[0];
	right_controller_pose = controller_pose_[1];
	all_mutex_.unlock();
	//RVR_LOG_A("wait event ok");
	if (ret== WAIT_TIMEOUT)
	{
		return;
	}
	int64_t ts = RVR::nowInNs();
	int delay_time = ts - last_present_time_ - 1000000000 / gConfigReader.GetFps();
	present_time = RVR::nowInNs();
	int64_t present_offset = present_time - last_present_time_;
	//if (present_offset<5000000)
	//{
	//	//RVR_LOG_A("present_offset too little");
	//	present_time = nowInNs();
	//	ResetNewSensorNotyfyEvent();
	//	
	//	AddSensorByNotifyEvent(present_time);
	//	timeEndPeriod(1);
	//	return;
	//}
	
	int64_t sdk_offset = hmd_pose.poseTimeStamp - last_get_sdk_time_;
	int64_t recv_offset = hmd_pose.poseRecvTime - last_sensor_get_time_;
	bool predict = false;
	/*if ((last_present_time_>0) && ((present_offset - per_frame)>2500000))
	{
		predict = true;

		//RVR_LOG_A("need predict %lld sdk %lld predict-sdk =%lld",present_offset-per_frame,sdk_offset-per_frame,present_offset-sdk_offset);
		//RVR_LOG_A("present with predict offset=%lld sdk offset=%lld recv offset=%lld", (present_offset) / 1000, (sdk_offset) / 1000, (hmd_pose.poseRecvTime - last_sensor_get_time_) / 1000);
		last_get_sdk_time_ = hmd_pose.poseTimeStamp;
		last_present_time_ = present_time;
		last_sensor_get_time_ = hmd_pose.poseRecvTime;
		RVR::RVRPoseHmdData input_sensor = hmd_pose_;
		double predict_to_time = hmd_pose_.poseTimeStamp + recv_offset -per_frame;
		PredictMotionRvrHmd(input_sensor, predict_to_time, hmd_pose_);

		predict_to_time = left_controller_pose.timestamp + recv_offset - per_frame;
		RVR::RVRControllerData input_left_sensor =left_controller_pose;
		PredictMotionRvrController(input_left_sensor, predict_to_time, left_controller_pose);

		predict_to_time = right_controller_pose.timestamp + recv_offset - per_frame;
		RVR::RVRControllerData input_right_sensor = right_controller_pose;
		PredictMotionRvrController(input_right_sensor, predict_to_time, right_controller_pose);
		//if (gConfigReader.GetAppRunValue() == 2 || gOverlayShow || gDashboardActivated || gConfigReader.GetAppRunValue() == 0)
		{
			//HmdKalman.DoKalma(hmd_pose);
			//LeftKalman.DoKalma(left_controller_pose);
			//RightKalman.DoKalma(right_controller_pose);
		}
	}*/
	int64_t addts = nowInNs();
	//RVR_LOG_A("sensor sub=%lld", (addts -hmd_pose.poseRecvTime)/1000000);
	//g_svrDriver.AddControllerPose(0, &left_controller_pose);
	//g_svrDriver.AddControllerPose(1, &right_controller_pose);
	//g_svrDriver.AddHmdPose(&hmd_pose);
	if (predict==false)
	{// 不需要预测的，直接使用sensor  ，使用后 做卡尔曼滤波，该滤波主要是给需要预测时做平滑用。需要预测时做预测，预测后进卡尔曼进steamvr，不需要再次卡尔曼
	//	HmdKalman.DoKalma(hmd_pose);
		//LeftKalman.DoKalma(left_controller_pose);
		//.DoKalma(right_controller_pose);
		//RVR_LOG_A("present offset=%lld sdk offset=%lld recv offset=%lld", (present_offset) / 1000, (sdk_offset) / 1000, (hmd_pose.poseRecvTime - last_sensor_get_time_) / 1000);
		last_get_sdk_time_ = hmd_pose.poseTimeStamp;
		last_present_time_ = present_time;
		last_sensor_get_time_ = hmd_pose.poseRecvTime;
	}
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);

	g_svrDriver.GetStreamingHmdDriver()->hmd_data = hmd_pose;
	g_svrDriver.GetStreamingHmdDriver()->left_data = left_controller_pose;
	g_svrDriver.GetStreamingHmdDriver()->right_data = right_controller_pose;
	char msg[2048] = { 0 };
	sprintf_s(msg, "add predict %d rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf   \n",
		predict,
		hmd_pose.rotation.w, hmd_pose.rotation.x, hmd_pose.rotation.y, hmd_pose.rotation.z,
		hmd_pose.position.x, hmd_pose.position.y, hmd_pose.position.z);
	//RVR_LOG_A(msg);
	timeEndPeriod(1);
}


void SensorPasser::AddSensorByNotifyEventWithWait(int64_t present_time, int wait_time)
{
	timeBeginPeriod(1);
	RVR::RVRPoseHmdData  hmd_pose = { 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };
	int64_t wait_begin = nowInNs();
	DWORD ret = WaitForSingleObject(new_sensor_notify_, wait_time);
	int64_t wait_end = nowInNs();
	int64_t wait_cost = (wait_end - wait_begin)/1000000;

	int64_t sleep_base = 24;
	sleep_base = sleep_base - wait_cost;
	/*if (sleep_base>1)
	{
		Sleep(sleep_base);
	}
	RVR_LOG_A("wait =%lld ,sleep=%lld", wait_cost,sleep_base);*/

	all_mutex_.lock();
	hmd_pose = hmd_pose_;
	left_controller_pose = controller_pose_[0];
	right_controller_pose = controller_pose_[1];
	all_mutex_.unlock();
	//RVR_LOG_A("wait event ok");
	if (ret == WAIT_TIMEOUT)
	{
		return;
	}
	int64_t ts = RVR::nowInNs();
	int delay_time = ts - last_present_time_ - 1000000000 / gConfigReader.GetFps();
	present_time = RVR::nowInNs();
	int64_t present_offset = present_time - last_present_time_;
	//if (present_offset<5000000)
	//{
	//	//RVR_LOG_A("present_offset too little");
	//	present_time = nowInNs();
	//	ResetNewSensorNotyfyEvent();
	//	
	//	AddSensorByNotifyEvent(present_time);
	//	timeEndPeriod(1);
	//	return;
	//}

	int64_t sdk_offset = hmd_pose.poseTimeStamp - last_get_sdk_time_;
	int64_t recv_offset = hmd_pose.poseRecvTime - last_sensor_get_time_;
	
	// 不需要预测的，直接使用sensor  ，使用后 做卡尔曼滤波，该滤波主要是给需要预测时做平滑用。需要预测时做预测，预测后进卡尔曼进steamvr，不需要再次卡尔曼
		//HmdKalman.DoKalma(hmd_pose);
		//LeftKalman.DoKalma(left_controller_pose);
		//RightKalman.DoKalma(right_controller_pose);
		RVR_LOG_A("present offset=%lld sdk offset=%lld recv offset=%lld", (present_offset) / 1000, (sdk_offset) / 1000, (hmd_pose.poseRecvTime - last_sensor_get_time_) / 1000);
		last_get_sdk_time_ = hmd_pose.poseTimeStamp;
		last_present_time_ = present_time;
		last_sensor_get_time_ = hmd_pose.poseRecvTime;
	
	g_svrDriver.AddControllerPose(0, &left_controller_pose);
	g_svrDriver.AddControllerPose(1, &right_controller_pose);
	g_svrDriver.AddHmdPose(&hmd_pose);

	timeEndPeriod(1);
}
void SensorPasser::AddSensorWithTimeStore( )
{

	RVR::RVRPoseHmdData hmd_pose = { 0 };
	RVR::RVRControllerData left_controller_pose = { 0 };
	RVR::RVRControllerData right_controller_pose = { 0 };
	if (g_svrDriver.sensor_queue_.GetPoseWithTimeStore(hmd_pose, left_controller_pose, right_controller_pose)==1)
	{

		g_svrDriver.AddControllerPose(0, &left_controller_pose);
		g_svrDriver.AddControllerPose(1, &right_controller_pose);
		g_svrDriver.AddHmdPose(&hmd_pose);
	}
	
	

}
void SensorPasser::ResetNewSensorNotyfyEvent() 
{
	ResetEvent(new_sensor_notify_);
}
 
SensorPasser* SensorPasser::GetInstance()
{
	if (sensor_passer_instance_ == NULL)
	{
		sensor_passer_instance_ = new SensorPasser();
		sensor_passer_instance_->per_frame = 1000000000 / gConfigReader.GetFps();
	}
	return sensor_passer_instance_;
}

void SensorPasser::SetHmdSensor(RVR::RVRPoseHmdData hmd_pose_t)
{
	hmd_mutex_.lock();
	hmd_pose_ = hmd_pose_t;
	last_sdk_time_ = hmd_pose_.poseTimeStamp;
	hmd_mutex_.unlock();
}
void  SensorPasser::SetAllSensor(RVR::RVRPoseHmdData hmd_pose_t, RVR::RVRControllerData left_controller_pose, RVR::RVRControllerData right_controller_pose)
{
	int sleep_time = 24;
	all_mutex_.lock();
	hmd_pose_ = hmd_pose_t;
	
	last_sensor_recv_time_ = hmd_pose_.poseRecvTime;

	controller_pose_[0] = left_controller_pose;
	controller_pose_[1] = right_controller_pose;
	all_mutex_.unlock();
	g_svrDriver.sensor_queue_.SetPose(hmd_pose_t, left_controller_pose, right_controller_pose);
	 //RVR::RVR_LOG_A("sensor:i=%d,h=%f,%f,%f,%f,%f,%f,%f,%f,%f\
		//r=%f,%f,%f,%f,%f,%f,%f,%f,%f,ts=%lld,h_i=%d",
		//hmd_pose_.hmd_index,
		//hmd_pose_.position.x, hmd_pose_.position.y, hmd_pose_.position.z,
		//hmd_pose_.linearVelocity.x, hmd_pose_.linearVelocity.y, hmd_pose_.linearVelocity.z,
		//hmd_pose_.angularVelocity.x, hmd_pose_.angularVelocity.y, hmd_pose_.angularVelocity.z,
		// right_controller_pose.position.x, right_controller_pose.position.y, right_controller_pose.position.z,
		// right_controller_pose.vecVelocity.x, right_controller_pose.vecVelocity.y, right_controller_pose.vecVelocity.z,
		// right_controller_pose.vecAngularVelocity.x, right_controller_pose.vecAngularVelocity.y, right_controller_pose.vecAngularVelocity.z,
		//hmd_pose_.poseTimeStamp,hmd_pose_.hmd_index);

	SetEvent(new_sensor_notify_);

	
}
void SensorPasser::SetControllerSensor(RVR::RVRControllerData controller_pose_t, ControllerIndex controller_index)
{
	controller_mutex_[int(controller_index)].lock();
	controller_pose_[int(controller_index)] = controller_pose_t;
	controller_mutex_[int(controller_index)].unlock();
}

void SensorPasser::GetHmdSensor(RVR::RVRPoseHmdData& hmd_pose_t)
{
	hmd_mutex_.lock();
	hmd_pose_t = hmd_pose_;
	hmd_mutex_.unlock();
}
void SensorPasser::GetControllerSensor(RVR::RVRControllerData& controller_pose_t, ControllerIndex controller_index)
{
	controller_mutex_[int(controller_index)].lock();
	controller_pose_t=controller_pose_[int(controller_index)];
	controller_mutex_[int(controller_index)].unlock();
}