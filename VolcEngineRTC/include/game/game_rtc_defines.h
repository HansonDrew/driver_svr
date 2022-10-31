/*
 *  Copyright (c) 2021 The ByteRtc project authors. All Rights Reserved.
 *  @company ByteDance.Inc
 *  @brief 游戏语音引擎配置信息
 */
#pragma once

#ifdef WIN32
    #define GAME_RTC_API extern "C" __declspec(dllexport)
#elif __APPLE__
#include <TargetConditionals.h>
    #if TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
        #define GAME_RTC_API __attribute__((__visibility__("default"))) extern "C"
    #else
        #define GAME_RTC_API __attribute__((__visibility__("default")))
    #endif
#else
    #define GAME_RTC_API __attribute__((__visibility__("default")))
#endif

namespace bytertc {
namespace gamertc {

/**
* @type keytype
* @brief 用户机型等级信息。
*        在部分应用场景下，需要关注用户机型的配置高低，根据不同的配置，选择不同复杂度的音频处理算法，在能耗、用户体验上进行折中。
*/
enum class AudioPerfProfile {
    /**
    * @brief 自动策略，SDK 会在加入房间时上传机型信息。服务端根据机型信息判断性能级别，下发对应配置。
    */
    kAudioPerfProfileAuto = 0,
    /**
    * @brief 低配策略，关闭AEC、ANS、AGC算法，播放sample rate为16000。
    *        策略内容可以定制，联系销售咨询具体方案
    */
    kAudioPerfProfileLow = 1,
    /**
    * @brief 中配策略，开启AEC、ANS、AGC算法，播放sample rate为16000。
    *        策略内容可以定制，联系销售咨询具体方案
    */
    kAudioPerfProfileMid = 2,
    /**
    * @brief 高配策略，开启AEC、ANS、AGC算法，播放sample rate为48000。
    *        策略内容可以定制，联系销售咨询具体方案
    */
    kAudioPerfProfileHigh = 3,
};

/**
 * @type keytype
 * @brief SDK 的服务器环境。
 */
enum RTCEnv {
    /**
     * @brief 线上环境。
     */
    kRTCEnvProduct = 0,
    /**
     * @brief BOE 环境。
     */
    kRTCEnvBOE,
    /**
     * @brief 测试环境。
     */
    kRTCEnvTest
};

/**
 * @type keytype
 * @brief 引擎配置信息
 */
struct EngineConfig {
    /**
     * @brief 用于区分使用 RTC 服务的应用，只有属于同一 app_id 的应用才能实现实时语音互通
     */
    const char* app_id;
    /**
     * @brief 设备 ID，用来唯一区分使用实时语音的设备
     */
    const char* device_id;
    /**
     * @brief 指定服务的环境类型
     */
    RTCEnv env = kRTCEnvProduct;
    /**
     * @brief 指定机型等级
     */
    AudioPerfProfile audio_perf_profile = AudioPerfProfile::kAudioPerfProfileAuto;
};

/**
 * @type keytype
 * @brief 房间类型
 */
enum class RoomType {
    /**
     * @brief 小队房间，进入同一房间的成员是队友关系，打开麦克风可以向房间所有成员讲话
     * 打开扬声器就可以收听房间内其他成员讲话
     */
    kRoomTypeTeam = 0,
    /**
     * @brief 世界房间，进入同一房间的需要判断是否为队友关系，进而决定收听逻辑
     */
    kRoomTypeWorld = 1
};

/**
 * @type keytype
 * @brief 房间配置信息
 */
struct RoomConfig {
    /**
     * @brief 设置是否开启范围语音
     */
    bool enable_range_audio = false;
    /**
     * @brief 建议设置到大于等于200 毫秒。少于 10 毫秒时，行为未定义
     */
    int audio_volume_indication_interval = 0;
    /**
     * @brief 房间类型
     */
    RoomType room_type = RoomType::kRoomTypeTeam;
};

/**
 * @type keytype
 * @brief 用于表示用户音量大小
 */
struct AudioVolumeInfo {
    AudioVolumeInfo(unsigned int v, const char* u): volume(v), user_id(u) {}
    /**
     * @brief 音量大小，范围 [0,255]
     */
    unsigned int volume;
    /**
     * @brief 用户ID
     */
    const char* user_id;
};

/**
 * @type keytype
 * @brief 用户离线原因
 * 房间内的远端用户离开房间时，本地用户会收到 OnUserOffline{@link #OnUserOffline} 回调通知，
 * 此枚举类型为回调的用户离线原因
 */
enum UserLeaveReasonType {
    /**
     * @brief 无效值
     */
    kUserLeaveReasonTypeInvalid = -1,
    /**
     * @brief 用户主动离开。即远端用户调用 LeaveChannel{@link #LeaveChannel} 方法退出房间
     */
    kUserLeaveReasonTypeQuit = 0,
    /**
     * @brief 用户掉线。远端用户因为网络等原因掉线
     */
    kUserLeaveReasonTypeDropped = 1
};

/**
 * @type keytype
 * @brief SDK 与信令服务器连接状态。
 */
enum ConnectionState {
    /**
     * @brief 无效值。
     */
    kConnectionStateInvalid = -1,
    /**
     * @brief 连接断开。
     */
    kConnectionStateDisconnected = 0,
    /**
     * @brief 首次连接，正在连接中。
     */
    kConnectionStateConnecting = 1,
    /**
     * @brief 首次连接成功。
     */
    kConnectionStateConnected = 2,
    /**
     * @brief 连接断开后重新连接中。
     */
    kConnectionStateReconnecting = 3,
    /**
     * @brief 连接断开后重连成功。
     */
    kConnectionStateReconnected = 4,
    /**
     * @brief 网络连接断开超过 10 秒，仍然会继续重连。
     */
    kConnectionStateLost = 5,
};

/**
 * @brief 描述范围语音中语音接收范围
 */
struct ReceiveRange {
    /**
     * @brief 不同小队成员设置收发世界模式并且开启区域语音时，设置收听的最小范围值，在小于 min 的时候，声音完全无衰减;
     */
    int min;
    /**
     *  @brief 不同小队成员设置收发世界模式并且开启区域语音时，设置收听的最大范围值，在大于 max 的时候，声音无法听见
     *         在 [min, max) 之间根据距离远近实现声音的衰减
     */
    int max;
};

/**
 * @brief 描述范围语音中玩家位置
 */
struct PositionInfo {
    /**
     * @brief x 坐标
     */
    int x;
    /**
     * @brief y 坐标
     */
    int y;
    /**
     * @brief z 坐标
     */
    int z;
};

/**
 * @brief 设置id最大长度
 */
const unsigned int MAX_DEVICE_ID_LENGTH = 512;

/**
 * @brief 音频设备信息
 */
struct AudioDeviceInfo {
    /**
     * @brief 音频设备id
     */
    char device_id[MAX_DEVICE_ID_LENGTH];
    /**
     * @brief 音频设备名字
     */
    char device_name[MAX_DEVICE_ID_LENGTH];
};

/**
 * @type keytype
 * @brief 媒体流网络质量。
 */
enum NetworkQuality {
    /**
     * @brief 无效值。
     */
    kNetworkQualityInvalid = -1,
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

/**
 * @brief 加入房间错误码
 *        用户加入房间回调的状态码，0 为成功，非0 表示失败
 */
 enum JoinRoomErrorCode {
     /**
    * @brief 加入房间成功
    */
     kJoinRoomSuccess = 0,
     /**
      * @brief Token 无效。
      *        调用 JoinRoom 方法时使用的 Token 无效或过期失效。需要用户重新获取 Token，并调用
      *        RenewToken 方法更新 Token。
      */
     kJoinRoomInvalidToken = -1000,
     /**
      * @brief 加入房间错误。
      *        调用 JoinRoom 方法时发生未知错误导致加入房间失败。需要用户重新加入房间。
      */
     kJoinRoomError = -1001,
     /**
      * @brief 加入房间失败。
      *        用户调用 JoinRoom 加入房间或由于网络状况不佳断网重连时，由于服务器错误导致用户加入房间失败，
      *        SDK 会自动重试加入房间。
      */
     kJoinRoomFailed = -2001
 };
/**
 * @brief 房间错误码
 */
enum RoomErrorCode {
    /**
    * @brief 无效值
    */
    kRoomErrorInvalid = -1,
    /**
     *  @brief 没有发布音频流权限，
     *         用户在所在房间中发布音视频流失败，失败原因为用户没有发布流的权限，需要检查 appId 是否正确
     */
    kRoomErrorNoPublishPermission = -1002,
    /**
     * @brief 没有订阅音频流权限。
     *        用户订阅所在房间中的音视频流失败，失败原因为用户没有订阅流的权限，需要检查 appId 是否正确
     */
    kRoomErrorNoSubscribePermission = -1003,
    /**
     * @brief 本端用户所在房间中有相同用户ID的用户登录，导致本端用户被踢出房间
     */
    kRoomErrorDuplicateLogin = -1004
};

/**
 * @brief 房间警告码
 */
enum RoomWarnCode {
    /**
    * @brief 无效值
    */
    kRoomWarnInvalid = -1,
    /**
     * @brief 发布音频流失败。
     *        用户在所在房间中发布音视频流时，由于服务器错误导致发布失败，
     *        SDK 会自动重试发布。
     */
    kRoomWarnPublishStreamFailed = -2002,
    /**
     * @brief 订阅音频流失败。
     *        当前房间中找不到订阅的音视频流导致订阅失败。
     *        建议用户退出房间重新进房。
     */
    kRoomWarnSubscribeStreamFailed_404 = -2003,
    /**
     * @brief 订阅音频流失败。
     *        用户订阅所在房间中的音视频流时，由于服务器错误导致订阅失败
     *        SDK 会自动重试订阅。
     */
    kRoomWarnSubscribeStreamFailed_5xx = -2004,
    /**
     * @brief 调度异常，服务器返回的媒体服务器地址不可用。
     */
    kRoomWarnInvalidExpectMsAddr = -2007
};

/**
 * @brief 引擎警告码
 */
enum EngineWarnCode {
    /**
     * @brief 无效值
     */
    kEngineWarnInvalid = -1,
    /**
     * @brief 麦克风权限异常，当前应用没有获取麦克风权限
     */
    kEngineWarnNoMicrophonePermission = -5002,
    /**
     * @brief 录音设备启动失败。
     *        启动录音设备失败，当前录音设备可能被其他应用占用。
     */
    kEngineWarnADMRecordingStartFail = -5003,
    /**
     * @brief 播放设备启动失败警告
     *      可能由于系统资源不足，或参数错误
     */
    kEngineWarnADMPlayoutStartFail = -5004,
    /**
     * @brief 无可用录音设备
     *        启动录音设备失败，请插入可用的录音设备
     */
    kEngineWarnADMNoRecordingDevice = -5005,
    /**
     * @brief 无可用播放设备
     *        启动播放设备失败，请插入可用的播放设备
     */
    kEngineWarnADMNoPlayoutDevice = -5006
};

} // namespace gamertc
} // namespace bytertc