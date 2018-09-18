#ifndef __C_LVK_MANAGER_H_INCLUDED__
#define __C_LVK_MANAGER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_

#include "CVulkanPlatform.h"

namespace irr
{
namespace video
{
    class CLinuxVulkanPlatform : public CVulkanPlatform
    {
    public:
        //! Constructor.
        CLinuxVulkanPlatform(CVulkanDriver* driver, Display* dpy)
            : CVulkanPlatform(driver)
            , mDisplay(dpy)
        {}

		//! Destructor
        virtual ~CLinuxVulkanPlatform();

        // Inherited via CVulkanPlatform
        virtual void initialize() override;
        virtual void resizeSwapBuffers() override;

    protected:

        Display* mDisplay = nullptr;
        bool mIsFullScreen = false;

    };
}
}

#endif // _IRR_COMPILE_WITH_X11_DEVICE_

#endif
