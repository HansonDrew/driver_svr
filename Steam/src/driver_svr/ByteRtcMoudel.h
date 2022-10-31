#pragma once
#include <streaming_rtc_engine_interface.h>
#include <string>
#include <windows.h>
#define RTC_BUF_SIZE 8*1024*1024
using namespace  bytertc;
namespace pico_streaming
{
	class ByteRtcMoudel : public bytertc::streamingrtc::IStreamingRTCEventHandler
	{
	protected:
		ByteRtcMoudel() {};
		~ByteRtcMoudel() { engine_startup_ =false;};
	public:
		bool Startup(char* server_ip, char* device_id);
		bool Shutdown();
		bool SetVideoConfig(int width, int height, int fps, int max_send_kbps, int codec_type);
		bool SetAudioConfig(int recording_channel, int  recording_sample_rate, int playback_channel, int playback_sample_rate);
		bool SendVideoFrame(uint8_t* data, int length, int picture_type, int eye_inedex, int64_t gen_timestamp);
		bool SendAudioFrame(uint8_t* data, int length);
		bool GetEngineStartup() { return engine_startup_; };
		bool SendMsg(char* buf, int len);
		bool SendMsg(char* buf );
		void OnLogReport(const char* log_type, const char* log_content);
		std::string dst_ip_;
		void OnWarning(int code);

		void OnError(int code);

		void OnConnectionStateChanged(streamingrtc::ConnectionState state);

		void OnP2PMessageSendResult(int64_t msg_id, int error);

		void OnP2PMessageReceived(const char* message);

		

		//bool OnRemoteVideoFrame(IVideoFrame* video_frame, streamingrtc::StreamIndex video_index);

		void OnConnectionStats(const streamingrtc::ConnectionStats& stats);

		void OnLocalStreamStats(const streamingrtc::LocalStreamStats& stats);
		void OnRemoteStreamStats(const streamingrtc::RemoteStreamStats& stats);

		
		void OnMessageReceived(const char* message);
		void OnP2PBinaryMessageReceived(const uint8_t* message, int size);
		void OnLocalExternalEncoderStart(streamingrtc::StreamIndex stream_index);
		void OnRequestLocalExternalEncoderKeyFrame(streamingrtc::StreamIndex stream_index, int32_t video_index);
		void OnSuggestLocalExternalEncoderRateUpdate(streamingrtc::StreamIndex stream_index, int32_t video_index, streamingrtc::VideoRateInfo info);
		static ByteRtcMoudel* GetInstance();
		static unsigned int __stdcall  CheckSensorReceiveThread(void* lpParameter);
		int64_t last_hmd_recv_timestamp = -1;
		int64_t last_left_controller_recv_timestamp = -1;
		int64_t last_right_controller_recv_timestamp = -1;
		bool left_idr_flag_ = false;
		bool right_idr_flag_ = false;
		bool left_update_rate_flag_ = false;
		bool right_update_rate_flag_ = false;
		int left_suggest_rate_ = 0;
		int right_suggest_rate_ = 0;
		class GC
		{
		public:
			GC()
			{
			}
			~GC()
			{
				if (gByteRtcMoudelInstance != NULL)
				{
					delete gByteRtcMoudelInstance;
					gByteRtcMoudelInstance = NULL;
				}
			}
		};
		static GC gc;  //垃圾回收类的静态成员
	private:
		HANDLE checksensor_thread_event_ = INVALID_HANDLE_VALUE;
		uint8_t* left_buf_;
		uint8_t* right_buf_;
		int send_buf_len_;
		static ByteRtcMoudel* gByteRtcMoudelInstance;
		bool engine_startup_ = false;
		bytertc::streamingrtc::IStreamingRTCEngine* engine_ = nullptr;
		int width_ = 1472;
		int height_ = 1472;
		int fps_ = 72;
		int max_send_kbps_ = 60000;
		int codec_type_ = 0;
		int audio_recording_channel_ = 2;
		int audio_recording_sample_rate_ = 48000;

		int audio_playback_channel_ = 2;
		int audio_playback_sample_rate_ = 48000;
		int64_t gen_time_stamp_ = -1;
		bool connect_ = false;
	};


}
