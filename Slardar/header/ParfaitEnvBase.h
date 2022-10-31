#pragma once
#include <stdint.h>
#include <memory>

#include "ParfaitConstants.h"

namespace parfait {

/**
 * Windows注册Crash监听的回调，获取初始化结果和子进程注册崩溃监听需要的ipc_pipe值
 * @param init_res 是否成功初始化Crashpad
 * @param ipc_pipe InitCrashClientOnWin时需要传入的值
 */
using CrashServerInitCallback = void (*)(bool init_res, const char* ipc_pipe);

class ParfaitEnvBuilderBase {
  public:
    class ParfaitEnvBuilderBaseImpl;
    
    struct ResponseStruct {
        virtual ~ResponseStruct() = default;

        int error = -1;
        int code = -1;
        char* response_data = nullptr;
        int64_t len;
    };

    struct IUploader {
        virtual ~IUploader() = default;

        /**
         * 回调是在子线程，业务方需要同步进行网络请求并返回网络请求结果
         * @param url 上传url
         * @param request_data body值
         * @param len body长度
         * @param header_content_type Content-Type头的值
         * @param header_content_encoding Content-Encoding头的值
         * @return ParfaitEnvBuilderBase::ResponseStruct 返回的上传结果，code为http status code
         */
        virtual ParfaitEnvBuilderBase::ResponseStruct upload(const char* url,
                                                             const char* request_data,
                                                             int64_t len,
                                                             const char* header_content_type,
                                                             const char* header_content_encoding) = 0;
    };

    struct IPcCrashCallback {
        virtual ~IPcCrashCallback() = default;

        /**
         * 回调是在子线程，业务可以同步进行网络数据上报
         * @param url 上报的url
         * @param crash_info_json parfait生成的crash上报所需信息json数据
         * @param file_upload_key crash文件上传的key
         * @param dump_file_path crash文件的路径
         * @param header_content_type 上报文件的方式，multipart/form-data
         * @param len crash_json的长度
         */
        virtual ParfaitEnvBuilderBase::ResponseStruct OnCrashUpload(const char* url,
                                                                    const char* crash_info_json,
                                                                    const char* file_upload_key,
                                                                    const char* dump_file_path,
                                                                    const char* header_content_type,
                                                                    int64_t len) = 0;
    };
    
    ParfaitEnvBuilderBase(std::unique_ptr<ParfaitEnvBuilderBaseImpl> impl);

    virtual ~ParfaitEnvBuilderBase();
    
    /**
     * 设置进程号
     * @param pid 进程号
     */
    PARFAIT_API ParfaitEnvBuilderBase& SetPid(const char* pid);
    /**
     * 设置进程名
     * @param process_name 进程名
     */
    PARFAIT_API ParfaitEnvBuilderBase& SetProcessName(const char* process_name);
    /**
     * （可选，不推荐）传入日志和指标的上报实现，日志和指标上报时调用此uploader的upload方法
     * @param uploader 业务方自己实现的uploader
     */
    PARFAIT_API ParfaitEnvBuilderBase& SetUploader(ParfaitEnvBuilderBase::IUploader* uploader);
    /**
     * （可选，不推荐）传入crash的上报实现，crash上报时调用此callback的OnCrashUpload方法
     * @param crash_callback 业务方自己实现的crash callback
     */
    PARFAIT_API ParfaitEnvBuilderBase& SetPcCrashCallback(ParfaitEnvBuilderBase::IPcCrashCallback* crash_callback);
    /**
     * 添加自定义事件/日志的Context信息，用于单点展示以及事件过滤
     * @param key 键
     * @param value 值
     */
    PARFAIT_API ParfaitEnvBuilderBase& AddRecordContext(const char* key, const char* value);
    /**
     * 添加崩溃的Context信息，用于单点展示以及崩溃列表页面的过滤
     * @param key 键
     * @param value 值
     */
    PARFAIT_API ParfaitEnvBuilderBase& AddCrashContext(const char* key, const char* value);
    
  protected:
    std::unique_ptr<ParfaitEnvBuilderBaseImpl> impl;
};

class ParfaitGlobalEnvBuilderBase {
  public:
    class ParfaitGlobalEnvBuilderBaseImpl;
    
    ParfaitGlobalEnvBuilderBase(std::unique_ptr<ParfaitGlobalEnvBuilderBaseImpl> impl);
    
    virtual ~ParfaitGlobalEnvBuilderBase();
    /**
     * （可选）是否输出parfait内部的debug log，未设置默认为不输出
     * @param is_debug true输出，false不输出
     */
    PARFAIT_API static void SetIsDebug(bool is_debug);
    /**
     * （可选）是否是非中国应用，未设置默认为中国应用
     * @param is_overseas true走海外域名，false走国内域名
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetIsOverseas(bool is_overseas);
    /**
     * 设置存储文件的根路径，最后以‘/’结尾
     * @param root_path_name 存储文件的根路径
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetRootPathName(const char* root_path_name);
    /**
     * 设置Device ID
     * @param did device_id
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetDid(const char* did);
    /**
     * （可选）设置日志指标的单个存储文件最大的存储日志条数，未设置默认1000条
     * @param max_line 最大的存储日志条数
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetMaxLogNumber(int max_line);
    /**
     * （可选）设置日志指标的班车发车上报频率，未设置默认60s，最小值为1s
     * @param interval 上报的间隔时间，单位ms
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetReportInterval(int interval);
    /**
     * （可选）设置日志指标的单次网络上报最大容量，未设置默认100kb
     * @param max_size 单位byte
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetMaxReportSize(int max_size);
    /**
     * （可选）设置日志指标的单个存储文件最大的存储容量，未设置默认1mb
     * @param max_file_size 单位byte
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetMaxFileSize(int max_file_size);

    /**
     * 设置User ID
     * @param uid user_id
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetUid(const char* uid);
    /**
     * 设置app版本号
     * @param app_version app版本号
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetAppVersion(const char* app_version);
    /**
     * 设置app小版本号
     * @param minor_version app小版本号
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetAppMinorVersion(const char* minor_version);
    /**
     * 设置编译版本号
     * @param build_id Build id
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetBuildID(const char* build_id);
    /**
     * (可选) 如果业务使用了TTNet, 务必注入初始化后的Cronet_EnginePtr
     * 没注入Uploader/CrashCallback/Cronet_EnginePtr的情况下，parfait内部会初始化TTNet，用于上报数据
     * @param cornet_engine TTNet的Cronet_EnginePtr指针
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetTTNetEngine(const void* cornet_engine);
    /**
     * (可选) 设置应用channel
     * @param channel 渠道名称
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetChannel(const char* channel);
    /**
     * (可选) 设置session id
     * @param session_id session_id
     */
    PARFAIT_API ParfaitGlobalEnvBuilderBase& SetSessionID(const char* session_id);
    /**
     *（已废弃）1.1.6.0版本起不支持自定义上报url，数据全部上传到slardar，需要转发请联系后端同学
     * 此方法设置不生效！
     * @param report_url 自定义的日志和指标上报url
     */
    PARFAIT_API attribute_deprecated ParfaitGlobalEnvBuilderBase& SetReportUrl(const char* report_url);
    /**
     * （已废弃）1.1.9.0版本起parfait会自动获取系统版本号，业务设置的系统版本号不生效
     */
    PARFAIT_API attribute_deprecated ParfaitGlobalEnvBuilderBase& SetOsVersion(const char* os_version);
  protected:
    std::unique_ptr<ParfaitGlobalEnvBuilderBaseImpl> impl;
};

} //namespace parfait
