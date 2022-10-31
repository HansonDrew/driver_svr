 
#include "ByteRtcMoudel.h"
#include <bytertc_video_frame.h>
#include <string> 
#include "RVRLogger.h"
#include "RVRPluginDefinitions.h"
#include "SensorManger.h"
#include "config_reader.h"
#include "SeiHelp.h"
#include "driver_define.h"
#include "SensorManger.h"
#include "driver_pico.h"
#include "TimeTool.h"
using namespace RVR;
extern ConfigReader gConfigReader;
extern PicoVRDriver g_svrDriver;
namespace pico_streaming
{
	ByteRtcMoudel* ByteRtcMoudel::gByteRtcMoudelInstance = nullptr;
	bool ByteRtcMoudel::Startup(char* server_ip, char* device_id)
	{
		 
		left_buf_ = new uint8_t[RTC_BUF_SIZE];
		right_buf_ = new uint8_t[RTC_BUF_SIZE];
		send_buf_len_ = RTC_BUF_SIZE;

		bytertc::streamingrtc::EngineConfig config;
		config.app_id = "62133291eaee1f00a0987acb";
		config.role = bytertc::streamingrtc::kP2PRoleServer;
		config.server_ip = server_ip;
		config.server_port_min = 50000;
		config.server_port_max = 50004;
		std::string device_id_str = device_id;
		 
		std::string params = "{\"rtc.device_id\":\"" + device_id_str + "\"" + "}";
		engine_ = bytertc::streamingrtc::CreateStreamingRTCEngine(config, this, params.c_str());
		if (engine_ == nullptr)
		{
			RVR_LOG_A("CreateStreamingRTCEngine error!");
			return false;
		}

		bytertc::VideoSolution vs[1];
		vs[0].width = width_;
		vs[0].height = height_;
		vs[0].fps = fps_;
		vs[0].max_send_kbps = max_send_kbps_;
		int ret=engine_->SetVideoEncoderConfig(bytertc::streamingrtc::kStreamIndexIndex0, vs, 1);
		if (ret != 0)
		{
			//RVR_LOG_A("SetVideoEncoderConfig kStreamIndexIndex0 error!");
			return false;
		}
		ret = engine_->SetVideoEncoderConfig(bytertc::streamingrtc::kStreamIndexIndex1, vs, 1);
		if (ret != 0)
		{
			//RVR_LOG_A("SetVideoEncoderConfig kStreamIndexIndex1 error!");
			return false;
		}

		bytertc::streamingrtc::AudioFormat recording_format;
		recording_format.channel = (bytertc::streamingrtc::AudioChannel)audio_recording_channel_;
		recording_format.sample_rate = (bytertc::streamingrtc::AudioSampleRate)audio_recording_sample_rate_;
		bytertc::streamingrtc::AudioFormat playback_format;
		playback_format.channel = (bytertc::streamingrtc::AudioChannel)audio_playback_channel_;
		playback_format.sample_rate = (bytertc::streamingrtc::AudioSampleRate)audio_playback_sample_rate_;
		engine_->SetAudioConfig(recording_format, playback_format);

		engine_->SetVideoSourceType(bytertc::streamingrtc::kStreamIndexIndex0, bytertc::streamingrtc::VideoSourceTypeEncodedWithoutAutoSimulcast);
		engine_->SetVideoSourceType(bytertc::streamingrtc::kStreamIndexIndex1, bytertc::streamingrtc::VideoSourceTypeEncodedWithoutAutoSimulcast);

		ret = engine_->Startup();
		if (ret == 0)
		{
			engine_startup_ = true;
			
			_beginthreadex(NULL, 0, &CheckSensorReceiveThread, this, 0, NULL);
			DriverLog("RTCEngine Startup !");
		}
		else
		{
			DriverLog("RTCEngine Startup error!");
		}

		return ret == 0 ? true : false;
	}
	bool ByteRtcMoudel::Shutdown() 
	{
		int ret = engine_->Shutdown();
		if (ret == 0)
		{
			engine_startup_ = false;
			WaitForSingleObject(pico_streaming::ByteRtcMoudel::GetInstance()->checksensor_thread_event_, INFINITE);
			DriverLog("RTCEngine Shutdown !");
		}
		else
		{
			DriverLog("RTCEngine Shutdown error!");
		}

		return ret == 0 ? true : false;
	}
	bool ByteRtcMoudel::SetVideoConfig(int width, int height, int fps, int max_send_kbps, int codec_type)
	{
		width_ = width;
		height_ = height;
		fps_ = fps;
		max_send_kbps_ = max_send_kbps;
		codec_type_ = codec_type;
		DriverLog("SetVideoConfig width=%d height=%d fps=%d max_send_kbps=%d codec_type=%d",
			width_,height_,fps_,max_send_kbps_,codec_type_);
		return true;
	}
	bool ByteRtcMoudel::SetAudioConfig(int recording_channel, int  recording_sample_rate, int playback_channel, int playback_sample_rate)
	{
		audio_recording_channel_ = recording_channel;
		audio_recording_sample_rate_ = recording_sample_rate;
		audio_playback_channel_ = playback_channel;
		audio_playback_sample_rate_ = playback_sample_rate;
		DriverLog("SetAudioConfig  recording_channel_=%d sample_rate=%d playback_channel=%d sample_rate=%d  ",
			audio_recording_channel_, audio_recording_sample_rate_, audio_playback_channel_, audio_playback_sample_rate_);
		return true;
	}
	
	
	//FILE* fp = fopen("rtc.h265","wb+");
	bool idr = false;
	bool ByteRtcMoudel::SendVideoFrame(uint8_t* data, int length, int picture_type, int eye_inedex, int64_t gen_timestamp)
	{
		if (connect_ == false) 
		{
			return false;
		}
		RVR::RVRPoseHmdData sensor;
		SensorManger::GetInstance()->GetSensor(gConfigReader.BigPicture() == 1 ? PictureControlType::KBigPicture : PictureControlType::KLittlePicture, eye_inedex, sensor);
		int send_len = send_buf_len_;
		uint8_t* send_data = eye_inedex == 0 ? left_buf_ : right_buf_;
		memset(send_data, 0, sizeof(uint8_t) * send_len);

		WireLessType::AddPoseData add_data;
		WireLessType::AddQuaternion add_quaternion;
		WireLessType::AddVector3 add_vector3;
		add_quaternion.w = -sensor.rotation.w;
		add_quaternion.x = -sensor.rotation.x;
		add_quaternion.y = -sensor.rotation.y;
		add_quaternion.z = -sensor.rotation.z;
		add_data.rotation = add_quaternion;

		add_vector3.x = sensor.position.x;
		add_vector3.y = sensor.position.y;
		add_vector3.z = sensor.position.z;
		add_data.position = add_vector3;
		add_data.hmdPoseTimeTs = sensor.poseRecvTime;
		add_data.poseTimeStamp = sensor.poseTimeStamp;
		add_data.predictedTimeMs = sensor.predictedTimeMs;
		bool ret=pico_streaming::EncodedFramePutSei(gConfigReader.GetHEVC(), data, length, (uint8_t*)&add_data, sizeof(WireLessType::AddPoseData), send_data, send_len);
		bytertc::EncodedVideoFrameBuilder builder;
		builder.width = width_;
		builder.height = height_;
		builder.data = send_data;
		builder.picture_type = (bytertc::VideoPictureType)picture_type;
		builder.size = send_len;
		builder.codec_type = (bytertc::VideoCodecType)codec_type_;
		builder.timestamp_dts_us = gen_timestamp;
		builder.timestamp_us = gen_timestamp;
		builder.memory_deleter = [](uint8_t* data, int size, void* user_opaque) -> int { return 0; };
		auto frame = bytertc::streamingrtc::BuildStreamingEncodedVideoFrame(builder);
		if (picture_type== bytertc::VideoPictureType::kVideoPictureTypeI)
		{
			DriverLog("streamindex %d send i frame frameindex=%llu", eye_inedex, add_data.poseTimeStamp);
		}
		ret = false;
		
		if (eye_inedex == 0)
		{
			
			/*int itype = ((data[4]) & 0x7E) >> 1;
			if (itype==32)
			{
				idr = true;
			}
			if (idr)
			{
				fwrite(data, sizeof(char), length, fp);
			}*/
			ret = engine_->PushEncodedVideoFrame(bytertc::streamingrtc::kStreamIndexIndex0, 0, frame);
			
		}
		else
		{
			
			
			ret = engine_->PushEncodedVideoFrame(bytertc::streamingrtc::kStreamIndexIndex1, 0, frame);
			
		}
		if (ret==false)
		{
			DriverLog("send video fasle");
		}
		return ret;
	}
	
	
	bool ByteRtcMoudel::SendAudioFrame(uint8_t* date, int samples)
	{
		//DriverLog("audio samples%d", samples);
		//fwrite(date,sizeof(char),samples*4,paudio);
		bool ret= engine_->PushAudioFrame((int8_t*)date, samples);
		return ret;
	}
	bool ByteRtcMoudel::SendMsg(char* buf, int len)
	{
		return engine_->SendP2PBinaryMessage((uint8_t*)buf, len, true);		
	}

	bool ByteRtcMoudel::SendMsg(char* buf)
	{
		return engine_->SendP2PMessage(buf,true);
	}
	void ByteRtcMoudel::OnLogReport(const char* log_type, const char* log_content)
	{

		//RVR_LOG_A("rtclog OnLogReport :%s,%s", log_type, log_content);
	}

	void ByteRtcMoudel::OnWarning(int code)
	{
		//RVR_LOG_A("rtclog OnWarning :%d", code);
	}

	void ByteRtcMoudel::OnError(int code)
	{
		//RVR_LOG_A("rtclog OnError :%d", code);
	}

	void ByteRtcMoudel::OnConnectionStateChanged(streamingrtc::ConnectionState state)
	{
		DriverLog("rtclog OnConnectionStateChanged :%d", state);
		if (state== streamingrtc::ConnectionState::kConnectionStateConnected|| state == streamingrtc::ConnectionState::kConnectionStateReconnected)
		{
			left_idr_flag_ = right_idr_flag_ = true;
			connect_ = true;
		}
		if (state==streamingrtc::ConnectionState::kConnectionStateDisconnected)
		{
			connect_ = false;
		}
	}

	void ByteRtcMoudel::OnP2PMessageSendResult(int64_t msg_id, int error)
	{
	}

	void ByteRtcMoudel::OnP2PMessageReceived(const char* message)
	{
		std::string msg = message;
		int cut_index = msg.find(":");
		if (cut_index==std::string::npos)
		{
			return;
		}
		std::string msg_type = msg.substr(0, cut_index);
		if (msg_type.compare("ip")==0)
		{
			dst_ip_ = msg.substr(cut_index + 1, msg.length() - 1 - cut_index);
		}

		if (msg_type.compare("idr_request") == 0)
		{
			std::string index = msg.substr(cut_index + 1, msg.length() - 1 - cut_index);
			if (index.compare("0") == 0)
			{
				left_idr_flag_ = true;
			} 
			else if(index.compare("1") == 0)
			{
				right_idr_flag_ = true;
			}
			else if (index.compare("2") == 0)
			{
				left_idr_flag_ = right_idr_flag_ = true;
			}
			DriverLog("pico idr_request %s", index.c_str());
		}
	}

	//bool OnRemoteVideoFrame(IVideoFrame* video_frame, streamingrtc::StreamIndex video_index);
	void ByteRtcMoudel::OnLocalStreamStats(const streamingrtc::LocalStreamStats& stats) 
	{
	}
	void ByteRtcMoudel::OnConnectionStats(const streamingrtc::ConnectionStats& stats)
	{
		 
		//RVR_LOG_A("rtclog OnConnectionStats :%d", stats.rtt);
	}

 
	void ByteRtcMoudel::OnRemoteStreamStats(const streamingrtc::RemoteStreamStats& stats)
	{
	}
	void ByteRtcMoudel::OnLocalExternalEncoderStart(streamingrtc::StreamIndex stream_index) 
	{
		if (stream_index== streamingrtc::StreamIndex::kStreamIndexIndex0)
		{
			left_idr_flag_ = true;
		} 
		else if(stream_index == streamingrtc::StreamIndex::kStreamIndexIndex1)
		{
			right_idr_flag_ = true;
		}
	}
	void ByteRtcMoudel::OnMessageReceived(const char* message) 
	{
		 
	}
	ByteRtcMoudel* ByteRtcMoudel::GetInstance()
	{
	
	   if (gByteRtcMoudelInstance == NULL)
	   {
		   gByteRtcMoudelInstance = new ByteRtcMoudel();
		   
	   }
	  return gByteRtcMoudelInstance;
		
	}
	unsigned int __stdcall ByteRtcMoudel::CheckSensorReceiveThread(void* lpParameter) 
	{
		pico_streaming::ByteRtcMoudel::GetInstance()->checksensor_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
		while (pico_streaming::ByteRtcMoudel::GetInstance()->GetEngineStartup())
		{
			int64_t timestamp = GetTimestampUs();
			if (pico_streaming::ByteRtcMoudel::GetInstance()->last_hmd_recv_timestamp>0&&
				(timestamp- pico_streaming::ByteRtcMoudel::GetInstance()->last_hmd_recv_timestamp)>5000000)
			{
				RVR::RVRPoseHmdData hmd_pose = {0};
				hmd_pose.rotation.w = 1;
				hmd_pose.valid = false;
				g_svrDriver.AddHmdPose(&hmd_pose);
			}

			if (pico_streaming::ByteRtcMoudel::GetInstance()->last_left_controller_recv_timestamp > 0 &&
				(timestamp - pico_streaming::ByteRtcMoudel::GetInstance()->last_left_controller_recv_timestamp) > 5000000)
			{
				RVR::RVRControllerData controller_data;
				controller_data.connectionState= RVR::RVRControllerConnectionState::kDisconnected;
				g_svrDriver.AddControllerPose(0,&controller_data);
			}

			if (pico_streaming::ByteRtcMoudel::GetInstance()->last_right_controller_recv_timestamp > 0 &&
				(timestamp - pico_streaming::ByteRtcMoudel::GetInstance()->last_right_controller_recv_timestamp) > 5000000)
			{
				RVR::RVRControllerData controller_data;
				controller_data.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
				g_svrDriver.AddControllerPose(1,&controller_data);
			}

			Sleep(2000);
		}
		SetEvent(pico_streaming::ByteRtcMoudel::GetInstance()->checksensor_thread_event_);
		return 0;
	}
	void ByteRtcMoudel::OnP2PBinaryMessageReceived(const uint8_t* message, int size)
	{
		if (message[0]!=0x13||message[size-1]!=0xEE||size<sizeof(WireLessType::TransPoseData))
		{
			
			return;
		}
		if (message[1]==0x05)
		{
			RVR::RVRPoseHmdData hmd_pose = {0};
			SensorManger::GetInstance()->ConvertToHmdSensor((uint8_t*)(message + 2), size - 3, hmd_pose);
			/*DriverLog("get hmd_pose   r:%f,%f,%f,%f,p:%f,%f,%f\n",
				hmd_pose.rotation.w, hmd_pose.rotation.x, hmd_pose.rotation.y, hmd_pose.rotation.z,
				hmd_pose.position.x, hmd_pose.position.y, hmd_pose.position.z);*/
			last_hmd_recv_timestamp = GetTimestampUs();
   		    g_svrDriver.AddHmdPose(&hmd_pose);
			return ;
		}
		else if (message[1]== 0x06)
		{
			RVR::RVRControllerData controller_pose;
			SensorManger::GetInstance()->ConvertToControllerSensor((uint8_t*)(message + 2), size - 3,0, controller_pose);
			last_left_controller_recv_timestamp = GetTimestampUs();
			/*RVR_LOG_A("left_controll t =%d r:%f,%f,%f,%f,p:%f,%f,%f,v:%f,%f,%f ", 
				last_left_controller_recv_timestamp,controller_pose.rotation.w, controller_pose.rotation.x, controller_pose.rotation.y, controller_pose.rotation.z,
				controller_pose.position.x, controller_pose.position.y, controller_pose.position.z,
				controller_pose.vecVelocity.x, controller_pose.position.y, controller_pose.position.z);*/
			g_svrDriver.AddControllerPose(0,&controller_pose);
			return;
		}
		else if (message[1] == 0x07) 
		{
			RVR::RVRControllerData controller_pose;
			SensorManger::GetInstance()->ConvertToControllerSensor((uint8_t*)(message + 2), size - 3, 1, controller_pose);
			last_right_controller_recv_timestamp = GetTimestampUs();
			/*RVR_LOG_A("right_controll t = % d r : % f, % f, % f, % f, p : % f, % f, % f, v : % f, % f, % f ",
				last_left_controller_recv_timestamp, controller_pose.rotation.w, controller_pose.rotation.x, controller_pose.rotation.y, controller_pose.rotation.z,
				controller_pose.position.x, controller_pose.position.y, controller_pose.position.z,
				controller_pose.vecVelocity.x, controller_pose.position.y, controller_pose.position.z);*/
			g_svrDriver.AddControllerPose(1, &controller_pose);
			return;
		}
		else 
		{
			//RVR_LOG_A("OnP2PBinaryMessageReceived error size :%d", size);
		}
	}

	void ByteRtcMoudel::OnRequestLocalExternalEncoderKeyFrame(streamingrtc::StreamIndex stream_index, int32_t video_index)
	{
		if (stream_index== streamingrtc::StreamIndex::kStreamIndexIndex0)
		{
			
			DriverLog("RTC Request keyframe left");
			left_idr_flag_ = true;
		}
		else if (stream_index == streamingrtc::StreamIndex::kStreamIndexIndex1)
		{
			
			DriverLog("RTC Request keyframe right");
			right_idr_flag_ = true;
		}
	}
	void ByteRtcMoudel::OnSuggestLocalExternalEncoderRateUpdate(streamingrtc::StreamIndex stream_index, int32_t video_index, streamingrtc::VideoRateInfo info) 
	{
		int suggest_rate = 0;
		if (stream_index== bytertc::streamingrtc::kStreamIndexIndex0)
		{
			
			left_suggest_rate_ = info.bitrate_kbps * 1000;
			suggest_rate = left_suggest_rate_;
			left_update_rate_flag_ = true;
		} 
		else if (stream_index == bytertc::streamingrtc::kStreamIndexIndex1)
		{
			
			right_suggest_rate_ = info.bitrate_kbps * 1000;
			suggest_rate = right_suggest_rate_;
			right_update_rate_flag_ = true;
		}
		
		DriverLog("RTC SuggestLocalExternalEncoderRateUpdate %d rate%d", stream_index, suggest_rate);
		
	}
}
