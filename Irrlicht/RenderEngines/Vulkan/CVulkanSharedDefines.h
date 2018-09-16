#ifndef __CVULKAN_SHADER_DEFINES_H__
#define __CVULKAN_SHADER_DEFINES_H__

#include "CCommonDefines.h"
#include "standard/enum.h"

namespace irr
{
    namespace video
    {
        /** Flags that determine how is a resource being used by the GPU. */
        enum class VulkanUseFlag : unsigned char
        {
            eNone = 0,
            eRead = 0x1,
            eWrite = 0x2
        };

        ENABLE_ENUM_CLASS_FLAG(VulkanUseFlag)
    }
}

#endif // __CVULKAN_SHADER_DEFINES_H__