//
// Created by xuzhi on 2020/12/6.
//

#ifndef PARFAIT_PARFAITLOGRECORDERBASE_H
#define PARFAIT_PARFAITLOGRECORDERBASE_H

#include <memory>
#include "ParfaitConstants.h"

/**
 * 写入自定义日志的接口
 * 写入数据对应Slardar上'日志细查'-'自定义日志'
 */

namespace parfait {
class ParfaitLogRecorderBase final {
  public:
    class ParfaitLogRecorderBaseImpl;
    
    ParfaitLogRecorderBase(std::unique_ptr<ParfaitLogRecorderBaseImpl> impl, bool delete_by_self = true);

    ~ParfaitLogRecorderBase();
    
    PARFAIT_API ParfaitLogRecorderBase& WriteCategory(const char* json);

    PARFAIT_API ParfaitLogRecorderBase& WriteLog(const char* log);

    PARFAIT_API void DoRecord();
    
  private:
    std::unique_ptr<ParfaitLogRecorderBaseImpl> impl_;
    bool delete_by_self_ = true;
};
} //namespace parfait

#endif //PARFAIT_PARFAITLOGRECORDERBASE_H
