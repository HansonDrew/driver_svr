#pragma once

#include <memory>
#include "ParfaitConstants.h"
#include "ParfaitEnvBase.h"
#include "ParfaitLogRecorderBase.h"
#include "ParfaitRecorderBase.h"
#include "ParfaitUserDefinedRecorderBase.h"
namespace parfait {

struct JankUploadRequest {
    const unsigned int struct_size = sizeof(JankUploadRequest);
    /**
     * 返回dump文件上传结果，回调是在子线程
     * @param dump_path 本次上传文件的路径
     * @param is_success 上传是否成功
     */
    void (*result)(const char* dump_path, bool is_success) = nullptr;
    const char* dump_path = nullptr;          // 必填，dmp文件绝对路径
    uint64_t thread_id = 0;                   // 必填，卡顿thread dmp id
    const char *thread_name = nullptr;        // 卡顿thread名，不填默认为空
    long long duration = -1;                  // 卡顿时长，单位s，不填默认为unkown
    long long timestamp_s = 0;                // 卡顿发生时间戳，单位s，不填默认unknown
    const char* process_name = nullptr;       // 卡顿进程名，不填默认为当前实例的进程名
    const char* app_version = nullptr;        // 卡顿版本号，不填默认为当前版本号
    const char* app_minor_version = nullptr;  // 卡顿小版本号，不填默认为当前小版本号
    const char* build_id = nullptr;           // 卡顿build_id，不填默认为当前build_id
    const char* session_id = nullptr;         // 卡顿session_id，不填默认为空
    const char* scene = nullptr;              // 卡顿场景，不填默认为空
};

class ParfaitWrapperBase final {
  public:
    class ParfaitWrapperBaseImpl;

    ParfaitWrapperBase(std::unique_ptr<ParfaitWrapperBaseImpl> impl);

    ~ParfaitWrapperBase();
    
    /**
     * 全局初始化
     * @param global_env_builder CreateParfaitGlobalEnvBuilder指向的全局环境变量builder
     */
    PARFAIT_API void InitGlobalEnv(ParfaitGlobalEnvBuilderBase& global_env_builder);

    /**
     * 实例初始化
     * @param env_builder  CreateParfaitEnvBuilder指向的实例环境变量builder
     */
    PARFAIT_API void InitInstanceEnv(ParfaitEnvBuilderBase& env_builder);
    
    /**
     * 重置全局初始化，不保证内存可见性，但是是线程安全的
     * @param reset_global_env_builder 会将所有配置全部覆盖
     */
    PARFAIT_API void ResetGlobalEnv(ParfaitGlobalEnvBuilderBase& reset_global_env_builder);

    /**
     * 触发日志和指标数据的班车上传
     */
    PARFAIT_API void Upload();

    /* 旧接口，废弃！请使用CreateRecorder */
    PARFAIT_API attribute_deprecated ParfaitRecorderBase& StartRecord(enum RECORD_INTERVAL interval,
                                                                      const char* service_name);
    
    /**
     * 新接口，生成‘自定义事件’的recorder，业务方可用返回的recorder写自定义事件
     * @param interval 数据flush间隔，flush越短越快被上报
     * @param service_name 自定义事件名称
     * @return ‘自定义事件’的recorder指针
     */
    PARFAIT_API ParfaitRecorderBase* CreateRecorder(enum RECORD_INTERVAL interval, const char* service_name);
    
    /**
     * 新接口，回收‘自定义事件’的recorder
     * @param recorder CreateRecorder返回的指针
     */
    PARFAIT_API void DestroyRecorder(ParfaitRecorderBase* recorder);

    /* 旧接口，废弃！请使用CreateLogRecorder */
    PARFAIT_API attribute_deprecated ParfaitLogRecorderBase& StartLogRecord(enum RECORD_INTERVAL interval,
                                                                            enum LOG_RECORD_LEVEL log_level);
    
    /**
     * 新接口，生成‘自定义日志’的log recorder，业务方可用返回的log recorder写自定义日志
     * @param interval 数据flush间隔，flush越短越快被上报
     * @param log_level 日志等级
     * @return ‘自定义日志’的log recorder指针
     */
    PARFAIT_API ParfaitLogRecorderBase* CreateLogRecorder(enum RECORD_INTERVAL interval, enum LOG_RECORD_LEVEL log_level);
    
    /**
     * 新接口，回收‘自定义日志’的log recorder
     * @param recorder CreateLogRecorder返回的指针
     */
    PARFAIT_API void DestroyLogRecorder(ParfaitLogRecorderBase* recorder);
    
    /**
     * 生成‘自定义type数据’的log recorder，业务方可用返回的recorder写自定义type数据
     * @param interval 数据flush间隔，flush越短越快被上报
     * @param type 数据类型，需要提前和Slardar PC服务端协议，禁止使用预设字段，详见接入文档
     * @return ‘自定义type数据’的recorder指针
     */
    PARFAIT_API ParfaitUserDefinedRecorderBase* CreateUserDefinedRecorder(enum RECORD_INTERVAL interval,
                                                                          const char* type);
    
    /**
     * 回收‘自定义type数据’的recorder
     * @param recorder CreateUserDefinedRecorder返回的指针
     */
    PARFAIT_API void DestroyUserDefinedRecorder(ParfaitUserDefinedRecorderBase* recorder);

    /**
     * 启动pv/uv数据上报，用于统计pv/uv以及崩溃影响用户数，初始化完成后立即上报数据
     */
    PARFAIT_API void LaunchReport();
    
    /**
     * 即刻触发一次flush+upload指标&日志数据
     */
    PARFAIT_API void UploadWithFlushImmediately();
//----------------------------------------------性能监控 相关----------------------------------------------
    /**
     * 设置性能数据的对应场景，可随时调用该接口修改场景
     * @param scene 场景信息
     */
    PARFAIT_API void SetPerformanceScene(const char* scene);
    /**
     * 启动本进程cpu性能监控。如果想要监控其他进程，请使用带pid的方法
     */
    PARFAIT_API void StartCPUMonitor();
    /**
     * 启动cpu性能监控
     * @param pid 进程pid。windows和Linux支持pid非本进程，mac情况不支持多进程且不能使用该接口，请使用无参数接口
     * @param process_name pid所绑定的进程标识，也是slardar上所展示时使用的标识
     */
    PARFAIT_API void StartCPUMonitor(int pid, const char* process_name);
    /**
     * 启动本进程cpu性能监控。如果想要监控其他进程，请使用带pid的方法
     */
    PARFAIT_API void StartMemoryMonitor();
    /**
     * 启动memory性能监控
     * @param pid 进程pid。windows和Linux支持pid非本进程，mac情况不支持多进程且不能使用该接口，请使用无参数接口
     * @param process_name pid所绑定的进程标识，也是slardar上所展示时使用的标识
     */
    PARFAIT_API void StartMemoryMonitor(int pid, const char* process_name);
    /**
     *
     * 启动本进程的磁盘io性能监控。仅支持windows
     */
    PARFAIT_API void StartDiskIOMonitor();
    /**
     * 启动磁盘io性能监控
     * @param pid 进程pid。仅支持windows
     * @param process_name pid所绑定的进程标识，也是slardar上所展示时使用的标识
     */
    PARFAIT_API void StartDiskIOMonitor(int pid, const char* process_name);
    /**
     * 启动disk监控。windows监控的是parfait所在盘符的容量，Linux和mac监控的是根文件系统的容量
     */
    PARFAIT_API void StartDiskMonitor();


//-----------------------------------------Crash相关（仅支持PC端）----------------------------------------------
    /**
     * 仅Mac/Linux可用
     * 初始化Crashpad，并为调用进程注册Crash监听
     * 调用进程的子进程会自动注册Crash监听，不需要调用InitCrashClient方法
     * @param dump_dir crash文件存储路径，最后以‘/’结尾
     * @return Crashpad是否初始化成功
     */
    PARFAIT_API bool InitCrashServer(const char* dump_dir);

    /**
     * 仅Mac/Linux可用
     * 在parfait初始化前初始化Crashpad，参数和上面一致
     * 初始化parfait后仍需要调用上述的InitCrashServer，才能上传Crash文件
     * ⚠️应用生命周期内，Crashpad只会初始化一次，参数以第一次初始化为准
     */
    PARFAIT_API static bool InitCrashServerEarly(const char* dump_dir);

    /**
     * 仅Windows可用
     * ⚠️注意：必须在主进程的主线程中调用！！
     * 异步初始化Crashpad，并为调用进程注册Crash监听
     * @param handler_path crashpad_handler.exe的路径，最后以crashpad_handler.exe结尾
     * @param dump_dir crash文件存储路径，最后以‘\\’结尾
     * @param callback 返回异步初始化Crashpad的结果和其他进程调用InitCrashClientOnWin注册崩溃监听需要的ipc_pipe
     */
    PARFAIT_API void InitCrashServerOnWin(const char* handler_path, const char* dump_dir, CrashServerInitCallback callback);

    /**
     * 仅Windows可用
     * 在parfait初始化前初始化Crashpad，并为调用进程注册Crash监听
     * ⚠️注意：必须在主进程的主线程中调用！！Crashpad只会初始化一次，参数以第一次初始化为准!!
     * ⚠️注意：初始化parfait后仍需要调用上述的InitCrashServerOnWin，才能上传Crash文件
     * @param handler_path crashpad_handler.exe的路径，最后以crashpad_handler.exe结尾
     * @param dump_dir crash文件存储路径，最后以‘\\’结尾
     * @param async_start 异步初始化, true的话callback返回异步初始化结果，false该方法同步返回crashpad初始化结果
     * @param callback 返回异步初始化Crashpad结果和其他进程调用InitCrashClientOnWin注册崩溃监听需要的ipc_pipe
     */
    PARFAIT_API static bool InitCrashServerOnWinEarly(const char* handler_path, const char* dump_dir,
                                                      bool async_start, CrashServerInitCallback callback);

    /**
     * 仅Windows可用
     * 为当前进程注册Crash监听
     * @param ipc_pipe InitCrashServerOnWin的callback获得的ipc_pipe值
     */
    PARFAIT_API bool InitCrashClientOnWin(const char* ipc_pipe);

    /**
     * 仅Windows可用
     * 在parfait初始化前为当前进程注册Crash监听，参数和上面一致
     * ⚠️注意：调用本方法后不需要再调用上面的InitCrashClientOnWin方法
     */
    PARFAIT_API static bool InitCrashClientOnWinEarly(const char* ipc_pipe);
    
    /**
     * 三端通用
     * 发生异常后，允许parfait异常传递给之前的异常处理器
     * ⚠️注意：必须在注册崩溃监听前调用，也就是必须在InitCrashServer系列方法前调用
     */
    PARFAIT_API static void EnablePassExceptionToOriginalHandler();

    /**
     * 业务方返回JS栈的回调，Parfait会在发生崩溃后调用
     */
    typedef const char*(*CrashV8Callback)();
    /**
     * 仅Windows可用
     * 向Parfait注入JS栈回调，进程崩溃后上传JS栈
     * ⚠️注意：必须在注册崩溃监听前调用，也就是必须在InitCrashServer系列方法前调用
     */
    PARFAIT_API static void SetCrashClientJsStackCallback(const CrashV8Callback v8callback);

    /**
     * 仅Windows可用
     * Crashpad VEH默认捕获Heap Corruption类型异常，业务方可自定义设置其他需要捕获的异常类型
     * VEH捕获异常后会生成dmp文件，并将异常抛出
     * @param exception_type_code 异常的错误码，可在winnt.h中查询, e.g.STATUS_ACCESS_VIOLATION/0xC0000005L
     */
    PARFAIT_API static void AddVehTargetExceptionType(const unsigned long exception_type_code);
    
    struct CrashAnnotation {
        const char* scene = nullptr; // 当前场景
    };
    
    /**
     * 三端可用，卡顿/崩溃后parfait crashpad会自动带上crash annotation字段信息，所有进程共享字段，可随时调用
     * e.g. 应用刚启动，设置scene为launch
     */
    PARFAIT_API static void SetCrashAnnotation(const CrashAnnotation* annotation);
    
    struct UploadCrashDumpCallback {
    	virtual ~UploadCrashDumpCallback() = default;
    	virtual void result(bool is_success) = 0;
    };

    /* ⚠️废弃，请使用UploadCrashFile接口 */
    PARFAIT_API attribute_deprecated void UploadCrashDumpFile(const char* dump_file,
                                                              long crash_time,
                                                              const UploadCrashDumpCallback* callback);
    
    struct CrashUploadRequest {
        const unsigned int struct_size = sizeof(CrashUploadRequest);
    
        /**
         * 返回dump文件上传结果，回调是在子线程
         * @param dump_path 本次上传文件的路径
         * @param is_success 上传是否成功
         */
        virtual void result(const char* dump_path, bool is_success) = 0;
        const char* dump_path = nullptr;          // 必填，dmp文件绝对路径
        long long crashtime_s = 0;                // 崩溃时间，单位s，不填默认unknown
        const char* process_name = nullptr;       // 崩溃进程名，不填默认为当前实例的进程名
        const char* app_version = nullptr;        // 崩溃版本号，不填默认为当前版本号
        const char* app_minor_version = nullptr;  // 崩溃小版本号，不填默认为当前小版本号
        const char* build_id = nullptr;           // 崩溃build_id，不填默认为当前build_id
        const char* session_id = nullptr;         // 崩溃session_id，不填默认为空
        const char* crash_scene = nullptr;        // 崩溃场景，不填默认为空
    };
    
    /**
     * 上报崩溃.dmp文件到slardar，可同步调用，上传结果通过result方法异步返回
     * @param request 请求参数，具体细节参考上面的结构体说明
     */
    PARFAIT_API void UploadCrashFile(const CrashUploadRequest* request);

//-----------------------------------------卡顿相关（仅支持PC端）----------------------------------------------
    /**
     * 暂时仅Windows可用
     * 业务检测到卡顿场景后，调用此接口捕获卡顿堆栈生成.dmp文件并立即上传到slardar，异步接口
     * ⚠️注意：必须在崩溃监听初始化成功后调用，依赖于parfait的crashpad版本
     * @param thread_id 必传，卡顿真实线程id，Parfait自动转换真实tid为dmp id，平台依赖dmp tid做聚合
     * @param thread_name 选传，卡顿线程名，展示在卡顿详情页
     * @param duration_s 选传，卡顿时长，单位秒，不确定填-1，展示在卡顿详情页
     */
    PARFAIT_API void DumpJankAndUploadImmediately(const uint64_t thread_id, const char* thread_name, long long duration_s);

    /**
     * 上报卡顿.dmp文件到slardar，可同步调用，上传结果通过result方法异步返回
     * @param request 请求参数，具体细节参考上面的结构体说明
     */
    PARFAIT_API void UploadJankFile(const JankUploadRequest request);

    //----------------------------------------------ALog + 回捞相关----------------------------------------------
    /**
     * 开启云控回捞功能
     * @param interval 轮询频率，单位ms，不设置默认30s
     */
    PARFAIT_API void OpenCloudMessage(long interval = 30 * 1000);

    /**
     * 初始化alog
     * @param alog_dir alog存储路径
     * @param process_name 进程名
     */
    PARFAIT_API void InitAlog(const char* alog_dir, const char* process_name);

    /**
     * 写Alog日志，level 为 INFO
     * @param tag tag内容
     * @param msg 日志内容
     */
    PARFAIT_API void WriteAlog(const char* tag, const char* msg);

    /**
     * 写Alog日志
     * @param level 日志 level
     * @param tag tag内容
     * @param msg 日志内容
     */
    PARFAIT_API void WriteAlog(enum ALOG_LEVEL level, const char* tag,
                               const char* msg);

    /**
     * 主动上报时间窗口内的alog日志。业务方可以同步调用，函数会异步操作
     * @param start_time 开始时间戳，精确到秒
     * @param end_time 结束时间戳，精确到秒
     */
    PARFAIT_API void UploadAlog(long start_time, long end_time);
    
    /**
     * 三端通用
     * 开启后，业务方发生崩溃/上报崩溃时，parfait自动上传崩溃前半小时的alog文件
     */
    PARFAIT_API void UploadAlogIfCrashed();

//----------------------------------------------网络监控----------------------------------------------
    /**
     * 上报网络日志-回调方式
     * 注入monitor回调监控整个ttnet engine
     * 因Native接口问题暂未支持本方法， 将来支持
     * @param engine_ptr 需要监控的ttnet engine实例
     */
    PARFAIT_API void InjectNetworkMonitor(void* engine_ptr);

    /**
     * 上报网络日志-侵入方式
     * 在 OnSucceeded、OnFailed、OnCanceled回调中调用该函数，需要侵入业务代码
     * @param request_ptr ttnet网络请求
     * @param response_ptr ttnet网络返回
     */
    PARFAIT_API void UploadNetworkLog(void* request_ptr, void* response_ptr);

     /**
      * 上报网络日志
      * 与ttnet解耦，无论业务方选择 1.Parfait内置ttnet上报方式 2.业务方注入ttnet方式 3.业务方自己实现Uploader ，都可以用该接口
      * @param url 网络请求链接
      * @param status_code 网络请求的status code
      * @param request_log 需要上报的请求日志：需要符合ttnet日志格式
     */
     PARFAIT_API void UploadNetworkLog(const char* url, const int status_code, const char* request_log);
    
//----------------------------------------------其他功能----------------------------------------------

    struct UploadFileCallback {
        virtual ~UploadFileCallback() = default;
        /**
         * 返回上传结果，回调是在子线程
         * @param is_success 上传文件是否成功
         */
        virtual void result(bool is_success) = 0;
    };
    /**
     * 主动将业务方的任意文件上传到slardar-日志文件查询中
     * @param file_path 文件所在路径
     * @param callback 异步回调
     */
    PARFAIT_API void UploadFile(const char* file_path, const UploadFileCallback* callback);
    
  private:
    std::unique_ptr<ParfaitWrapperBaseImpl> impl_;
};
} //namespace parfait
