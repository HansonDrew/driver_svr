/*
 *  Copyright (c) 2021 The ByteRtc project authors. All Rights Reserved.
 *  @company ByteDance.Inc
 *  @brief 游戏语音引擎接口
 */
#pragma once

#include "game_rtc_defines.h"

#include <memory>

namespace bytertc {
namespace gamertc {
class IGameRTCEngine;
class IGameRTCEventHandler;

GAME_RTC_API IGameRTCEngine* CreateEngine(const EngineConfig& config,
    IGameRTCEventHandler* handler, const char* params);

GAME_RTC_API IGameRTCEngine* CreateEngineWithPtr(const EngineConfig& config,
    std::unique_ptr<IGameRTCEventHandler> handler, const char* params);

GAME_RTC_API void DestroyEngine(IGameRTCEngine* engine);


GAME_RTC_API const char* GetSdkVersion();

class IGameRTCEngine {
public:
    /**
     * @type api
     * @region 游戏语音
     * @brief 加入游戏语音房间，加入成功会收到回调 OnJoinRoomSuccess{@link #IGameRTCEventHandler#OnJoinRoomSuccess}
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] user_id
     *        用户 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] token
     *        动态密钥，用于对登录用户进行鉴权验证。进入房间需要携带 Token。可以在控制台生成临时 Token
     *        进行测试，正式上线需要使用密钥 SDK 在您的服务端生成并下发 Token
     * @param [in] config
     *        用于设置进房前的配置选项，具体参考 RoomConfig{@link #RoomConfig}
     * @return
     *        0:  表示参数检查通过，不代表进房成功，例如网络不通
     *        -1: 传入的参数为空导致失败
     */
    virtual int JoinRoom(const char* room_id, const char* user_id, const char* token,
        const RoomConfig& config) = 0;

    /*
     * @type api
     * @region 游戏语音
     * @brief 离开游戏语音房间，会触发 OnLeaveRoom{@link #IGameRTCEventHandler#OnLeaveRoom} 回调
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @return
     *        0:  表示参数检查通过，不代表退房成功，可能房间不存在
     *        -1: 传入的参数为空
     */
    virtual int LeaveRoom(const char* room_id) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 更新进房 token，在进房时如果因为 token 过期导致进房失败，会收到 OnRoomError{@link
     * #IGameRTCEventHandler#OnRoomError} 回调，在该回调中，可以再次调用 UpdateToken 以新 token 进房
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] token
     *        动态密钥，用于对登录用户进行鉴权验证。进入房间需要携带 Token。可以在控制台生成临时 Token
     *        进行测试，正式上线需要使用密钥 SDK 在您的服务端生成并下发 Token
     * @return
     *        0:  表示参数检查通过，不代表更新成功
     *        -1: 传入的参数为空
     */
    virtual int UpdateToken(const char* room_id, const char* token) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 更新收听范围
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] range
     *        语音收听范围值，具体参考类型定义
     * @return
     *        0:  表示参数检查通过，不代表更新成功
     *        -1: 传入的 room_id 为空
     */
    virtual int UpdateReceiveRange(const char* room_id, const ReceiveRange& range) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 更新坐标，在小对语音世界模式下开启区域语音时，距离远近会影响不同小队成员收听到的声音大小
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] pos
     *        三维坐标的值
     * @return
     *        0:  表示参数检查通过，不代表更新成功
     *        -1: 传入的 room_id 为空
     */
    virtual int UpdatePosition(const char* room_id, const PositionInfo& pos) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 开始讲话或者停止讲话时调用，调用该接口会启动或停止音频采集。同房间其他用户会收到
     * 回调 OnMicrophoneEnabled{@link #IGameRTCEventHandler#OnMicrophoneEnabled}
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] enable
     *             true 表示开启采集
     *             false 表示关闭采集
     * @return
     *        0:  表示参数检查通过，不代表打开麦克风会成功，比如房间不存在
     *        -1: 传入的room_id为空导致失败
     */
    virtual int EnableMicrophone(const char* room_id, bool enable) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 开始讲话或者停止讲话时调用，调用该接口会启动或停止音频发送，不影响音频采集。同房间其他用户会收到
     * 回调 OnAudioSendEnabled{@link #IGameRTCEventHandler#OnAudioSendEnabled}
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] enable
     *             true 表示启动音频数据的发送
     *             false 表示停止音频数据的发送
     * @return
     *        0:  表示参数检查通过，不代表打开麦克风会成功，比如房间不存在
     *        -1: 传入的room_id为空导致失败
     */
    virtual int EnableAudioSend(const char* room_id, bool enable) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 开关扬声器。自动订阅其他用户的数据，在开关扬声器时，不会执行订阅和取消订阅的操作，
     *        只是发送信令通知服务端，是否接收其他用户的音频数据。同房间其他用户会收到回调
     *        OnSpeakerEnabled{@link #IGameRTCEventHandler#OnSpeakerEnabled}
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     *  @param [in] enable
     *             true 表示接收所有远端用户的音频数据
     *             false  表示不接收所有远端用户的音频数据
     * @return
     *        0:  表示参数检查通过，不代表成功收听远端用户，比如房间不存在时仍然无法收听
     *        -1: 传入的room_id为空导致失败
     */
    virtual int EnableSpeakerphone(const char* room_id, bool enable) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 是否收听某个用户的声音
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] user_id
     *        用户 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] enable
     *        true：表示收听指定用户
     *        false：表示不收听指定用户
     * @return
     *        0:  表示参数检查通过，不代表实际操作是否成功
     *        -1: 传入的用户名或房间号参数为空导致失败
     */
    virtual int EnableAudioReceive(const char* room_id, const char* user_id, bool enable) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 调节录音音量
     * @param [in] volume 录音音量，可在 0~400 范围内进行调节
     *       + 0: 静音
     *       + 100: 原始音量
     *       + 400: 最大可为原始音量的 4 倍(自带溢出保护)
     * @return
     *       0: 返回值始终为0
     * @notes 为保证更好的通话质量，建议将 volume 值设为 [0,100]
     */
    virtual int SetRecordingVolume(int volume) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 调节本地播放的所有远端用户音量
     * @param [in] volume 播放音量，可在 0~400 范围内进行调节
     *       + 0: 静音
     *       + 100: 原始音量
     *       + 400: 最大可为原始音量的 4 倍(自带溢出保护) 
     * @return
     *       0: 返回值始终为0
     * @notes  
     *       + 该方法调节的是本地播放的所有远端用户混音后的音量
     *       + 为保证更好的通话质量，建议将 volume 值设为 [0,100]
     */
    virtual int SetPlaybackVolume(int volume) = 0;

    /**
     * @type api
     * @region 游戏语音
     * @brief 调节接收到的远端指定用户音量
     * @param [in] room_id
     *        标识通话房间的房间 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] user_id
     *        用户 ID，最大长度为128字节的非空字符串。支持的字符集范围为:
     *            1. 26个大写字母 A ~ Z
     *            2. 26个小写字母 a ~ z
     *            3. 10个数字 0 ~ 9
     *            4. 下划线"_", at符"@", 减号"-"
     * @param [in] volume  播放音量，可在 0~400 范围内进行调节  <br>
     *              + 0: 静音  <br>
     *              + 100: 原始音量  <br>
     *              + 400: 最大可为原始音量的 4 倍(自带溢出保护)
     * @return
     *        0:  表示参数检查通过，不代表实际操作是否成功
     *        -1: 传入的用户名或房间号参数为空导致失败
     */
    virtual int SetRemoteAudioPlaybackVolume(const char* room_id, const char* user_id, int volume) = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取设备列表中录音设备个数
     * @return 设备列表中设备个数
     * @notes  
     *       + 如果音频录音设备有变更，你需要重新调用该函数
     */
    virtual int GetRecordingDeviceCount() = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 设置音频采集设备
     * @param [in] device_id
     *        音频采集设备ID, 可通过 GetAllRecordingDevices 获取。
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int SetRecordingDevice(const char device_id[MAX_DEVICE_ID_LENGTH]) = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取当前音频采集设备ID
     * @param [out] device_id
     *        设备ID，使用方负责按 MAX_DEVICE_ID_LENGTH 大小，分配与释放内存
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int GetCurrentRecordingDevice(char device_id[MAX_DEVICE_ID_LENGTH]) = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取全部的音频采集设备信息
     * @param [out] info
     *        返回音频设备信息的数组首地址
     * @param [out] len
     *        数组长度
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int GetAllRecordingDevices(AudioDeviceInfo* info, int len) = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取设备列表中播放设备个数
     * @return 设备列表中设备个数
     * @notes  
     *       + 如果音频播放设备有变更，你需要重新调用该函数
     */
    virtual int GetPlaybackDeviceCount() = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 设置音频播放设备
     * @param [in] device_id
     *        音频播放设备ID, 可通过 GetAllPlaybackDevices 获取。
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int SetPlaybackDevice(const char device_id[MAX_DEVICE_ID_LENGTH]) = 0;

    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取当前使用的音频播放设备ID
     * @param [out] device_id
     *        设备ID，使用方负责按 MAX_DEVICE_ID_LENGTH 大小，分配与释放内存
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int GetCurrentPlaybackDevice(char device_id[MAX_DEVICE_ID_LENGTH]) = 0;
    
    /**
     * @type api
     * @region 音频设备管理
     * @brief 获取全部的音频播放设备信息
     * @param [out] info
     *        返回音频设备信息的数组首地址
     * @param [out] len
     *        数组长度
     * @return  方法调用结果
     *        + 0：方法调用成功
     *        + <0：方法调用失败
     */
    virtual int GetAllPlaybackDevices(AudioDeviceInfo* info, int len) = 0;

    /**
     * @type api
     * @region 自定义音频采集渲染
     * @brief 启用外部音频采集渲染。
     * @param record_sample_rate 外部音频采集采样率
     * @param record_channels 外部音频采集声道
     * @param playout_sample_rate 外部音频渲染采样率
     * @param playout_channels 外部音频渲染声道
     * @notes  <br>
     *      + 该方法应该在 JoinRoom{@link #JoinRoom} 前使用。  <br>
     *      + 启用自定义音频采集渲染后，使用 PushExternalAudioFrame{@link #PushExternalAudioFrame} 推送音频数据。  <br>
     *      + 启用自定义音频采集渲染后，使用 PullExternalAudioFrame{@link #PullExternalAudioFrame} 拉取音频数据。  <br>
     */
    virtual void EnableExternalAudioDevice(unsigned int record_sample_rate,
            unsigned int record_channels, unsigned int playout_sample_rate, unsigned int playout_channels) = 0;
    
    /**
     * @type api
     * @region 自定义音频采集渲染
     * @brief 禁用外部音频采集渲染。
     * @notes  <br>
     *      + 该方法应该在 JoinRoom{@link #JoinRoom} 前使用。  <br>
     *      + 启用自定义音频采集请使用 EnableExternalAudioDevice{@link #EnableExternalAudioDevice}。  <br>
     */
    virtual void DisableExternalAudioDevice() = 0;
    
    /**
     * @type api
     * @region 自定义音频采集渲染
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
     * @brief 析构函数
     */
    virtual ~IGameRTCEngine() {}
};

} // namespace gamertc
} // namespace bytertc