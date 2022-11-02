/*
 * Copyright (c) 2020 The VolcEngineRTC project authors. All Rights Reserved.
 * @brief VolcEngine Interface Lite
 */

#pragma once

#ifndef BYTE_RTC_LITE_INTERFACE_H__
#define BYTE_RTC_LITE_INTERFACE_H__

#include "bytertc_room_interface.h"
#include "bytertc_device_manager.h"
#include "bytertc_audio_frame.h"

namespace bytertc {

class IExternalVideoEncoderEventHandler {
public:
    virtual ~IExternalVideoEncoderEventHandler(){}
    virtual void OnStart(StreamIndex index) = 0;
    virtual void OnStop(StreamIndex index) = 0;
    virtual void OnRateUpdate(StreamIndex index, int32_t video_index, VideoRateInfo info) = 0;
    virtual void OnRequestKeyFrame(StreamIndex index, int32_t video_index) = 0;
};

/**
 * @type callback
 * @region 视频数据回调
 * @brief 本地视频帧监测器
 */
class ILocalEncodedVideoFrameObserver {
public:
    /**
     * @brief 析构函数
     */
    virtual ~ILocalEncodedVideoFrameObserver() {
    }
    /**
     * @type callback
     * @region 视频数据回调
     * @brief 调用 RegisterLocalEncodedVideoFrameObserver{@link #RegisterLocalEncodedVideoFrameObserver} 后，SDK 收到本地视频帧信息时，回调该事件
     * @param [in] type 本地视频帧类型，参看 StreamIndex{@link #StreamIndex}
     * @param [in] video_stream 本地视频帧信息，参看 IEncodedVideoFrame{@link #IEncodedVideoFrame}
     */
    virtual void OnLocalEncodedVideoFrame(StreamIndex type, const IEncodedVideoFrame& video_stream) = 0;
};

/**
 * @hidden
 * @type callback
 * @region 视频管理
 * @brief 远端音频帧监测器
 */
class IRemoteEncodedVideoFrameObserver {
public:
    /**
     * @hidden
     * @brief 析构函数
     */
    virtual ~IRemoteEncodedVideoFrameObserver() {
    }
    /**
     * @hidden
     * @type callback
     * @region 视频数据回调
     * @brief 调用 RegisterRemoteEncodedVideoFrameObserver{@link #RegisterRemoteEncodedVideoFrameObserver} 后，SDK 收到远端视频帧信息时，回调该事件
     * @param [in] stream_id 收到的远端视频流的 ID
     * @param [in] video_stream 远端视频帧信息，参看 IEncodedVideoFrame{@link #IEncodedVideoFrame}
     * @param [in] stream_info 收到的远端视频流的信息，参看 RemoteStreamKey{@link #RemoteStreamKey}
     */
    virtual void OnRemoteEncodedVideoFrame(const RemoteStreamKey& stream_info, const IEncodedVideoFrame& video_stream) = 0;
};

/**
 * @type keytype
 * @author sunhang.io
 * @brief 视频帧中包含的 metadata 信息
 */
struct VideoMetadataBuffer {
    /**
     * @brief 接收或者发送的 metadata
     */
    char* data;
    /**
     * @brief 接收或者发送的 metadata 数据大小，不能超过 1024
     */
    int size;
    /**
     * @brief 包含 metadata 视频帧的时间戳，单位微秒
     */
    int64_t timestamp_us;
};

/**
 * @type callback
 * @region 视频数据回调
 * @author sunhang.io
 * @brief metadata 观察者，可以接收媒体流中的 metadata， 或者向媒体流中添加 metadata
 */
class IMetadataObserver {
public:
    /**
     * @hidden
     */
    virtual ~IMetadataObserver() {
    }

    /**
     * @type callback
     * @region 视频数据回调
     * @author sunhang.io
     * @brief 当 SDK 准备发送一个视频帧时，会回调该事件，以确定是否需要在该视频帧中添加 metadata。
     * @param [in/out] metadata 待发送的数据，把数据拷贝到 data 字段中，并将 size 设置为真实的大小。
     * @return  <br>
     *        + true：需要向视频帧中添加 metadata  <br>
     *        + false：不需要向视频帧中添加 metadata  <br>
     * @notes  <br>
     *        + metadata 的大小不能超过1024字节  <br>
     *        + metadata 中的 timestamp_us 是输入字段，代表视频帧的时间戳，做同步使用，不需要修改。  <br>
     *        + 回调中不能有耗时操作，以免影响视频卡顿  <br>
     */
    virtual bool OnReadyToSendMetadata(VideoMetadataBuffer* metadata) = 0;

    /**
     * @type callback
     * @region 视频数据回调
     * @author sunhang.io
     * @brief 当 SDK 收到一个视频帧时，并且其中包含 medatada 时，会回调该事件。
     * @param [in] roomid 当前帧所属的房间 ID。
     * @param [in] uid 当前帧所属的用户 ID。
     * @param [in] metadata 视频帧中包含的 metadata 信息。参看 VideoMetadataBuffer{@link #VideoMetadataBuffer}。
     * @notes 回调中不能有耗时操作，以免影响视频卡顿。
     */
    virtual void OnMetadataReceived(const char* roomid, const char* uid, const VideoMetadataBuffer& metadata) = 0;
};

/**
 * @type callback
 * @region 自定义音频采集渲染
 * @author wangjunzheng
 * @brief 音频外部设备事件监听，只用于自定义音频采集渲染的情况
 *      SDK内部需要控制外部设备时，通过这个事件监听类通知上层应用
 */
class IAudioDeviceObserver {
public:
    /**
     * @type callback
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 通知上层应用开启音频采集设备
     */
    virtual void OnAudioDeviceRecordStart() = 0;
    /**
     * @type callback
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 通知上层应用关闭音频采集设备
     */
    virtual void OnAudioDeviceRecordStop() = 0;
    /**
     * @type callback
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 通知上层应用开启音频播放设备
     */
    virtual void OnAudioDevicePlayoutStart() = 0;
    /**
     * @type callback
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 通知上层应用关闭音频播放设备
     */
    virtual void OnAudioDevicePlayoutStop() = 0;

    /**
     * @hidden
     * @brief 析构函数
     */
    virtual ~IAudioDeviceObserver() {
    }
};

/**
 * @type callback
 * @region 音频数据回调
 * @author wangjunzheng
 * @brief 音频数据回调观察者
 */
class IAudioFrameObserver {
public:
    /**
     * @type keytype
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 音频帧类型
     */
    enum AUDIO_FRAME_TYPE {
        /**
         * @brief PCM 16bit
         */
        FRAME_TYPE_PCM16 = 0,
    };

    /**
     * @type keytype
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 音频帧
     */
    struct AudioFrame {
        /**
         * @brief 音频帧类型
         */
        AUDIO_FRAME_TYPE type;
        /**
         * @brief 音频采样点数量
         */
        int samples;

        /**
         * @brief 音频通道数
         */
        int channels;

        /**
         * @brief 音频采样率
         */
        int samples_per_sec;

        /**
         * @brief 音频数据内存地址
         */
        void* buffer;

        /**
         * @brief 渲染时间
         */
        int64_t render_time_ms;
    };

public:
    /**
     * @hidden
     * @brief 析构函数
     */
    virtual ~IAudioFrameObserver() {
    }

    /**
     * @type callback
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 返回麦克风录制的音频数据
     * @param [in] audio_frame 麦克风录制的音频数据, 详见：{@link #AudioFrame}
     */
    virtual void OnRecordAudioFrame(const AudioFrame& audio_frame) = 0;

    /**
     * @type callback
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 返回远端所有用户混音后的音频数据
     * @param [in] audio_frame 远端所有用户混音后的音频数据, 详见：{@link #AudioFrame}
     */
    virtual void OnPlaybackAudioFrame(const AudioFrame& audio_frame) = 0;

    /**
     * @hidden
     * @region 音频数据回调
     * @author wangjunzheng
     * @notes 该接口未实现
     */
    virtual void OnPlaybackAudioFrameBeforeMixing(const char* uid, const AudioFrame& audio_frame) = 0;

    /**
     * @type callback
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 返回本地麦克风录制和远端所有用户混音后的音频数据
     * @param [in] audio_frame 本地麦克风录制和远端所有用户混音后的音频数据, 详见：{@link #AudioFrame}
     */
    virtual void OnMixedAudioFrame(const AudioFrame& audio_frame) = 0;
};

/**
 * @type callback
 * @region 视频管理
 * @brief 视频数据回调对象
 */
class IVideoFrameObserver {
public:
    /**
     * @hidden
     * @brief 析构函数
     */
    virtual ~IVideoFrameObserver() = default;

    /**
     * @type callback
     * @region 视频管理
     * @author liyi.000
     * @brief 本地屏幕共享流数据回调
     * @param [in] videoFrame 视频数据，参看 IVideoFrame{@link #IVideoFrame}。  <br>
     * @return  <br>
     *        + true: SDK 不处理返回值
     *        + false: SDK 不处理返回值
     */
    virtual bool OnLocalScreenFrame(IVideoFrame* videoFrame) = 0;

    /**
     * @type callback
     * @region 视频管理
     * @author liyi.000
     * @brief 本地视频流数据回调
     * @param [in] videoFrame 视频数据，参看 IVideoFrame{@link #IVideoFrame}。  <br>
     * @return  <br>
     *        + true: SDK 不处理返回值
     *        + false: SDK 不处理返回值
     */
    virtual bool OnLocalVideoFrame(IVideoFrame* videoFrame) = 0;

    /**
     * @type callback
     * @region 视频管理
     * @author liyi.000
     * @brief 远端屏幕共享流数据回调
     * @param [in] roomid 房间 ID。
     * @param [in] uid 远端用户 ID。
     * @param [in] videoFrame 视频数据，参看 IVideoFrame{@link #IVideoFrame}。  <br>
     * @return  <br>
     *        + true: SDK 不处理返回值
     *        + false: SDK 不处理返回值
     */
    virtual bool OnRemoteScreenFrame(const char* roomid, const char* uid, IVideoFrame* videoFrame) = 0;

    /**
     * @type callback
     * @region 视频管理
     * @author liyi.000
     * @brief 远端视频数据回调
     * @param [in] roomid 房间 ID。
     * @param [in] uid 远端用户 ID。
     * @param [in] videoFrame 视频数据，参看 IVideoFrame{@link #IVideoFrame}}。  <br>
     * @return  <br>
     *        + true: SDK 不处理返回值
     *        + false: SDK 不处理返回值
     */
    virtual bool OnRemoteVideoFrame(const char* roomid, const char* uid, IVideoFrame* videoFrame) = 0;
};

/**
 * @type callback
 * @region 音频数据回调
 * @author wangjunzheng
 * @brief 音频数据回调观察者
 */
class IRemoteAudioFrameObserver {
public:
    /**
     * @hidden
     * @brief 析构函数
     */
    virtual ~IRemoteAudioFrameObserver() {
    }

    /**
     * @hidden
     * @type callback
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 获得单个流的音频数据，此回调通过调用 RegisterRemoteAudioFrameObserver{@link #RegisterRemoteAudioFrameObserver} 触发。
     * @param [in] audio_frame 音频数据, 详见： IAudioFrame{@link #IAudioFrame}
     * @param [in] stream_info 该音频流的业务信息, 详见： RemoteStreamKey{@link #RemoteStreamKey}
     */
    virtual void OnRemoteAudioFrame(const IAudioFrame& audio_frame, const RemoteStreamKey& stream_info) = 0;
};

/**
 * @hidden(android,ios)
 * @region 屏幕共享
 * @author zhangzhenyu.samuel
 * @brief 屏幕共享对象的类型
 */
enum ScreenCaptureSourceType {
    /**
     * @brief 类型未知
     */
    kScreenCaptureSourceTypeUnknown,

    /**
     * @brief 应用程序的窗口
     */
    kScreenCaptureSourceTypeWindow,

    /**
     * @brief 桌面
     */
    kScreenCaptureSourceTypeScreen
};

/**
 * @hidden(android,ios)
 * @region 屏幕共享
 * @author zhangzhenyu.samuel
 * @brief 屏幕源类型信息
 */
struct ScreenCaptureSourceInfo {
    /**
     * @brief 屏幕分享时共享对象的类型。
     */
    ScreenCaptureSourceType type = kScreenCaptureSourceTypeUnknown;

    /**
     * @brief 屏幕分享时共享对象的 ID。
     */
    view_t source_id = nullptr;

    /**
     * @brief 屏幕分享时共享对象的名称。
     * @notes 内存在 IScreenCaptureSourceList{@link #IScreenCaptureSourceList}释放的时候一起释放，
     * 请及时转为string对象保存
     */
    const char* source_name = nullptr;
};

/**
 * @hidden(android,ios)
 * @region 屏幕共享
 * @author zhangzhenyu.samuel
 * @brief 屏幕源类型信息列表
 */
class IScreenCaptureSourceList {
public:
    /**
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 获取列表大小
     * @return 获取列表大小
     */
    virtual int32_t GetCount() = 0;

    /**
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 根据索引，获取屏幕共享列表中的元素
     * @param 列表索引号
     * @return 屏幕源类型信息
     * @notes 返回类型中有char*类型的字符串，该字符串在本对象Release后释放，请注意内存管理。
     */
    virtual ScreenCaptureSourceInfo GetSourceInfo(int32_t index) = 0;

    /**
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 析构当前对象，释放内存
     * @notes 严禁调用 delete 该结构体，该结构不需要的时候应该调用本函数释放资源
     */
    virtual void Release() = 0;
};

/**
 * @type api
 * @brief 引擎 API
 */
class IRtcEngineLite {
public:
    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 调节音频采集音量
     * @param [in] volume 音频采集音量，取值范围： [0,400]  <br>
     *       + 0: 静音  <br>
     *       + 100: 原始音量  <br>
     *       + 400: 最大可为原始音量的 4 倍(自带溢出保护)  <br>
     * @notes 为保证更好的通话质量，建议将 volume 值设为 [0,100]。
     */
    virtual void SetRecordingVolume(const int volume) = 0;


    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 调节本地播放的所有远端用户混音后的音量
     * @param [in] volume 音频播放音量，取值范围： [0,400] <br>
     *       + 0: 静音  <br>
     *       + 100: 原始音量  <br>
     *       + 400: 最大可为原始音量的 4 倍(自带溢出保护)  <br>
     * @notes 为保证更好的通话质量，建议将 volume 值设为 [0,100]。
     */
    virtual void SetPlaybackVolume(const int volume) = 0;


    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 开启声卡采集，将声卡播放的音频流合到本地采集的音频流中。<br>
     *         如果你希望将声卡播放的声音传输到远端（比如屏幕共享等场景），你必须开启声卡采集。如此，声卡播放的声音会合到本地采集的音频流在中，一起发送到远端。
     * @notes  <br>
     *       + 开启声卡采集后，你可以设置对应音量，参看 SetScreenAudioVolumeBeforeMixing{@link #SetScreenAudioVolumeBeforeMixing} <br>
     *       + 如果需要关闭声卡采集，参看 StopMixScreenAudioToMainStream{@link #StopMixScreenAudioToMainStream}
     */
    virtual void StartMixScreenAudioToMainStream() = 0;

    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 关闭声卡采集，声卡播放的音频流不再合到本地采集的音频流中。
     * @notes 如果需要开启声卡采集，请参看 StartMixScreenAudioToMainStream{@link #StartMixScreenAudioToMainStream}
     */
    virtual void StopMixScreenAudioToMainStream() = 0;


    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 调节本地声卡采集音量。<br>
     *        开启本地声卡采集后，你可以使用此接口调节采集音量。
     *        关于如何开启本地声卡采集，参考 StartMixScreenAudioToMainStream{@link #StartMixScreenAudioToMainStream}。
     * @param volume 声卡采集音量，取值范围： [0,400]  <br>
     *       + 0: 静音  <br>
     *       + 100: 原始音量  <br>
     *       + 400: 最大可为原始音量的 4 倍(自带溢出保护)  <br>
     */
    virtual void SetScreenAudioVolumeBeforeMixing(int volume) = 0;


    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 开启内部音频采集。默认为关闭状态。  <br>
     *        进房前调用该方法，本地用户会收到 OnMediaDeviceStateChanged{@link #OnMediaDeviceStateChanged} 的回调。  <br>
     *        进房后调用该方法，房间中的其他用户会收到 OnUserStartAudioCapture{@link #OnUserStartAudioCapture} 的回调。
     * @notes  <br>
     *       + 调用 StopAudioCapture{@link #StopAudioCapture} 可以关闭音频采集设备，否则，SDK 只会在销毁引擎的时候自动关闭设备。  <br>
     *       + 无论是否发布音频数据，你都可以调用该方法开启音频采集，并且调用后方可发布音频。  <br>
     *       + 尚未进房并且已使用自定义采集时，关闭自定义采集后并不会自动开启内部采集。你需调用此方法手动开启内部采集。
     */
    virtual void StartAudioCapture() = 0;

    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 关闭内部音频采集。默认为关闭状态。  <br>
     *        进房前调用该方法，本地用户会收到 OnMediaDeviceStateChanged{@link #OnMediaDeviceStateChanged} 的回调。  <br>     
     *        进房后调用该方法后，房间中的其他用户会收到 OnUserStopAudioCapture{@link #OnUserStopAudioCapture} 的回调。
     * @notes  <br>
     *       + 调用 StartAudioCapture{@link #StartAudioCapture} 可以开启音频采集设备。  <br>
     *       + 设备开启后若一直未调用该方法关闭，则 SDK 会在销毁引擎的时候自动关闭音频采集设备。  <br>
     */
    virtual void StopAudioCapture() = 0;

    /**
     * @hidden(macOS,Windows)
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 设置音频场景类型。  <br>
     *        你可以根据你的应用所在场景，选择合适的音频场景类型。
     *        选择音频场景后，RTC 会自动根据客户端音频路由和发布订阅状态，适用通话音量/媒体音量。  <br>
     *        在进房前和进房后设置均可生效。
     * @param [in] scenario 音频场景类型，
     *        参见 AudioScenarioType{@link #AudioScenarioType}
     * @notes  <br>
     *        + 通话音量更适合通话，会议等对信息准确度要求更高的场景。通话音量会激活系统硬件信号处理，使通话声音会更清晰。此时，音量无法降低到 0。<br>
     *        + 媒体音量更适合娱乐场景，因其声音的表现力会更强。媒体音量下，音量最低可以降低到 0。
     */
    virtual void SetAudioScenario(AudioScenarioType scenario) = 0;

    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 控制本地音频流的发送状态：发送/不发送  <br>
     *        使用此方法后，房间中的其他用户会收到回调： OnUserMuteAudio{@link #OnUserMuteAudio}
     * @param [in] mute_state 发送状态，标识是否发送本地音频流，详见：MuteState{@link #MuteState}
     * @notes 本方法仅控制本地音频流的发送状态，并不影响本地音频采集状态。
     */
    virtual void MuteLocalAudio(MuteState mute_state) = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 启用自定义音频采集渲染
     * @param [in] recording_format 自定义音频数据采集格式，详见 AudioFormat{@link #AudioFormat}
     * @param [in] playback_format 自定义音频数据渲染格式，详见 AudioFormat{@link #AudioFormat}
     * @notes  <br>
     *      + 该方法需要在进房前调用。  <br>
     *      + 启用自定义音频采集渲染的状态在离开房间后仍然有效，会一直持续到调用 DisableExternalAudioDevice{@link #DisableExternalAudioDevice} 关闭自定义音频采集渲染为止。  <br>
     *      + 启用自定义音频采集渲染后，仍需要使用 PushExternalAudioFrame{@link #PushExternalAudioFrame}，推送外部音频数据，再使用 PullExternalAudioFrame{@link #PullExternalAudioFrame} 拉取外部音频数据。  <br>
     *      + 当你已调用 StartAudioCapture{@link #StartAudioCapture} 开启内部采集后，再调用此方法切换至自定义采集时，SDK 会自动关闭内部采集。  <br>
     *      + 当你调用此方法开启自定义采集后，想要切换至内部采集，你必须先调用 DisableExternalAudioDevice{@link #DisableExternalAudioDevice} 关闭自定义采集，然后调用 StartAudioCapture{@link #StartAudioCapture} 手动开启内部采集。
     */   
    virtual void EnableExternalAudioDevice(const AudioFormat &recording_format, const AudioFormat &playback_format) = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 禁用已开启的自定义音频采集渲染，将音频采集渲染由自定义模块切换至内部模块。
     * @notes  <br>
     *      + 如果你已开启自定义音频采集渲染，你可以在进房前，使用本接口将音频采集渲染由自定义模块切换至内部模块。  <br>
     *      + 使用该 API 禁用自定义音频采集渲染后， SDK 不会自动开启内部的音频采集，需要开启 SDK 内部采集请使用 StartAudioCapture{@link #StartAudioCapture}。  <br>
     *      + 启用自定义音频采集渲染请使用 EnableExternalAudioDevice{@link #EnableExternalAudioDevice}。  <br>
     */
    virtual void DisableExternalAudioDevice() = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 启动音频外部源采集渲染时，添加音频外部设备事件监控
     * @param [in] obs
     *        音频外部设备事件监控，详见：IAudioDeviceObserver{@link #IAudioDeviceObserver}
     */
    virtual void SetAudioDeviceObserver(IAudioDeviceObserver* obs) = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 推送外部音频数据。使用 EnableExternalAudioDevice{@link #EnableExternalAudioDevice}
     * 启用自定义音频采集渲染后，可以使用本方法推送外部音频数据。
     * @param [in] data
     *        pcm 数据，内存大小应该为 samples * record_format.channel * 2。
     * @param [in] samples
     *        采样点数量，应该为 EnableExternalAudioDevice{@link #EnableExternalAudioDevice} 中设置的 record_format.sample_rate / 100。
     *        当设置采样率为48000时， 每次应该推送480个采样点
     * @return  方法调用结果  <br>
     *        + 0：方法调用成功  <br>
     *        + <0：方法调用失败  <br>
     * @notes  <br>
     *       + 必须是 pcm 数据，推送间隔是 10ms，音频采样格式为 s16。  <br>
     *       + 该函数运行在用户调用线程内，是一个同步函数  <br>
     */
    virtual bool PushExternalAudioFrame(int8_t* data, int samples) = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @author wangjunzheng
     * @brief 拉取远端音频数据。使用 EnableExternalAudioDevice{@link #EnableExternalAudioDevice}
     * 启用自定义音频采集渲染后，可以使用本方法拉取远端音频数据。
     * @param [out] data
     *        pcm 数据，内存大小应该为 samples * playback_format.channel * 2。
     * @param [in] samples
     *        采样点数量，应该为 EnableExternalAudioDevice{@link #EnableExternalAudioDevice} 中设置的 playback_format.sample_rate / 100。
     *        当设置采样率为 48000 时， 每次应该拉取 480 个采样点
     * @return  方法调用结果  <br>
     *        + true:方法调用成功  <br>
     *        + false：方法调用失败  <br>
     * @notes  <br>
     *       + 必须是 pcm 数据，拉取间隔是 10ms，音频采样格式为 s16。  <br>
     *       + 该函数运行在用户调用线程内，是一个同步函数  <br>
     */
    virtual bool PullExternalAudioFrame(int8_t* data, int samples) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 开始播放音乐文件及混音。
     * @param [in] file_path  <br>
     *        指定需要混音的本地文件的绝对路径。支持音频文件格式有: mp3，aac，m4a，3gp，wav。
     * @param [in] loopback  <br>
     *       + true: 只有本地可以听到混音或替换后的音频流。  <br>
     *       + false: 本地和对方都可以听到混音或替换后的音频流
     * @param [in] replace  <br>
     *       + true: 只推送设置的本地音频文件，不传输麦克风采集的音频。  <br>
     *       + false: 音频文件内容将会和麦克风采集的音频流进行混音
     * @param [in] cycle 指定音频文件循环播放的次数。  <br>
     *       + >0: 循环的次数。  <br>
     *       + -1: 无限循环
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 该方法指定本地和麦克风采集的音频流进行混音或替换。替换是指用音频文件替换麦克风采集的音频流。  <br>
     *      + 该方法可以选择是否让对方听到本地播放的音频，并指定循环播放的次数。
     * 调用本方法播放音乐文件及混音结束后，应用会收到 OnAudioMixingFinished{@link #OnAudioMixingFinished} 回调。  <br>
     *      + 开始播放音乐文件及混音后，可以调用 StopAudioMixing{@link #StopAudioMixing} 方法停止播放音乐文件。  <br>
     *      + 该方法混音数据来源外部文件，且支持的文件格式有: mp3，aac，m4a，3gp，wav；而SetAudioPlayoutMixStream{@link
     * #SetAudioPlayoutMixStream} 混音的数据来源外部缓存且音频格式为PCM；这两种混音方式可以共存。  <br>
     *      + 单个房间只支持一路音乐文件的播放，多次调用该函数后，只有最后一次调用会生效。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int StartAudioMixing(const char* file_path, bool loopback, bool replace, int cycle) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 停止播放音乐文件及混音
     * @notes  <br>
     *      + 调用 StartAudioMixing{@link #StartAudioMixing}
     * 方法开始播放音乐文件及混音后，调用该方法可以停止播放音乐文件及混音。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual void StopAudioMixing() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 暂停播放音乐文件
     * @return  <br>
     *        + 0: 成功  <br>
     *        + <0: 失败
     * @notes  <br>
     *      + 调用 StartAudioMixing{@link #StartAudioMixing}
     * 方法开始播放音乐文件及混音后，可以通过调用该方法暂停播放音乐文件。  <br>
     *      + 调用该方法暂停播放音乐文件后，可调用 ResumeAudioMixing{@link #ResumeAudioMixing} 方法恢复播放。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int PauseAudioMixing() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 恢复播放音乐文件
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 调用 PauseAudioMixing{@link #PauseAudioMixing} 方法暂停播放音乐文件后，可以通过调用该方法恢复播放。  <br>
     *      + 请在房间内调用该方法
     */
    virtual int ResumeAudioMixing() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 调节音乐文件的文件音量。为保证更好的音质，建议将 volume 值设为 [0,100]。
     * @param [in] volume 音乐文件播放音量范围为 0~400。  <br>
     *       + 0：静音  <br>
     *       + 100：原始音量  <br>
     *       + 400: 最大可调音量 (自带溢出保护)
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 调用该方法可同时调节的是本地和远端播放音量。仅调节本端音量可使用 AdjustAudioMixingPlayoutVolume{@link
     * #AdjustAudioMixingPlayoutVolume} ， 仅调节远端音量可使用 AdjustAudioMixingPublishVolume{@link
     * #AdjustAudioMixingPublishVolume}。  <br>
     *      + 该方法对 AdjustAudioMixingPlayoutVolume{@link #AdjustAudioMixingPlayoutVolume} 和
     * AdjustAudioMixingPublishVolume{@link #AdjustAudioMixingPublishVolume} 的音量调节影响是乘积关系。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int AdjustAudioMixingVolume(int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 调节音乐文件的本地播放音量。为保证更好的音质，建议将 volume 值设为 [0,100]。
     * @param [in] volume 音乐文件播放音量范围为 0~400。  <br>
     *       + 0：静音  <br>
     *       + 100：原始音量  <br>
     *       + 400: 最大可调音量 (自带溢出保护)
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 调用该方法可以调节混音的音乐文件在本地播放的音量大小。如果需要同时调节本地和远端播放音量可使用
     * AdjustAudioMixingVolume{@link #AdjustAudioMixingVolume} 方法。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int AdjustAudioMixingPlayoutVolume(int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 调节音乐文件的远端播放音量。为保证更好的音质，建议将 volume 值设为 [0,100]。
     * @param [in] volume 音乐文件播放音量范围为 0~400。  <br>
     *       + 0：静音  <br>
     *       + 100：原始音量  <br>
     *       + 400: 最大可调音量 (自带溢出保护)
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 该方法调节混音的音乐文件在远端播放的音量大小。如果需要同时调节本地和远端播放音量可使用
     * AdjustAudioMixingVolume{@link #AdjustAudioMixingVolume} 方法。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int AdjustAudioMixingPublishVolume(int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 获取音乐文件时长
     * @return  <br>
     *       + >0: 成功, 音乐文件时长，单位为毫秒。  <br>
     *       + <0: 失败
     * @notes <br>
     *       请在房间内调用该方法。
     */
    virtual int GetAudioMixingDuration() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 获取音乐文件播放进度
     * @return  <br>
     *       + >0: 成功, 音乐文件播放进度，单位为毫秒。  <br>
     *       + <0: 失败
     * @notes <br>
     *       请在房间内调用该方法。
     */
    virtual int GetAudioMixingCurrentPosition() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 设置音频文件的播放位置
     * @param [in] pos
     *        整数。进度条位置，单位为毫秒。
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *       该方法可以设置音频文件的播放位置，这样你可以根据实际情况从指定的位置播放音乐文件，无需从头到尾完整播放一个音乐文件。
     */
    virtual int SetAudioMixingPosition(int pos) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 获取指定音效文件的文件音量
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。确保此处的 sound_id 与  PlayEffect{@link #PlayEffect} 设置的
     * sound_id 相同。
     * @return  <br>
     *       + >0: 成功，文件音量，音量范围为 0~400。  <br>
     *       + <0: 失败
     * @notes  <br>
     *       请在房间内调用该方法。
     */
    virtual int GetEffectVolume(int sound_id) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 调节指定音效文件的文件音量
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。确保此处的 sound_id 与  PlayEffect{@link #PlayEffect} 设置的
     * sound_id 相同。
     * @param [in] volume  <br>
     *        音乐文件播放音量范围为 0~400。默认 100 为原始文件音量
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes
     *       + 该方法调节音效文件在本地和远端播放的音量大小。  <br>
     *       + 请在房间内调用该方法。
     */
    virtual int SetVolumeOfEffect(int sound_id, int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 开始播放指定音效文件。你可以在该方法中设置音效文件的播放次数、音量大小，以及远端用户是否能听到该音效。
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。如果已经通过 PreloadEffect{@link #PreloadEffect}
     * 将音效加载至内存，确保此处的 sound_id 与 PreloadEffect{@link #PreloadEffect} 设置的 sound_id 相同。
     * @param [in] file_path  <br>
     *        指定需要混音的音频文件名和文件路径名。支持以下音频格式: mp3，aac，m4a，3gp，wav
     * @param [in] loopback  <br>
     *       + true: 只有本地可以听到该音效。  <br>
     *       + false: 本地和远端用户都可以听到该音效。
     * @param  cycle 指定音频文件循环播放的次数。  <br>
     *       + >0：循环的次数。  <br>
     *       + -1：无限循环，直至调用 StopEffect{@link #StopEffect} 或 StopAllEffects{@link #StopAllEffects} 后停止。
     * @param [in] volume  <br>
     *        音效文件播放音量范围为 0~400。默认 100 为原始文件音量。
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 调用该方法播放音效结束后，应用会收到 OnAudioEffectFinished{@link #OnAudioEffectFinished} 回调。  <br>
     *      + 可以多次调用该方法，通过传入不同的音效文件的 sound_id 和
     * file_path，以实现同时播放多个音效文件，实现音效叠加。  <br>
     *      + 可以调用 StopEffect{@link #StopEffect} 方法停止播放指定音效文件。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual int PlayEffect(int sound_id, const char* file_path, bool loopback, int cycle, int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 预加载指定音效文件。
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。
     * @param [in] file_path  <br>
     *        指定需要混音的音频文件名和文件路径名。支持以下音频格式: mp3，aac，m4a，3gp，wav。
     * @return  <br>
     *        + 0: 成功  <br>
     *        + <0: 失败
     * @notes  <br>
     *       +
     * 调用该方法预加载播放指定音效文件。预加载操作可以在进房间之前完成（所有混音相关接口，如果没有标注请在房间内调用都可在没有进入房间的时候调用）。如果音效文件很长，加载操作可能会耗时较长，建议应用开发者将预加载操作放在子线程进行。
     * <br>
     *       + 该方法只是预加载播放指定音效文件，只有调用 PlayEffect{@link #PlayEffect} 方法才开始播放指定音效文件。
     * <br>
     *       + 该方法预加载指定音效文件可以通过 UnloadEffect{@link #UnloadEffect} 来卸载。
     */
    virtual int PreloadEffect(int sound_id, const char* file_path) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 卸载指定音效文件
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *       + 调用该方法卸载指定音效文件。  <br>
     *       + 如果调用 StopEffect{@link #StopEffect} 方法时，音效文件没有被卸载，SDK会自动调用该方法卸载音效文件。
     */
    virtual int UnloadEffect(int sound_id) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 停止播放指定音效文件
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。如果已经通过 PreloadEffect{@link #PreloadEffect}
     * 将音效加载至内存，确保此处的 sound_id 与  PreloadEffect{@link #PreloadEffect} 设置的 sound_id 相同。
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *       + 调用该方法停止播放指定音效文件。  <br>
     *       + 该方法内部会主动调用 UnloadEffect{@link #UnloadEffect} 来卸载指定音效文。
     */
    virtual int StopEffect(int sound_id) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 暂停播放指定音效文件
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *       调用该方法暂停播放指定音效文件，可以调用 ResumeEffect{@link #ResumeEffect} 方法恢复播放。
     */
    virtual int PauseEffect(int sound_id) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 恢复播放指定音效文件
     * @param [in] sound_id  <br>
     *        音效ID，应用调用者维护，请保证唯一性。
     * @return  <br>
     *       + 0: 成功  <br>
     *       + <0: 失败
     * @notes  <br>
     *      + 调用 PauseEffect{@link #PauseEffect} 方法暂停播放指定音效文件后， 可以通过该方法恢复播放。  <br>
     *      + 调用 PauseAllEffects{@link #PauseAllEffects}
     * 方法暂停所有音效文件的播放后，也可以通过该方法恢复单个音效文件播放。
     */
    virtual int ResumeEffect(int sound_id) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 调节所有音效文件的文件音量
     * @param [in] volume  <br>
     *        音乐文件播放音量范围为 0~400。默认 100 为原始文件音量。
     * @notes  <br>
     *      + 该方法调节混音的所有音效文件在本地和远端播放的音量大小。  <br>
     *      + 请在房间内调用该方法。
     */
    virtual void SetEffectsVolume(int volume) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 停止所有音效文件的播放
     * @notes  <br>
     *      该方法停止所有音效文件的播放。
     */
    virtual void StopAllEffects() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 暂停所有音效文件的播放。
     * @notes  <br>
     *      调用该方法暂停所有音效文件的播放，可调用 ResumeAllEffects{@link #ResumeAllEffects} 方法恢复播放。
     */
    virtual void PauseAllEffects() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 恢复所有音效文件的播放
     * @notes  <br>
     *     + 调用该方法恢复所有音效文件的播放。在调用 PauseAllEffects{@link #PauseAllEffects}
     * 暂停所有音效文件的播放后，可以调用该方法可恢复所有音效文件的播放。  <br>
     *     + 调用 PauseEffect{@link #PauseEffect}
     * 方法暂停单个指定音效文件的播放后，也可以调用该方法恢复播放，但是效率太低，不推荐这么使用。
     */
    virtual void ResumeAllEffects() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 卸载所有音效文件
     * @notes  <br>
     *      + 调用该方法卸载所有音效文件。如果引擎被销毁时有音效文件没有被卸载，SDK会自动调用该方法卸载所有音效文件。 <br>
     *      + 调用 LeaveRoom{@link #LeaveRoom} 离开房间后，不会自动调用该方法卸载所有音效文件。如果需要在离开房间后卸载所有音效文件，需要应用主动调用该方法。
     */
    virtual void UnloadAllEffects() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 启动/停止外部音频流混音， 并设置混音数据格式
     * @param [in] enable  <br>
     *       + true: 启用外部音频流混音。  <br>
     *       + false: 停止外部音频流混音。
     * @param [in] sample_rate  <br>
     *        音频采样率，单位HZ。目前支持的采样率有：8000HZ， 16000HZ， 32000HZ， 44100HZ，48000HZ。
     * @param [in] channel_num  <br>
     *        音频声道个数。目前支持单通道(1)，双通道(2)
     * @notes  <br>
     *      + 调用该方法设置外部音频混音的PCM格式，即 PushAudioMixingStreamData{@link #PushAudioMixingStreamData} 的
     * audio_frame 的音频数据格式。  <br>
     *      + 调用该方法混音的数据来源外部缓存，且音频格式为PCM； StartAudioMixing{@link #StartAudioMixing}
     * 方法混音数据来源外部文件，且支持的文件格式有: mp3，aac，m4a，3gp，wav。这两种混音方式可以共存。  <br>
     *      + enable 为 false 时，停止外部音频流混音。 或者引擎释放时，SDK内部会停止混音。  <br>
     *      + 该方法启动的外部音频流混音不支持音量调节，暂停，暂停恢复操作。
     */
    virtual void SetAudioPlayoutMixStream(bool enable, int sample_rate, int channel_num) = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 获取SDK当前缓冲数据
     * @return  <br>
     *       + >0: 成功，缓冲的音频采样点个数。  <br>
     *       + 0: 失败
     * @notes
     *      调用该方法可以实时获取缓存数据量，并以此数据为基准调整数据推送节奏来避免内存溢出
     */
    virtual int GetAudioMixingStreamCachedFrameNum() = 0;

    /**
     * @type api
     * @region 混音
     * @author wangjunzheng
     * @brief 向SDK推送混音的音频数据
     * @param [in] audio_frame  <br>
     *        PCM音频数据，其格式与 SetAudioPlayoutMixStream{@link #SetAudioPlayoutMixStream} 保持一致。
     * @param [in] frame_num  <br>
     *        采样点数量，应该为 SetAudioPlayoutMixStream{@link #SetAudioPlayoutMixStream} 中设置的 sample_rate / 100。
     *        当设置采样率为48000时， 每次应该推送480个采样点
     * @return
     *       + true: 成功  <br>
     *       + false: 失败
     * @notes
     *      + 调用该方法前，必须通过 SetAudioPlayoutMixStream{@link #SetAudioPlayoutMixStream} 方法设置 audio_frame
     * 的数据格式。  <br>
     *      + 调用该方法前，先通过 GetAudioMixingStreamCachedFrameNum{@link #GetAudioMixingStreamCachedFrameNum}
     * 获取缓存的数据量，并调整推送数据的节奏以避免内部缓存溢出导致推送失败。  <br>
     *      + 使用参考建议：首次推送数据，请在应用侧先缓存一定数据（如 200 毫秒），
     * 然后一次性推送过去；此后的推送操作定时 10 毫秒一次，并且每次的音频数据量为 10 毫秒数据量。
     */
    virtual bool PushAudioMixingStreamData(int8_t* audio_frame, int frame_num) = 0;

#ifndef ByteRTC_AUDIO_ONLY

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 开启内部视频采集。默认为关闭状态。  <br>
     *        进房前调用该方法，本地用户会收到 OnMediaDeviceStateChanged{@link #OnMediaDeviceStateChanged} 的回调。  <br>     
     *        进房后调用该方法后，房间中的其他用户会收到 OnUserStartVideoCapture{@link #OnUserStartVideoCapture} 的回调。
     * @notes  <br>
     *       + 调用 StopVideoCapture{@link #StopVideoCapture} 可以关闭视频采集设备，否则，SDK 只会在销毁引擎的时候自动关闭设备。  <br>
     *       + 无论是否发布视频数据，你都可以调用该方法开启视频采集，并且调用后方可发布视频。  <br>
     *       + 已使用自定义采集时，关闭自定义采集后并不会自动开启内部采集。你需调用此方法手动开启内部采集。     
     */
    virtual void StartVideoCapture() = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 关闭内部视频采集。默认为关闭状态。  <br>
     *        进房前调用该方法，本地用户会收到 OnMediaDeviceStateChanged{@link #OnMediaDeviceStateChanged} 的回调。  <br>     
     *        进房后调用该方法后，房间中的其他用户会收到 OnUserStopVideoCapture{@link #OnUserStopVideoCapture} 的回调。
     * @notes  <br> 
     *       + 调用 StartVideoCapture{@link #StartVideoCapture} 可以开启视频采集设备。  <br>
     *       + 设备开启后若一直未调用该方法关闭，则 SDK 会在销毁引擎的时候自动关闭音频采集设备。  <br>
     */
    virtual void StopVideoCapture() = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 启动推送多路视频流，设置推送多路流时的各路视频参数，
     *        包括分辨率、帧率、码率、缩放模式、网络不佳时的回退策略等。
     * @param [in] solutions 视频参数数组首地址。参看 VideoSolution{@link #VideoSolution}。
     * @param [in] solution_num 视频参数数组长度。
     * @return  <br>
     *        + 0：成功  <br>
     *        + !0：失败  <br>
     * @notes  <br>
     *       + 视频参数数组中，分辨率必须是依次减小，从大到小排列的。  <br>
     *       + 最大分辨率没有限制。但是如果设置的分辨率无法编码，就会导致编码推流失败。  <br>
     *       + 数组元素不超过3个。  <br>
     */
    virtual int SetVideoEncoderConfig(const VideoSolution* solutions, int solution_num) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author sunhang.io
     * @brief 设置本地视频渲染时，使用的视图，并设置渲染模式。 <br>
     *        你应在加入房间前，绑定本地视图。退出房间后，此设置仍然有效。
     *        如果需要解除绑定，你可以调用本方法绑定空视图。
     * @param [in] index 视频流属性, 参看 StreamIndex{@link #StreamIndex}
     * @param [in] canvas 视图信息和渲染模式，参看：VideoCanvas{@link #VideoCanvas}
     * @return  <br>
     *        + 0：成功  <br>
     *        + !0：失败  <br>
     */
    virtual int SetLocalVideoCanvas(StreamIndex index, const VideoCanvas& canvas) = 0;

    /**
     * @type api
     * @region 自定义视频采集渲染
     * @author sunhang.io
     * @brief 设置本地视频渲染器
     *   该方法设置本地视频外部渲染器。应用程序通过调用此接口绑定本地视频流的渲染器canvas。
     *   在应用程序开发中，通常在初始化后调用该方法进行本地视频设置，然后再加入频道。退出频道后，绑定仍然有效，
     *   如果需要解除绑定，可以指定空的VideoSink。
     * @param [in] sink
     *        外部渲染器，详见：{@link #IVideoSink}
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + !0: 失败；  <br>
     */
    virtual int SetLocalVideoRender(IVideoSink* sink) = 0;

    /**
     * @type api
     * @region 自定义视频采集渲染
     * @author sunhang.io
     * @brief 设置屏幕共享本地渲染器
     *   该方法设置屏幕共享本地外部渲染器。应用通过调用此接口绑定本地屏幕共享流的渲染器 IVideoSink{@link #IVideoSink} 。
     *   在应用开发中，通常在初始化后调用该方法进行本地屏幕共享设置，然后再加入房间。退出房间后，绑定仍然有效，
     *   如果需要解除绑定，可以指定空的 VideoSink{@link #IVideoSink}。
     * @param [in] sink
     *        外部渲染器，详见：IVideoSink{@link #IVideoSink}
     * @return
     *        0 :成功；
     *       -1 :失败；
     */
    virtual int SetLocalScreenRender(IVideoSink* sink) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 停止/启动发送本地视频流，默认不发送。<br>
     *        无论你使用 SDK 的视频采集编码功能，还是使用自定义采集功能，你都应使用此接口启动发送本地视频流。
     *        调用该方法后，房间中的其他用户会收到 OnUserMuteVideo{@link #OnUserMuteVideo} 的回调。
     * @param  [in] muteState 发送状态，标识是否发送本地音/视频流，参看 MuteState{@link #MuteState}   <br>
     *       + true：停止发送  <br>
     *       + false：开启发送  <br>
     * @return  <br>
     *        + 0：成功  <br>
     *        + !0：失败  <br>
     * @notes 本方法只是停止本地视频流的发送，不影响视频采集状态。
     */
    virtual int MuteLocalVideo(MuteState muteState) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 切换移动端前置/后置摄像头
     * @param  [in] Camera ID，移动端摄像头。参看 CameraID {@link #CameraID}。
     * @return  <br>
     *        + 0：成功  <br>
     *        + !0：失败  <br>
     * @notes  <br>
     *       + 移动平台可用，PC 平台不可用。  <br>
     *       + 如果本地有多个摄像头且想选择特定工作摄像头可通过 IVideoDeviceManager{@link #IVideoDeviceManager} 来控制。  <br>
     */
    virtual int SwitchCamera(CameraID camera_id) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author zhangzhenyu.samuel
     * @brief 发送屏幕共享本地视频数据
     * @param [in] frame 设置屏幕视频帧，详见：IVideoFrame{@link #IVideoFrame}。  <br>
     * @return  <br>
     *        + true: 成功；  <br>
     *        + false: 失败；  <br>
     * @notes  <br>
     *       + 暂时只支持 YUV420P 格式的视频帧。  <br>
     *       + 该函数运行在用户调用线程内，即将销毁 RtcEngine 实例前，请停止调用该函数推送屏幕共享数据  <br>
     */
    virtual bool PushScreenFrame(IVideoFrame* frame) = 0;

    /**
      * @hidden(iOS,Android)
      * @type api
      * @region 屏幕共享
      * @author zhangzhenyu.samuel
      * @brief 该方法共享一个窗口或该窗口的部分区域。用户需要在该方法中指定想要共享的窗口 id 。
      * @param [in] window_id
      *        指定待共享的窗口 id，详见：view_t{@link #view_t}
      * @param [in] region_rect
      *        指定共享区域相对于整个窗口的位置，当值都为0时共享整个窗口，详见：Rectangle{@link #Rectangle}
      * @param [in] capture_params
      *        屏幕共享的编码参数配置，详见：DesktopCaptureParameters{@link #DesktopCaptureParameters}
      * @return  <br>
      *        + 0: 成功；  <br>
      *        + -1: 失败；  <br>
      * @notes  <br>
      *       + 本函数和 PublishScreen{@link #IRtcRoom#PublishScreen} 互斥使用，当调用 StopScreenCapture{@link
      * #StopScreenCapture} 后方可再用 PublishScreen{@link #IRtcRoom#PublishScreen}。  <br>
      *       + 本函数和 StopScreenCapture{@link #StopScreenCapture} 是成对调用的。  <br>
      *       + 在收到 OnFirstRemoteVideoFrameRendered{@link #IRTCRoomEventHandler#OnFirstRemoteVideoFrameRendered} 事件后通过调用
      * SetRemoteVideoCanvas{@link #SetRemoteVideoCanvas} 或 SetRemoteScreenRender{@link #SetRemoteScreenRender}
      * 函数来设置远端屏幕共享视图。  <br>
      *       + 本地可调用 SetLocalVideoCanvas{@link #IRtcEngineLite#SetLocalVideoCanvas} 或 SetLocalScreenRender{@link
      * #IRtcEngineLite#SetLocalScreenRender} 函数设置本地屏幕共享视图。  <br>
      *       + 也可通过注册 RegisterVideoFrameObserver{@link #RegisterVideoFrameObserver} 视频回调对象，监听
      * OnLocalScreenFrame{@link #IVideoFrameObserver#OnLocalScreenFrame} 本地屏幕视频回调事件和
      * OnRemoteScreenFrame{@link #IVideoFrameObserver#OnRemoteScreenFrame} 远端屏幕共享视频回调事件来获取原始数据。 <br>
      */
    virtual int StartScreenCaptureByWindowId(
            view_t window_id, const Rectangle& region_rect, const DesktopCaptureParameters& capture_params) = 0;

    /**
     * @hidden(macOS,iOS,Android)
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 通过指定区域共享屏幕，共享一个屏幕或该屏幕的部分区域。用户需要在该方法中指定想要共享的屏幕区域。
     * @param [in] screen_rect
     *        指定待共享的屏幕相对于虚拟屏的位置，虚拟屏指所有屏幕组成的图像区域，详见：Rectangle{@link #Rectangle}
     * @param [in] region_rect
     *        指定待共享区域相对于screen_rect的位置，详见：Rectangle{@link #Rectangle}
     * @param [in] capture_params
     *        屏幕共享的编码参数配置，详见：DesktopCaptureParameters{@link #DesktopCaptureParameters}
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + -1: 失败；  <br>
     * @notes  <br>
     *       + 本函数和 PublishScreen{@link #IRtcRoom#PublishScreen} 互斥使用，当调用 StopScreenCapture{@link
     * #StopScreenCapture} 后方可再用 PublishScreen{@link #IRtcRoom#PublishScreen}。  <br>
     *       + 本函数和 StopScreenCapture{@link #StopScreenCapture} 是成对调用的  <br>
     *       + 在收到 OnFirstRemoteVideoFrameRendered{@link #IRTCRoomEventHandler#OnFirstRemoteVideoFrameRendered} 事件后通过调用
     * SetRemoteVideoCanvas{@link #SetRemoteVideoCanvas} 或 SetRemoteScreenRender{@link #SetRemoteScreenRender}
     * 函数来设置远端屏幕共享视图。  <br>
     *       + 本地可调用 SetLocalVideoCanvas{@link #IRtcEngineLite#SetLocalVideoCanvas} 或 SetLocalScreenRender{@link
     * #IRtcEngineLite#SetLocalScreenRender} 函数设置本地屏幕共享视图。  <br>
     *       + 也可通过注册 RegisterVideoFrameObserver{@link #RegisterVideoFrameObserver} 视频回调对象，监听
     * OnLocalScreenFrame{@link #IVideoFrameObserver#OnLocalScreenFrame} 本地屏幕视频回调事件和
     * OnRemoteScreenFrame{@link #IVideoFrameObserver#OnRemoteScreenFrame} 远端屏幕共享视频回调事件来获取原始数据。 <br>
     */
    virtual int StartScreenCaptureByScreenRect(const Rectangle& screen_rect, const Rectangle& region_rect,
            const DesktopCaptureParameters& capture_params) = 0;

    /**
     * @hidden(iOS,Android)
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 通过屏幕 id 共享屏幕，共享一个屏幕或该屏幕的部分区域。用户需要在该方法中指定想要共享的屏幕 id 。
     * @param [in] display_id
     *        指定待共享的屏幕 id 。
     * @param [in] region_rect
     *        指定待共享区域相对于整个屏幕的位置，详见：Rectangle{@link #Rectangle}
     * @param [in] capture_params
     *        屏幕共享的编码参数配置，详见：DesktopCaptureParameters{@link #DesktopCaptureParameters}
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + -1: 失败；  <br>
     * @notes 共享一个窗口或该窗口的部分区域。需要在该方法中指定想要共享的窗口 id 。
     */
    virtual int StartScreenCaptureByDisplayId(
            unsigned int display_id, const Rectangle& region_rect, const DesktopCaptureParameters& capture_params) = 0;

     /**
     * @hidden(macOS,Windows)
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 通过传入的Context开启屏幕共享。
     * @param [in] context
     *        Android平台是MediaProjection对象，由业务方申请系统录屏权限后获得, iOS上暂未用到，传空即可。
     * @param [in] group_id
     *        iOS 平台需传入 App 和 Extension 共同使用的 group id，Android平台传空即可。
     * @param [in] capture_params
     *        屏幕共享的编码参数配置，详见：DesktopCaptureParameters{@link #DesktopCaptureParameters}
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + -1: 失败；  <br>
     * @notes 开启屏幕共享，移动端专用接口。
     */
    virtual int StartScreenCapture(
                                   void *context,
                                   const char *group_id,
                                   const DesktopCaptureParameters& capture_params) = 0;

    /**
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 更新屏幕共享区域。
     * @param  [in] region_rect  <br>
     *       + 当共享屏幕时，指定待共享区域相对于虚拟屏幕的位置，详见：Rectangle{@link #Rectangle}  <br>
     *       + 当共享窗口时，指定待共享区域相对于整个窗口的位置，详见：Rectangle{@link #Rectangle}  <br>
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + -1: 失败；  <br>
     */
    virtual int UpdateScreenCaptureRegion(const Rectangle& region_rect) = 0;

    /**
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 更新屏幕共享的编码参数配置。
     * @param [in] capture_params
     *        屏幕共享的编码参数配置，详见：DesktopCaptureParameters{@link #DesktopCaptureParameters}
     * @return  <br>
     *        + 0: 成功；  <br>
     *        + -1: 失败；  <br>
     */
    virtual int UpdateScreenCaptureParameters(const DesktopCaptureParameters& capture_params) = 0;

    /**
     * @type api
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     * @brief 停止屏幕或者窗口共享。
     * @notes  <br>
     *       + 本函数必须在 StartScreenCaptureByScreenRect{@link #StartScreenCaptureByScreenRect} 或者
     * StartScreenCaptureByWindowId{@link #StartScreenCaptureByWindowId} 之后调用  <br>
     *       + 本函数不可和 PublishScreen{@link #IRtcRoom#PublishScreen} 、UnpublishScreen{@link
     * #IRtcRoom#UnpublishScreen} 混用。  <br>
     */
    virtual void StopScreenCapture() = 0;

    /**
     * @brief 获取共享对象(应用窗口和桌面)列表, 使用完之后需要调用对应的release接口释放
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     */
    virtual IScreenCaptureSourceList* GetScreenCaptureSourceList() = 0;

    /**
     * @brief 获取共享对象缩略图
     * @region 屏幕共享
     * @author zhangzhenyu.samuel
     */
    virtual IVideoFrame* GetThumbnail(
            ScreenCaptureSourceType type, view_t source_id, int max_width, int max_height) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 设置是否启用自定义视频采集
     * @param  [in] enable  <br>
     *       + true: 开启  <br>
     *       + false: 关闭
     * @notes  <br>
     *       + 1. 该方法在进房前后均可以调用。  <br>
     *       + 2. 当你已调用 StartVideoCapture{@link #StartVideoCapture} 开启内部采集后，再调用此方法切换至自定义采集时，SDK 会自动关闭内部采集。  <br>
     *       + 3. 当你调用此方法开启自定义采集后，想要切换至内部采集，你必须先调用此方法关闭自定义采集，然后调用 StartVideoCapture{@link #StartVideoCapture} 手动开启内部采集。 
     */
    virtual void SetExternalVideoSource(bool enable) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 切换视频采集方式（内部采集/自定义采集）
     * @param [in] stream_index 媒体流类型，参看 StreamIndex{@link #StreamIndex}
     * @param [in] type 视频采集方式，参看 VideoSourceType{@link #VideoSourceType}
     * @notes  <br>
     *       + 默认使用内部采集。内部采集指：使用 RTC SDK 内置的视频采集机制进行视频采集。 <br>
     *       + 该方法进房前后均可调用。  <br>
     *       + 当你已调用 StartVideoCapture{@link #StartVideoCapture} 开启内部采集后，再调用此方法切换至自定义采集时，SDK 会自动关闭内部采集。  <br>
     *       + 当你调用此方法开启自定义采集后，想要切换至内部采集，你必须先调用此方法关闭自定义采集，然后调用 StartVideoCapture{@link #StartVideoCapture} 手动开启内部采集。
     */
    virtual void SetVideoSourceType(StreamIndex stream_index, VideoSourceType type) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author liyi.000
     * @brief 发送外部源视频数据
     * @param [in] frame 设置视频帧，参看 IVideoFrame{@link #IVideoFrame}。  <br>
     * @notes  <br>
     *       + 暂时只支持 YUV420P 格式的视频帧。  <br>
     *       + 该函数运行在用户调用线程内  <br>
     */
    virtual bool PushExternalVideoFrame(IVideoFrame* frame) = 0;
#endif

    /**
     * @hidden(macOS,Windows)
     * @type api
     * @region 音频设备管理
     * @author wangjunzheng
     * @brief 设置音频播放设备，默认使用扬声器。  <br>
     *        音频播放设备发生变化时，会收到 OnAudioPlaybackDeviceChanged{@link #IRtcEngineLiteEventHandler#OnAudioPlaybackDeviceChanged} 回调。
     * @param [in] device 音频播放设备。参看 AudioPlaybackDevice{@link #AudioPlaybackDevice} <br>
     * @return 方法调用结果  <br>
     *        + 0: 方法调用成功  <br>
     *        + <0: 方法调用失败  <br>
     * @notes  <br>
     *       + 1. 该接口仅适用于移动设备。  <br>
     *       + 2. 该方法只支持将音视频播放设备设置为听筒或者扬声器。当 App 连接有线或蓝牙音频播放设备时，SDK 会自动切换到有线或蓝牙音频播放设备。主动设置为有线或蓝牙音频播放设备，会返回调用失败。  <br>
     *       + 3. 若连接有线或者蓝牙音频播放设备时，将音频播放设备设置为扬声器或听筒将调用成功，但不会立马切换到扬声器或听筒，会在有线或者蓝牙音频播放设备移除后，根据设置自动切换到听筒或者扬声器。  <br>
     *       + 4. 通话前和通话中都可以调用该方法。  <br>
     */
    virtual int SetAudioPlaybackDevice(AudioPlaybackDevice device) = 0;

    /**
     * @hidden
     * @type api
     * @region 房间管理
     * @author shenpengliang
     * @brief 创建并获取一个 IRTCRoom{@link #IRTCRoom} 对象
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @notes
     *        1.参数 room_id 没有默认值，请确保对该参数正确赋值。
     *        2.请勿给参数 room_id 赋空字符串""或者空指针，否则将无法正确的创建房间对象。
     *        3.用户可以多次调用此方法创建多个 IRTCRoom{@link #IRTCRoom} 对象，再分别调用各 IRTCRoom{@link #IRTCRoom}
     * 对象的 JoinRoom{@link #JoinRoom} 方法，实现同时加入多个房间。
     *        4.加入多个房间后，用户可以同时订阅各房间的音视频流，但是目前仅支持同一时间在一个房间内发布一路音视频流。
     */
    virtual IRtcRoom* CreateRtcRoom(const char* room_id) = 0;

    /**
     * @type api
     * @region 音视频回退
     * @author panjian.fishing
     * @brief 设置发布的音视频流回退选项  <br>
     *       你可以调用这个接口来设置网络情况不佳或设备性能不足时只发送小流，以保证通话质量。
     * @param [in] option 本地发布的音视频流回退选项  <br>
     *       + 0：（默认）上行网络较弱或性能不佳时，不对音视频流作回退处理。  <br>
     *       + 1：上行网络较弱或性能不佳时，只发送视频小流。
     * @return  <br>
     *        + 0: 方法调用成功  <br>
     *        + <0: 方法调用失败
     * @notes  <br>
     *       + 这个方法只在设置了发送多个流的情况下有效。  <br>
     *       + 你必须在进房前设置，进房后设置或更改设置无效。  <br>
     *       + 设置回退选项后，本端发布的音视频流发生回退或从回退中恢复时，订阅该音视频流的客户端会收到 OnSimulcastSubscribeFallback{@link
     * #OnSimulcastSubscribeFallback} 回调通知。  <br>
     *       + 你可以调用 API 或者在服务端下发策略设置回退。当使用服务端下发配置实现时，下发配置优先级高于在客户端使用 API 设定的配置。
     */
    virtual int SetPublishFallbackOption(PublishFallbackOption option) = 0;

    /**
     * @type api
     * @region 音视频回退
     * @author panjian.fishing
     * @brief 设置订阅的音视频流回退选项  <br>
     *        你可以通过调用该接口来设置网络情况不佳或性能不足时只订阅小流或音频流，以保证通话质量。
     * @param [in] option 远端订阅流回退处理选项，详见枚举类型 SubscribeFallbackOption{@link #SubscribeFallbackOption}
     * @return 0: 方法调用成功  <br>
     *        + <0: 方法调用失败
     *  @notes  <br>
     *        + 你必须在进房前设置，进房后设置或更改设置无效。  <br>
     *        + 设置回退选项后，本端订阅的音视频流发生回退或从回退中恢复时,会收到 OnSimulcastSubscribeFallback{@link
     * #OnSimulcastSubscribeFallback} 回调通知。  <br>
     *        + 设置回退选项后，本端订阅的视频流因为回退分辨率发生变化时,会收到 OnRemoteVideoSizeChanged{@link
     * #OnRemoteVideoSizeChanged} 回调通知。  <br>
     *        + 你可以调用 API 或者在服务端下发策略设置回退。当使用服务端下发配置实现时，下发配置优先级高于在客户端使用 API 设定的配置。
     */
    virtual int SetSubscribeFallbackOption(SubscribeFallbackOption option) = 0;

    /**
     * @type api
     * @region 音视频回退
     * @author panjian.fishing
     * @brief 设置用户优先级
     * @param [in] user_id 远端用户的 ID
     * @param [in] priority 远端用户的需求优先级，详见枚举类型RemoteUserPriority{@link #RemoteUserPriority}
     * @return 0: 方法调用成功  <br>
     *        + <0: 方法调用失败  <br>
     * @notes  <br>
     *        + 1. 该方法与 SetSubscribeFallbackOption{@link #SetSubscribeFallbackOption} 搭配使用。  <br>
     *        + 2. 如果开启了订阅流回退选项，弱网或性能不足时会优先保证收到的高优先级用户的流的质量。  <br>
     *        + 3. 该方法在进房前后都可以使用，可以修改远端用户的优先级。  <br>
     */
    virtual int SetRemoteUserPriority(const char* user_id, RemoteUserPriority priority) = 0;

    /**
     * @type api
     * @region 引擎管理
     * @author chenweiming.push
     * @brief 设置业务标识参数。  <br>
     *        可通过 business_id 区分不同的业务场景（角色/策略等）。business_id 由客户自定义，相当于一个“标签”，
     *        可以分担和细化现在 AppId 的逻辑划分的功能。
     * @param [in] business_id  <br>
     *        用户设置的自己的 business_id 值。business_id 相当于一个 sub AppId，可以分担和细化现在 AppId
     * 的逻辑划分的功能， 但不需要鉴权。business_id 只是一个标签，颗粒度需要用户自定义。
     * @return  <br>
     *        + 0： 成功。  <br>
     *        + <0： 失败，具体原因参照 BusinessCheckCode{@link #BusinessCheckCode} 。  <br>
     *        + -6001： 用户已经在房间中。  <br>
     *        + -6002：
     * 输入非法，合法字符包括所有小写字母、大写字母和数字，除此外还包括四个独立字符分别是：英文句号，短横线，下划线和 @
     * 。
     * @notes  <br>
     *        + 需要在调用 JoinRoom{@link #JoinRoom} 之前调用，JoinRoom{@link #JoinRoom}之后调用该方法无效。
     */
    virtual int SetBusinessId(const char* business_id) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author zhangzhenyu.samuel
     * @brief 开启背景并设置背景模式与分割模式
     * @param [in] mode 背景模式，参看 BackgroundMode{@link #BackgroundMode}。  <br>
     * @param [in] divide_model 分割模型，参看 DivideMode{@link #DivideMode}。  <br>
     * @return  <br>
     *        + 0：成功  <br>
     *        + !0：失败  <br>
     */
    virtual int ReplaceBackground(BackgroundMode mode, DivideMode divide_model) = 0;

    /**
     * @type api
     * @region 视频特效
     * @author zhangzhenyu.samuel
     * @brief 关闭背景
     */
    virtual void DisableBackground() = 0;

    /**
     * @type api
     * @region 视频管理
     * @author zhangzhenyu.samuel
     * @brief 设置采用前置摄像头采集时，是否开启镜像模式。 <br>
     *        调用后，你会收到回调：OnMirrorStateChanged{@link #OnMirrorStateChanged}
     * @param  [in] mirrorMode 是否开启镜像模式
     */
    virtual void SetLocalVideoMirrorMode(MirrorMode mirrorMode) = 0;


    /**
     * @type api
     * @region 视频管理
     * @author zhangzhenyu.samuel
     * @brief 获得当前本地镜像状态
     * @return  <br>
     *        + true：是镜像模式  <br>
     *        + false：不是镜像模式  <br>
     */
    virtual bool GetLocalVideoMirrorMode() = 0;

    /**
     * @hidden
     * @type api
     * @region 视频特效
     * @author zhangzhenyu.samuel
     * @brief 获取视频特效接口
     * @return 视频特效接口指针
     */
    virtual IVideoEffect* GetVideoEffectInterface() = 0;

    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 通过设置音频处理策略等级，获得不同的音频处理质量。默认使用 auto 模式。 <br>
     *        此设置会影响音频采集端在音频采集后，发送前对音频的处理策略，达成不同的处理效果； <br>
     *        也会影响音频接收端在音频流接收后，播放前对音频的处理策略，达成不同的处理效果。 <br>
     *        你应使用此方法，实现音频体验和系统性能占用之间的平衡: <br>
     *        如果你希望系统性能占用较低，你应设置较低的策略等级；如果你更在意音频质量，你应选择设置更高的策略等级。 <br>
     *        一般情况下，你可使用 auto 模式（默认）。此时，sdk 会根据应用所在机型等级，自动适配音频处理策略。
     * @param profile 音频处理策略等级, 参看 AudioPerformanceProfile{@link #AudioPerformanceProfile}
     * @notes  <br>
     *       + 如果你希望在此次音视频通话中设置音频处理策略等级，你必须在进房前调用此方法。  <br>
     *       + 如果在进房后调用此方法，下次进房时才会应用你设置的策略等级。 <br>
     *       + 如果以下策略无法满足需求，请联系技术支持人员实现自定义配置。  <br>
     */
    virtual void SetAudioPerformanceProfile(AudioPerformanceProfile profile) = 0;


    /**
     * @type api
     * @region 加密
     * @author sunhang.io
     * @brief 设置传输时使用内置加密的方式 <br>
     * @param encrypt_type 内置加密算法，详见 EncryptType{@link #EncryptType}
     * @param [in] encrypt_type 加密类型，详见 EncryptType{@link #EncryptType}
     * @param [in] key 加密密钥，长度限制为 36 位，超出部分将会被截断
     * @param [in] key_size 参数 key 的长度
     * @notes  <br>
     *       + 使用传输时内置加密时，使用此方法；如果需要使用传输时自定义加密，参看 OnEncryptData{@link #OnEncryptData}。
     *         内置加密和自定义加密互斥，根据最后一个调用的方法确定传输是加密的方案。  <br>
     *       + 该方法必须在进房之前调用，可重复调用，以最后调用的参数作为生效参数。  <br>
     */
    virtual void SetEncryptInfo(EncryptType encrypt_type, const char* key, int key_size) = 0;

    /**
     * @type api
     * @region 加密
     * @author sunhang.io
     * @brief 设置自定义加密  <br>
     *        需要实现对应的加密/解密方法，详情参考 EncryptType{@link #EncryptType} 。 <br>
     * @param [in] handler 自定义加密 handler，需要实现 handler 的加密和解密方法  <br>
     * @notes  <br>
     *       + 该方法与 SetEncryptInfo{@link #SetEncryptInfo} 为互斥关系，
     *         只能选择自定义加密方式或者默认加密方式。最终生效的加密方式取决于最后一个调用的方法。  <br>
     *       + 该方法必须在进房之前调用，可重复调用，以最后调用的参数作为生效参数。  <br>
     *       + 无论加密或者解密，其对原始数据的长度修改，需要控制在 90% ~ 120% 之间，即如果输入数据为 100 字节，则处理完成后的数据必须在
     *         90 至 120 字节之间，如果加密或解密结果超出该长度限制，则该音视频帧会被丢弃。  <br>
     *       + 数据加密/解密为串行执行，因而视实现方式不同，可能会影响到最终渲染效率。是否使用该方法，需要由使用方谨慎评估。  <br>
     */
    virtual void SetCustomizeEncryptHandler(IEncryptHandler* handler) = 0;

    /**
     * @type api
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 设置本地麦克风录制的音频数据回调参数
     * @param [in] sample_rate 音频采样率（单位HZ），可以设置的值有 8000，16000，32000，44100，48000
     * @param [in] channels 音频通道数，支持单声道（1）和双声道（2）
     * @notes 使用本方法设置参数，并使用RegisterAudioFrameObserver{@link #RegisterAudioFrameObserver}注册数据观察者，
     *        之后可以在IAudioFrameObserver{@link #IAudioFrameObserver}的
     *        OnRecordAudioFrame{@link #IAudioFrameObserver#OnRecordAudioFrame}收到数据
     */
    virtual void SetRecordingAudioFrameParameters(int sample_rate, int channels) = 0;

    /**
     * @type api
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 设置远端所有用户音频数据混音后的音频数据回调参数
     * @param [in] sample_rate 音频采样率（单位HZ），可以设置的值有 8000，16000，32000，44100，48000
     * @param [in] channels 音频通道数，支持单声道（1）和双声道（2）
     * @notes 使用本方法设置参数，并使用RegisterAudioFrameObserver{@link #RegisterAudioFrameObserver}注册数据观察者，
     *        之后可以在IAudioFrameObserver{@link #IAudioFrameObserver}的
     *        OnPlaybackAudioFrame{@link #IAudioFrameObserver#OnPlaybackAudioFrame}收到数据
     */
    virtual void SetPlaybackAudioFrameParameters(int sample_rate, int channels) = 0;

    /**
     * @type api
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 设置本地麦克风录制音频数据和远端所有用户音频数据混音后的音频数据回调参数
     * @param [in] sample_rate 音频采样率（单位HZ），可以设置的值有 8000，16000，32000，44100，48000
     * @param [in] channels 音频通道数，支持单声道（1）和双声道（2）
     * @notes 使用本方法设置参数，并使用RegisterAudioFrameObserver{@link #RegisterAudioFrameObserver}注册数据观察者，
     *        之后可以在IAudioFrameObserver{@link #IAudioFrameObserver}的
     *        OnMixedAudioFrame{@link #IAudioFrameObserver#OnMixedAudioFrame}收到数据
     */
    virtual void SetMixedAudioFrameParameters(int sample_rate, int channels) = 0;

    /**
     * @type api
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 注册音频数据回调观察者
     * @param [in] observer 音频数据观察者，详见：IAudioFrameObserver{@link #IAudioFrameObserver}
     * @notes 本方法需要与下面3个方法配合使用：
     *        SetRecordingAudioFrameParameters{@link #SetRecordingAudioFrameParameters}、
     *        SetPlaybackAudioFrameParameters{@link #SetPlaybackAudioFrameParameters}、
     *        SetMixedAudioFrameParameters{@link #SetMixedAudioFrameParameters}
     */
    virtual void RegisterAudioFrameObserver(IAudioFrameObserver* observer) = 0;

    /**
     * @type api
     * @region 视频数据回调
     * @author sunhang.io
     * @brief 注册视频数据回调观察者
     * @param [in] observer 数据回调函数，详见： IVideoFrameObserver{@link #IVideoFrameObserver} 。
     * @notes  <br>
     *       + 该方法可以在任意时间调用，建议在 JoinRoom{@link #JoinRoom} 前。  <br>
     *       + 本接口支持动态取消注册，将参数设置为 nullptr 取消注册。  <br>
     */
    virtual void RegisterVideoFrameObserver(IVideoFrameObserver* observer) = 0;

    /**
     * @type api
     * @region 视频数据回调
     * @author sunhang.io
     * @brief 注册 metadata 观察者，用于接收或发送 metadata，底层通过在视频帧中添加 SEI 数据实现该功能。  <br>
     *        注册观察者后，发送的视频帧里面没有 SEI 信息， 会触发 OnReadyToSendMetadata{@link
     * #IMetadataObserver#OnReadyToSendMetadata} 回调。  <br> 注册观察者后，接收的视频帧里面有 SEI 信息，会触发接收
     * OnMetadataReceived{@link #IMetadataObserver#OnMetadataReceived} 回调。  <br>
     * @param [in] observer metadata 观察者，详见：IMetadataObserver{@link #IMetadataObserver}
     * @notes  <br>
     *      + 使用视频自定义采集与渲染时，可以直接在视频帧中添加与获取 metadata，不建议使用本接口。
     *      + 本接口支持动态取消注册，将参数设置为 nullptr 取消注册。  <br>
     */
    virtual void RegisterMetadataObserver(IMetadataObserver* observer) = 0;

    /**
     * @type api
     * @region 房间管理
     * @brief 设置引擎自动发布属性
     * @param [in] engine
     *       要设置的引擎，详见：{@link #IRtcEngine}
     * @param [in] auto_publish
     *       是否自动发布流，默认是true自动发布，设置false时可通过engine->publish()、engine->unpublish()来发布和取消发布本地流
     * @notes
     * 设置引擎是否自动发布用户的本地流，必须在joinChannel前调用。engine引擎默认auto_publish为true。当auto_publish为true时engine->publish()、
     * engine->unpublish()接口不应当被调用。
     */
    virtual void EnableAutoPublish(bool auto_publish) = 0;

    /**
     * @type api
     * @region 视频设备管理
     * @author liyi.000
     * @brief 创建视频设备管理实例
     * @param [in] engine
     *        用于创建视频设备管理实例的引擎，详见IRtcEngine{@link #IRtcEngine}
     * @return 视频设备管理实例，详见IVideoDeviceManager{@link #IVideoDeviceManager}
     * @notes 当不再使用IVideoDeviceManager实例时，调用该实例的 Release{@link #Release}
     * 接口，以免资源泄露
     */
    virtual IVideoDeviceManager* GetVideoDeviceManager() = 0;

    /**
     * @type api
     * @region 音频管理
     * @author wangjunzheng
     * @brief 设备音频管理接口创建
     * @param [in] engine
     *       要设置的引擎，详见：{@link #IRtcEngine}
     * @return 音频设备管理接口
     */
    virtual IAudioDeviceManager* GetAudioDeviceManager() = 0;

    /**
     * @hidden
     * @type api
     * @region 音频数据回调
     * @author wangjunzheng
     * @brief 注册音频数据回调观察者。
     * @param [in] observer 音频数据回调观察者，详见 IRemoteAudioFrameObserver{@link #IRemoteAudioFrameObserver}
     * @notes  <br>
     *         注册该回调，可以收到单个远端用户的 PCM 数据。
     */
    virtual void RegisterRemoteAudioFrameObserver(IRemoteAudioFrameObserver* observer) = 0;

    /**
     * @type api
     * @region 引擎管理
     * @author chenweiming.push
     * @brief 设置运行时的参数
     * @param [in] json_string  json 序列化之后的字符串
     * @notes
     */
    virtual void SetRuntimeParameters(const char * json_string) = 0;
    /**
     * @type api
     * @region 视频数据回调
     * @brief 注册本地视频帧监测器。  <br>
     *        无论使用内部采集还是自定义采集，调用该方法后，SDK 每监测到一帧本地视频帧时，都会将视频帧信息通过 OnLocalEncodedVideoFrame{@link #OnLocalEncodedVideoFrame} 回调给用户
     * @param [in] engine 需设置的引擎，参看 IRtcEngine{@link #IRtcEngine}
     * @param [in] observer 本地视频帧监测器，参看 ILocalEncodedVideoFrameObserver{@link #ILocalEncodedVideoFrameObserver}。将参数设置为 nullptr 则取消注册。
     * @notes 该方法可在进房前后的任意时间调用，调用越早，对视频帧的监测越早
     */
    virtual void RegisterLocalEncodedVideoFrameObserver(ILocalEncodedVideoFrameObserver* observer) = 0;

    /**
     * @hidden
     * @type api
     * @region 视频数据回调
     * @brief 注册远端视频帧监测器。  <br>
     *        调用该方法后，SDK 每监测到一帧远端视频帧时，都会将视频帧信息通过 OnRemoteEncodedVideoFrame{@link #OnRemoteEncodedVideoFrame} 回调给用户
     * @param [in] engine 需设置的引擎，参看 IRtcEngine{@link #IRtcEngine}
     * @param [in] observer 远端视频帧监测器，参看 IRemoteEncodedVideoFrameObserver{@link #IRemoteEncodedVideoFrameObserver}
     * @notes  <br>
     *       + 该方法建议在进房前调用。 <br>
     *       + 将参数设置为 nullptr 则取消注册。
     */
    virtual void RegisterRemoteEncodedVideoFrameObserver(IRemoteEncodedVideoFrameObserver* observer) = 0;

    /**
     * @hidden
     * @type api
     * @region 视频管理
     * @brief 注册远端视频帧监测器。  <br>
     *        调用该方法后，SDK 每监测到一帧远端视频帧时，都会将视频帧信息通过 OnRemoteEncodedVideoFrame{@link #OnRemoteEncodedVideoFrame} 回调给用户
     * @param [in] engine 需设置的引擎，参看 IRtcEngine{@link #IRtcEngine}
     * @param [in] observer 远端视频帧监测器，参看 IRemoteEncodedVideoFrameObserver{@link #IRemoteEncodedVideoFrameObserver}
     * @notes  <br>
     *       + 该方法建议在进房前调用。 <br>
     *       + 将参数设置为 nullptr 则取消注册。
     */
    virtual void SetExternalVideoEncoderEventHandler(IExternalVideoEncoderEventHandler* encoder_handler) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author panjian.fishing
     * @brief 推入编码后视频流
     * @param [in] stream_index 媒体流类型，参看 StreamIndex{@link #StreamIndex}
     * @param [in] video_index 视频流下标, SetVideoEncoderConfig
     * @param [in] frame 设置视频帧，参看 IVideoFrame{@link #IVideoFrame}。  <br>
     * @notes  <br>
     *       + 暂时只支持 YUV420P 格式的视频帧。  <br>
     *       + 该函数运行在用户调用线程内  <br>
     * @notes 推送外部视频帧前，必须调用 SetVideoSourceType{@link #SetVideoSourceType} 开启外部视频源采集。
     */
    virtual bool PushExternalEncodedVideoFrame(StreamIndex index, int video_index, IEncodedVideoFrame* video_stream) = 0;

    /**
     * @type api
     * @region 视频管理
     * @author panjian.fishing
     * @brief 在订阅一路远端视频流之前，设定是否要开启视频解码
     * @param [in] stream_key 远端流信息，用于哪一路视频流，参看 RemoteStreamKey{@link #RemoteStreamKey}。
     * @param [in] config 视频解码器配置，参看 VideoDecoderConfig{@link #VideoDecoderConfig}。
     * @notes  <br>
     *        + 需要开启手动订阅模式，并且在订阅之前设置。
     */
    virtual void SetVideoDecoderConfig(RemoteStreamKey key, VideoDecoderConfig config) = 0;

    // 订阅过程中向远端请求关键帧
    /**
     * @type api
     * @region 视频管理
     * @author panjian.fishing
     * @brief 在订阅一路远端视频流之后，订阅过程中向远端请求关键帧
     * @param [in] stream_key 远端流信息，用于指定需要渲染的视频流来源及属性，参看 RemoteStreamKey{@link #RemoteStreamKey}。
     * @param [in] config 视频解码器配置，参看 VideoDecoderConfig{@link #VideoDecoderConfig}。
     * @notes  <br>
     *        + 需要开启手动订阅模式，并且在订阅之前设置。
     */
    virtual void RequestRemoteVideoKeyFrame(const RemoteStreamKey& stream_info) = 0;
};

}  // namespace bytertc

#endif // BYTE_RTC_LITE_INTERFACE_H__
