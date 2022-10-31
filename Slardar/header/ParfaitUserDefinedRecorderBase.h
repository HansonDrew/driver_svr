//
//  ParfaitUserDefinedRecorder.h
//  parfait
//
//  Created by kilroy on 2022/3/1.
//

#pragma once

#include <memory>
#include "ParfaitConstants.h"

/**
 * 写入自定义type数据的公用接口
 * type值需要先和服务端对齐
 */

namespace parfait {
class ParfaitUserDefinedRecorderBase final {
  public:
    
    class ParfaitUserDefinedRecorderBaseImpl;
    
    ParfaitUserDefinedRecorderBase(std::unique_ptr<ParfaitUserDefinedRecorderBaseImpl> impl);

    ~ParfaitUserDefinedRecorderBase();
    
    PARFAIT_API ParfaitUserDefinedRecorderBase& WriteJsonData(const char* json_data);

    PARFAIT_API void DoRecord();
    
  private:
    std::unique_ptr<ParfaitUserDefinedRecorderBaseImpl> impl_;
};
} //namespace parfait
