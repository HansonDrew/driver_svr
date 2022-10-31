/*
 *  Copyright (c) 2021 The ByteRtc project authors. All Rights Reserved.
 *  @company ByteDance.Inc
 *  @brief 局域网串流引擎接口
 */
#pragma once

#ifdef WIN32
    #define STREAMING_RTC_API extern "C" __declspec(dllexport)
#elif __APPLE__
#include <TargetConditionals.h>
    #if TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
        #define STREAMING_RTC_API __attribute__((__visibility__("default"))) extern "C"
    #else
        #define STREAMING_RTC_API __attribute__((__visibility__("default")))
    #endif
#else
    #define STREAMING_RTC_API __attribute__((__visibility__("default")))
#endif

#ifdef __ANDROID__
#include "jni.h"
#endif

#include "bytertc_video_frame.h"

namespace bytertc {
namespace streamingrtc {
enum P2PRole {
    kP2PRoleClient = 0,
    kP2PRoleServer = 1,
};

struct EngineConfig {
    const char* app_id = nullptr;
    P2PRole role = kP2PRoleClient;
    const char* server_ip = nullptr;
    int server_port_min = 50000;
    int server_port_max = 50004;
};

enum StreamIndex{
    kStreamIndexIndex0,
    kStreamIndexIndex1
};

/**
* @type keytype
* @brief 音频采样率，单位为 HZ。
*/
enum AudioSampleRate {
    /**
    * @brief 8000 采样率
    */
    kAudioSampleRate8000 = 8000,
    /**
    * @brief 16000 采样率
    */
    kAudioSampleRate16000 = 16000,
    /**
    * @brief 32000 采样率
    */
    kAudioSampleRate32000 = 32000,
    /**
    * @brief 44100 采样率
    */
    kAudioSampleRate44100 = 44100,
    /**
    * @brief 48000 采样率
    */
    kAudioSampleRate48000 = 48000
};

/**
* @type keytype
* @brief 音频声道
*/
enum AudioChannel {
    /**
    * @brief 单声道
    */
    kAudioChannelMono = 1,
    /**
    * @brief 双声道
    */
    kAudioChannelStereo = 2
};

/**
* @type keytype
* @brief 音频参数格式
*/
struct AudioFormat {
    /**
    * @brief 音频采样率，详见 AudioSampleRate{@link #AudioSampleRate}
    */
    AudioSampleRate sample_rate;
    /**
    * @brief 音频声道，详见 AudioChannel{@link #AudioChannel}
    */
    AudioChannel channel;
};


/**
* @type keytype
* @brief 视频输入源类型
*/
enum VideoSourceType {
    /**
    *  自定义外部采集视频源
    */
    VideoSourceTypeExternal = 0,
    /**
    *  外部编码视频源, simulcast小流需要主动push
    */
    VideoSourceTypeEncodedWithoutAutoSimulcast = 3,
};

enum VideoSinkType {
    kVideoSinkTypeRaw, // 开启视频解码器，只回調解码后数据
    kVideoSinkTypeEncode, // 关闭视频解码器，只回調解码前数据
    kVideoSinkTypeBoth,  // 开启视频解码器，同时回調解码前、解码后数据
};

/**
 * @type keytype
 * @brief 局域网P2P连接状态。
 */
enum ConnectionState {
    /**
     * @brief 连接断开。
     */
    kConnectionStateDisconnected = 1,
    /**
     * @brief 首次连接，正在连接中。
     */
    kConnectionStateConnecting = 2,
    /**
     * @brief 首次连接成功。
     */
    kConnectionStateConnected = 3,
    /**
     * @brief 连接断开后重新连接中。
     */
    kConnectionStateReconnecting = 4,
    /**
     * @brief 连接断开后重连成功。
     */
    kConnectionStateReconnected = 5,
};

/**
* @type keytype
* @brief 通话相关的统计信息
*/
struct ConnectionStats {
    /**
    * @brief 当前应用的上行丢包率，取值范围为 [0, 1]。
    */
    float txLostrate;
    /**
    * @brief 当前应用的下行丢包率，取值范围为 [0, 1]。
    */
    float rxLostrate;
    /**
    * @brief 客户端到服务端数据传输的往返时延（单位 ms）
    */
    int rtt;
    /**
    * @brief 通话时长，单位为秒，累计值
    */
    unsigned int duration;
    /**
    * @brief 发送码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short tx_kbitrate;
    /**
    * @brief 接收码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short rx_kbitrate;
    /**
    * @brief 音频接收码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short rx_audio_kbitrate;
    /**
    * @brief 音频发送码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short tx_audio_kbitrate;
    /**
    * @brief 视频接收码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short rx_video_kbitrate;
    /**
    * @brief 视频发送码率 (kbps)，获取该数据时的瞬时值
    */
    unsigned short tx_video_kbitrate;
    /**
    * @brief 当前房间内的用户人数
    */
    unsigned int user_count;
    /**
    * @hidden
    * @brief 当前应用程序的 CPU 使用率（%），暂未被使用
    */
    double cpu_app_usage;
    /**
    * @hidden
    * @brief 当前系统的 CPU 使用率（%），暂未被使用
    */
    double cpu_total_usage;
};



/**
* @type keytype
* @brief 本地音频流统计信息，统计周期为 2s 。  <br>
*        本地用户发布音频流成功后，SDK 会周期性地通过 OnLocalStreamStats{@link #OnLocalStreamStats}
*        通知用户发布的音频流在此次统计周期内的发送状况。此数据结构即为回调给用户的参数类型。  <br>
*/
struct LocalAudioStats {
    /**
    * @brief 音频丢包率。此次统计周期内的音频上行丢包率，单位为 % ，取值范围为 [0, 1]。  <br>
    */
    float audio_loss_rate;
    /**
    * @brief 发送码率。此次统计周期内的音频发送码率，单位为 kbps 。  <br>
    */
    int send_kbitrate;
    /**
    * @brief 采集采样率。此次统计周期内的音频采集采样率信息，单位为 Hz 。  <br>
    */
    int record_sample_rate;
    /**
    * @brief 统计间隔。此次统计周期的间隔，单位为 ms 。  <br>
    * @notes 此字段用于设置回调的统计周期，默认设置为 2s 。
    */
    int stats_interval;
    /**
    * @brief 往返时延。单位为 ms 。  <br>
    */
    int rtt;
    /**
    * @brief 音频声道数。  <br>
    */
    int num_channels;
    /**
    * @brief 音频发送采样率。此次统计周期内的音频发送采样率信息，单位为 Hz 。  <br>
    */
    int sent_sample_rate;
};

/**
* @type keytype
* @brief 本地视频流统计信息，统计周期为 2s 。  <br>
*        本地用户发布视频流成功后，SDK 会周期性地通过 OnLocalStreamStats{@link #OnLocalStreamStats}
*        通知用户发布的视频流在此次统计周期内的发送状况。此数据结构即为回调给用户的参数类型。  <br>
*/
struct LocalVideoStats {
    /**
    * @brief 发送码率。此次统计周期内的视频发送码率，单位为 kbps 。
    */
    int sent_kbitrate;
    /**
    * @brief 采集帧率。此次统计周期内的视频采集帧率，单位为 fps 。
    */
    int input_frame_rate;
    /**
    * @brief 发送帧率。此次统计周期内的视频发送帧率，单位为 fps 。
    */
    int sent_frame_rate;
    /**
    * @brief 编码器输出帧率。当前编码器在此次统计周期内的输出帧率，单位为 fps 。
    */
    int encoder_output_frame_rate;
    /**
    * @brief 本地渲染帧率。此次统计周期内的本地视频渲染帧率，单位为 fps 。
    */
    int renderer_output_frame_rate;
    /**
    * @brief 目标发送码率。此次统计周期内的视频目标发送码率，单位为 kbps 。
    */
    int target_kbitrate;
    /**
    * @brief 目标发送帧率。当前编码器在此次统计周期内的目标发送帧率，单位为 fps 。
    */
    int target_frame_rate;
    /**
    * @brief 统计间隔，单位为 ms 。
    * @notes 此字段用于设置回调的统计周期，默认设置为 2s 。
    */
    int stats_interval;
    /**
    * @brief 视频丢包率。此次统计周期内的视频上行丢包率，取值范围为 [0，1] 。
    */
    float video_loss_rate;
    /**
    * @brief 往返时延，单位为 ms 。
    */
    int rtt;
    /**
    * @brief 视频编码码率。此次统计周期内的视频编码码率，单位为 kbps 。
    */
    int encoded_bitrate;
    /**
    * @brief 视频编码宽度，单位为 px 。
    */
    int encoded_frame_width;
    /**
    * @brief 视频编码高度，单位为 px 。
    */
    int encoded_frame_height;
    /**
    * @brief 此次统计周期内发送的视频帧总数。
    */
    int encoded_frame_count;
    /**
    * @brief 视频的编码类型，具体参考 VideoCodecType{@link #VideoCodecType} 。
    */
    VideoCodecType codec_type;
    /**
    * @brief 所属用户的媒体流是否为屏幕流。你可以知道当前统计数据来自主流还是屏幕流。
    */
    StreamIndex stream_index;
};

/**
* @type keytype
* @brief 媒体流网络质量。
*/
enum NetworkQuality {
    /**
    * @brief 网络质量未知。
    */
    kNetworkQualityUnknown = 0,
    /**
    * @brief 网络质量极好。
    */
    kNetworkQualityExcellent,
    /**
    * @brief 主观感觉和 kNetworkQualityExcellent 差不多，但码率可能略低。
    */
    kNetworkQualityGood,
    /**
    * @brief 主观感受有瑕疵但不影响沟通。
    */
    kNetworkQualityPoor,
    /**
    * @brief 勉强能沟通但不顺畅。
    */
    kNetworkQualityBad,
    /**
    * @brief 网络质量非常差，基本不能沟通。
    */
    kNetworkQualityVbad,
};

struct LocalStreamStats {
    /**
    * @brief 本地设备发送音频流的统计信息，详见 LocalAudioStats{@link #LocalAudioStats}
    */
    LocalAudioStats audio_stats;
    /**
    * @brief 本地设备发送视频流的统计信息，详见 LocalVideoStats{@link #LocalVideoStats}
    */
    LocalVideoStats video_stats;
    /**
    * @brief 所属用户的媒体流上行网络质量，详见 NetworkQuality{@link #NetworkQuality}
    */
    NetworkQuality local_tx_quality;
    /**
    * @brief 所属用户的媒体流下行网络质量，详见 NetworkQuality{@link #NetworkQuality}
    */
    NetworkQuality local_rx_quality;
    /**
    * @brief 所属用户的媒体流是否为屏幕流。你可以知道当前统计数据来自主流还是屏幕流。
    */
    StreamIndex stream_index;
};

/**
* @type keytype
* @brief 远端音频流统计信息，统计周期为 2s。  <br>
*        本地用户订阅远端音频流成功后，SDK 会周期性地通过 OnRemoteStreamStats{@link #OnRemoteStreamStats} 通知本地用户订阅的音频流在此次统计周期内的接收状况。此数据结构即为回调给本地用户的参数类型。  <br>
*/
struct RemoteAudioStats {
    /**
    * @brief 音频丢包率。统计周期内的音频下行丢包率，取值范围为 [0, 1] 。  <br>
    */
    float audio_loss_rate;
    /**
    * @brief 接收码率。统计周期内的音频接收码率，单位为 kbps 。  <br>
    */
    int received_kbitrate;
    /**
    * @brief 音频卡顿次数。统计周期内的卡顿次数。  <br>
    */
    int stall_count;
    /**
    * @brief 音频卡顿时长。统计周期内的卡顿时长，单位为 ms 。  <br>
    */
    int stall_duration;
    /**
    *@brief 用户体验级别的端到端延时。从发送端采集完成编码开始到接收端解码完成渲染开始的延时，单位为 ms 。  <br>
    */
    long e2e_delay;
    /**
    * @brief 播放采样率。统计周期内的音频播放采样率信息，单位为 Hz 。  <br>
    */
    int playout_sample_rate;
    /**
    * @brief 统计间隔。此次统计周期的间隔，单位为 ms 。  <br>
    */
    int stats_interval;
    /**
    * @brief 客户端到服务端数据传输的往返时延，单位为 ms。 <br>
    */
    int rtt;
    /**
    * @brief 发送端——服务端——接收端全链路数据传输往返时延。单位为 ms 。  <br>
    */
    int total_rtt;
    /**
    * @brief 远端用户发送的音频流质量。值含义参考 NetworkQuality{@link #NetworkQuality} 。  <br>
    */
    int quality;
    /**
    * @brief 因引入 jitter buffer 机制导致的延时。单位为 ms 。  <br>
    */
    int jitter_buffer_delay;
    /**
    * @brief 音频声道数。  <br>
    */
    int num_channels;
    /**
    * @brief 音频接收采样率。统计周期内接收到的远端音频采样率信息，单位为 Hz 。  <br>
    */
    int received_sample_rate;
    /**
    * @brief 远端用户在加入房间后发生音频卡顿的累计时长占音频总有效时长的百分比。音频有效时长是指远端用户进房发布音频流后，除停止发送音频流和禁用音频模块之外的音频时长。
    */
    int frozen_rate;
    /**
    * @brief 音频 PLC 样点总个数。  <br>
    */
    int concealed_samples;
    /**
    * @brief PLC 累计次数。  <br>
    */
    int concealment_event;
    /**
    * @brief 音频解码采样率。统计周期内的音频解码采样率信息，单位为 Hz 。  <br>
    */
    int dec_sample_rate;
    /**
    * @brief 解码时长。对此次统计周期内接收的远端音频流进行解码的总耗时，单位为 s 。  <br>
    */
    int dec_duration;
};

/**
* @type keytype
* @brief 远端音频流统计信息，统计周期为 2s 。  <br>
*        本地用户订阅远端音频流成功后，SDK 会周期性地通过 OnRemoteStreamStats{@link #OnRemoteStreamStats}
*        通知本地用户订阅的远端视频流在此次统计周期内的接收状况。此数据结构即为回调给本地用户的参数类型。  <br>
*/
struct RemoteVideoStats {
    /**
    * @brief 视频宽度
    */
    int width;
    /**
    * @brief 视频高度
    */
    int height;
    /**
    * @brief 视频丢包率。统计周期内的视频下行丢包率，单位为 % ，取值范围：[0，1] 。
    */
    float video_loss_rate;
    /**
    * @brief 接收码率。统计周期内的视频接收码率，单位为 kbps 。
    */
    int received_kbitrate;
    /**
    * @brief 解码器输出帧率。统计周期内的视频解码器输出帧率，单位 fps 。
    */
    int decoder_output_frame_rate;
    /**
    * @brief 渲染帧率。统计周期内的视频渲染帧率，单位 fps 。
    */
    int renderer_output_frame_rate;
    /**
    * @brief 卡顿次数。统计周期内的卡顿次数。
    */
    int stall_count;
    /**
    * @brief 卡顿时长。统计周期内的视频卡顿总时长。单位 ms 。
    */
    int stall_duration;
    /**
    * @brief 用户体验级别的端到端延时。从发送端采集完成编码开始到接收端解码完成渲染开始的延时，单位为 ms 。
    */
    long e2e_delay;
    /**
    * @brief 所属用户的媒体流是否为屏幕流。你可以知道当前数据来自主流还是屏幕流。
    */
    bool is_screen;
    /**
    * @brief 统计间隔，此次统计周期的间隔，单位为 ms 。  <br>
    * @notes 此字段用于设置回调的统计周期，目前设置为 2s 。
    */
    int stats_interval;
    /**
    * @brief 往返时延，单位为 ms 。
    */
    int rtt;
    /**
    * @brief 远端用户在进房后发生视频卡顿的累计时长占视频总有效时长的百分比（%）。视频有效时长是指远端用户进房发布视频流后，除停止发送视频流和禁用视频模块之外的视频时长。
    */
    int frozen_rate;
    /**
    * @brief 对应多种分辨率的流的下标，详见 VideoSolutionDescription{@link #VideoSolutionDescription}
    */
    int video_index;
};

/**
* @type keytype
* @brief 用户订阅的远端音/视频流统计信息以及网络状况，统计周期为 2s 。  <br>
*        订阅远端用户发布音/视频流成功后，SDK 会周期性地通过 OnRemoteStreamStats{@link #OnRemoteStreamStats}
*        通知本地用户订阅的远端音/视频流在此次统计周期内的接收状况。此数据结构即为回调给本地用户的参数类型。  <br>
*/
struct RemoteStreamStats {
    /**
    * @brief 远端音频流的统计信息，详见 RemoteAudioStats{@link #RemoteAudioStats}
    */
    RemoteAudioStats audio_stats;
    /**
    * @brief 远端视频流的统计信息，详见 RemoteVideoStats{@link #RemoteVideoStats}
    */
    RemoteVideoStats video_stats;
    /**
    * @brief 所属用户的媒体流上行网络质量，详见 NetworkQuality{@link #NetworkQuality}
    */
    NetworkQuality remote_tx_quality;
    /**
    * @brief 所属用户的媒体流下行网络质量，详见 NetworkQuality{@link #NetworkQuality}
    */
    NetworkQuality remote_rx_quality;
    /**
    * @brief 所属用户的媒体流是否为屏幕流。你可以知道当前统计数据来自主流还是屏幕流。
    */
    StreamIndex stream_index;
};

struct VideoRateInfo {
    int fps;
    int bitrate_kbps;
};

class IStreamingRTCEngine;
class IStreamingRTCEventHandler;

#ifdef __ANDROID__
STREAMING_RTC_API void SetApplicationContext(jobject j_application_context);
#endif

STREAMING_RTC_API IStreamingRTCEngine* CreateStreamingRTCEngine(const EngineConfig& config,
        IStreamingRTCEventHandler* handler, const char* params);

STREAMING_RTC_API void DestroyStreamingRTCEngine(IStreamingRTCEngine* engine);

STREAMING_RTC_API const char* GetStreamingSdkVersion();

STREAMING_RTC_API IVideoFrame* BuildStreamingVideoFrame(const VideoFrameBuilder& builder);

STREAMING_RTC_API IEncodedVideoFrame* BuildStreamingEncodedVideoFrame(const EncodedVideoFrameBuilder& builder);

STREAMING_RTC_API void SetStreamingEngineEngineAppState(IStreamingRTCEngine* engine, const char* app_state);

class IStreamingRTCEngine {
public:
    virtual int Startup() = 0;

    virtual int Shutdown() = 0;

    virtual int64_t SendP2PMessage(const char* message, bool reliable) = 0;

    virtual int64_t SendP2PBinaryMessage(const uint8_t* message, int length, bool reliable) = 0;

    virtual bool PushVideoFrame(streamingrtc::StreamIndex stream_index, IVideoFrame* frame) = 0;

    virtual int SetVideoEncoderConfig(streamingrtc::StreamIndex stream_index, const VideoSolution* solutions, int solution_num) = 0;

    virtual void SetAudioConfig(const streamingrtc::AudioFormat &recording_format, const streamingrtc::AudioFormat &playback_format) = 0;

    virtual bool PullAudioFrame(int8_t* data, int samples) = 0;

    virtual bool PushAudioFrame(int8_t* data, int samples) = 0;

    // encoded api
    virtual void SetVideoSourceType(streamingrtc::StreamIndex stream_index, streamingrtc::VideoSourceType type) = 0;

    // virtual void RegisterLocalEncodedVideoFrameObserver(ILocalEncodedVideoFrameObserver* observer) = 0;

    // virtual void RegisterRemoteEncodedVideoFrameObserver(IRemoteEncodedVideoFrameObserver* observer) = 0;

    // virtual void SetExternalVideoEncoderEventHandler(IExternalVideoEncoderEventHandler* encoder_handler) = 0;

    virtual bool PushEncodedVideoFrame(streamingrtc::StreamIndex stream_index, int video_index, IEncodedVideoFrame* video_stream) = 0;

    virtual void SetVideoSinkType(streamingrtc::StreamIndex stream_index, streamingrtc::VideoSinkType type) = 0;

    virtual void RequestRemoteVideoKeyFrame(streamingrtc::StreamIndex stream_index) = 0;

    virtual void SetReportExtraInfo(const char* extra_info)=0;

    /**
     * @brief 析构函数
     */
    virtual ~IStreamingRTCEngine() {}
};

class IStreamingRTCEventHandler {
public:
    virtual void OnLogReport(const char* log_type, const char* log_content) {
        (void)log_type;
        (void)log_content;
    }

    virtual void OnWarning(int code) {
        (void)code;
    }

    virtual void OnError(int code) {
        (void)code;
    }

    virtual void OnConnectionStateChanged(streamingrtc::ConnectionState state) {
        (void)state;
    }

    virtual void OnP2PMessageSendResult(int64_t msg_id, int error) {
        (void)msg_id;
        (void)error;
    }

    virtual void OnP2PMessageReceived(const char* message) {
        (void)message;
    }

    virtual void OnP2PBinaryMessageReceived(const uint8_t* message, int size) {
        (void)message;
        (void)size;
    }

    virtual bool OnRemoteVideoFrame(IVideoFrame* video_frame, streamingrtc::StreamIndex video_index) {
        (void)video_frame;
        (void)video_index;
		return true;
    }

    virtual void OnConnectionStats(const streamingrtc::ConnectionStats& stats) {
        (void)stats;
    }

    virtual void OnLocalStreamStats(const streamingrtc::LocalStreamStats& stats) {
        (void)stats;
    }

    virtual void OnRemoteStreamStats(const streamingrtc::RemoteStreamStats& stats) {
        (void)stats;
    }

    // 直推264， 265
    virtual bool OnRemoteEncodedVideoFrame(const IEncodedVideoFrame& encoded_video_frame, streamingrtc::StreamIndex video_index) {
        (void)encoded_video_frame;
        (void)video_index;
        return true;
    }

    virtual void OnLocalExternalEncoderStart(streamingrtc::StreamIndex stream_index) {
        (void) stream_index;
    }
    virtual void OnLocalExternalEncoderStop(streamingrtc::StreamIndex stream_index) {
        (void) stream_index;
    }
    virtual void OnSuggestLocalExternalEncoderRateUpdate(streamingrtc::StreamIndex stream_index, int32_t video_index, streamingrtc::VideoRateInfo info) {
        (void) stream_index;
        (void) video_index;
        (void) info;
    }
    virtual void OnRequestLocalExternalEncoderKeyFrame(streamingrtc::StreamIndex stream_index, int32_t video_index){
        (void) stream_index;
        (void) video_index;
    }

    /**
     * @brief 析构函数
     */
    virtual ~IStreamingRTCEventHandler() {}
};

} // namespace streamingrtc
} // namespace bytertc
