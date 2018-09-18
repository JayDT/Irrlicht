#include "CLinuxPlatform.h"
#include "CVulkanDriver.h"
#include "CVulkanDevice.h"
#include "CVulkanSwapChain.h"

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_

//! Destructor

irr::video::CLinuxVulkanPlatform::~CLinuxVulkanPlatform()
{
}

void irr::video::CLinuxVulkanPlatform::initialize()
{
    // Create Vulkan surface
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = (Window)mDriver->mCreateParams.WindowId;
    surfaceCreateInfo.dpy = mDisplay;

    VkInstance instance = mDriver->_getInstance();
    VkResult result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, VulkanDevice::gVulkanAllocator, &mSurface);
    assert(result == VK_SUCCESS);

    VulkanDevice* presentDevice = mDriver->_getPrimaryDevice();
    VkPhysicalDevice physicalDevice = presentDevice->getPhysical();

    mPresentQueueFamily = presentDevice->getQueueFamily(GQT_GRAPHICS);

    VkBool32 supportsPresent;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mPresentQueueFamily, mSurface, &supportsPresent);

    if (!supportsPresent)
    {
        // Note: Not supporting present only queues at the moment
        // Note: Also present device can only return one family of graphics queue, while there could be more (some of
        // which support present)
        throw std::runtime_error("Cannot find a graphics queue that also supports present operations.");
    }

    SurfaceFormat format = presentDevice->getSurfaceFormat(mSurface, mDriver->mCreateParams.Stencilbuffer, false);
    mColorFormat = format.colorFormat;
    mColorSpace = format.colorSpace;
    mDepthFormat = format.depthFormat;

    // Make the window full screen if required
    if (mDriver->mCreateParams.Fullscreen)
    {
        if (!mIsFullScreen)
        {
        }
    }

    // Create swap chain
    mSwapChain = new VulkanSwapChain();
    mSwapChain->rebuild(presentDevice, mSurface, mDriver->getScreenSize().Width, mDriver->getScreenSize().Height, mDriver->mCreateParams.Vsync, mColorFormat, mColorSpace, true, mDepthFormat);
}

void irr::video::CLinuxVulkanPlatform::resizeSwapBuffers()
{
    // Resize swap chain

    //// Need to make sure nothing is using the swap buffer before we re-create it
    // Note: Optionally I can detect exactly on which queues (if any) are the swap chain images used on, and only wait
    // on those

    // Make the window full screen if required
    if (mDriver->mCreateParams.Fullscreen)
    {
        if (!mIsFullScreen)
        {
        }
    }

    mDriver->_getPrimaryDevice()->waitIdle();
    mSwapChain->rebuild(mDriver->_getPrimaryDevice(), mSurface, mDriver->getScreenSize().Width, mDriver->getScreenSize().Height, mDriver->mCreateParams.Vsync, mColorFormat, mColorSpace, true, mDepthFormat);
}

#endif // _IRR_COMPILE_WITH_X11_DEVICE_