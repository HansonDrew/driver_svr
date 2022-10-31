#pragma once

#include "ParfaitWrapperBase.h"
#include "ParfaitEnvBase.h"
#include "ParfaitConstants.h"

namespace parfait {

#if defined(WIN32) || defined(_WIN32)
extern "C" {
#endif

PARFAIT_API ParfaitWrapperBase* CreateParfaitWrapper();

PARFAIT_API void DestroyParfaitWrapper(ParfaitWrapperBase* &wrapper);

PARFAIT_API ParfaitEnvBuilderBase* CreateParfaitEnvBuilder(int64_t aid, const char* instance_name);

PARFAIT_API void DestroyParfaitEnvBuilder(ParfaitEnvBuilderBase* &builder);

PARFAIT_API ParfaitGlobalEnvBuilderBase* CreateParfaitGlobalEnvBuilder(int64_t aid);

PARFAIT_API void DestroyParfaitGlobalEnvBuilder(ParfaitGlobalEnvBuilderBase* &builder);

#if defined(WIN32) || defined(_WIN32)
}
#endif

} //namespace parfait
