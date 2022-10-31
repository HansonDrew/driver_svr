#pragma once

#include <memory>
#include "ParfaitConstants.h"

/**
 * 写入自定义指标的接口
 * 写入数据对应Slardar上'日志细查'-'自定义事件'
 */

namespace parfait {
class ParfaitRecorderBase final {
  public:
    class ParfaitRecorderBaseImpl;
    
    ParfaitRecorderBase(std::unique_ptr<ParfaitRecorderBaseImpl> impl, bool delete_by_self = true);
    
    ~ParfaitRecorderBase();

    PARFAIT_API ParfaitRecorderBase& WriteCategory(const char* json);

    PARFAIT_API ParfaitRecorderBase& WriteMetric(const char* json);

    PARFAIT_API ParfaitRecorderBase& WriteExtra(const char* json);

    PARFAIT_API void DoRecord();
    
  private:
    std::unique_ptr<ParfaitRecorderBaseImpl> impl_;
    bool delete_by_self_ = true;
};
} //namespace parfait
