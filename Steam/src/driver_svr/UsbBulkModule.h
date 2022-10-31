#pragma once
#include "pxr_base/transport/transport_server.h"
#include "pxr_base/transport/transport_session.h"
#include "pxr_base/logger.h"
#include "pxr_base/time_util.h"

#include "../RVRPluginDefinitions.h"
#include "../RVRPlugin/IVEncPlugin.h"
#include <string>
#include <mutex>
#define  AUDIOLEN 1920

#define USB_BUF_SIZE 8*1024*1024
using namespace pxr_base;
namespace pico_streaming
{
	class UsbBulkModule : public TransportServerCallback, public TransportSessionCallback
	{
	public:
        static UsbBulkModule* GetInstance();
		void Start();
		void Shutdown();

		bool SendVideoFrame(uint8_t* data, int length, int picture_type,
			int eye_inedex, int64_t gen_timestamp,EncodeOutWithInfo encode_info);
		bool SendAudioFrame(uint8_t* data, int length, int samples);
		bool GetEngineStartup() { return engine_startup_; };
		bool SendMsg(char* buf, int len);
		RVR::RVRPoseHmdData hmd_pose_ = { 0 };
		RVR::RVRControllerData left_pose_ = { 0 };
		RVR::RVRControllerData right_pose_ = { 0 };

	private:
        UsbBulkModule();
        ~UsbBulkModule();

        //TransportServerCallback
        virtual void NotifyClientConnected(TransportSession::Ptr_t session);
        virtual void NotifyServerStopped(int res);

        //TransportSessionCallback
        virtual void NotifySessionStopped(int res);
        virtual void NotifyReceiveData(const TransportData& data);
		void HandleAudioData(const TransportData& data);
		void HandleSensorData(const TransportData& data);
		void CheckSensorReceiveThread();
		inline bool CheckUsbMode();
	private:
        void TestFuncSaveVideoFrame(uint8_t* data, int length, int picture_type,
            int eye_inedex, int64_t gen_timestamp);
		void TestFuncSaveAudioFrame(uint8_t* data, int length);

	private:
		std::atomic_bool engine_startup_;

		std::atomic_bool connected_;
		TransportServer::Ptr_t server_;
		TransportSession::Ptr_t session_;
		//SendData:0-video  1-audio  2-msg
		std::recursive_mutex video_mutex_;
		TransportData video_send_buffer_;
		TransportData audio_send_buffer_;
		TransportData msg_send_buffer_;

		char audio_recv_buffer_[AUDIOLEN] = { 0 };
		size_t audio_recv_count_ = 0;

		std::shared_ptr<std::thread> check_sensor_thread_;
		uint64_t last_hmd_recv_timestamp_ = 0;
		uint64_t last_left_controller_recv_timestamp_ = 0;
		uint64_t last_right_controller_recv_timestamp_ = 0;
	};
}
