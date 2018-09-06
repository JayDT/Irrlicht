#include "CWinPlatform.h"
#include "CVulkanDriver.h"
#include "CVulkanDevice.h"
#include "CVulkanSwapChain.h"

//! Destructor

irr::video::CWinVulkanPlatform::~CWinVulkanPlatform()
{
}

void irr::video::CWinVulkanPlatform::initialize()
{
    // get handle to exe file
    HINSTANCE hInstance = GetModuleHandle(0);

    // Create Vulkan surface
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hwnd = (HWND)mDriver->mCreateParams.WindowId;
    surfaceCreateInfo.hinstance = hInstance;

    VkInstance instance = mDriver->_getInstance();
    VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, VulkanDevice::gVulkanAllocator, &mSurface);
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
            DEVMODE displayDeviceMode;

            memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
            displayDeviceMode.dmSize = sizeof(DEVMODE);
            displayDeviceMode.dmBitsPerPel = 32;
            displayDeviceMode.dmPelsWidth = mDriver->getScreenSize().Width;
            displayDeviceMode.dmPelsHeight = mDriver->getScreenSize().Height;
            displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            //if (mDisplayFrequency)
            //{
            //    displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
            //    displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
            //
            //    if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
            //    {
            //        assert("ChangeDisplaySettings with user display frequency failed.");
            //    }
            //}

            if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
            {
                assert("ChangeDisplaySettings failed.");
            }
        }
    }

    // Create swap chain
    mSwapChain = new VulkanSwapChain();
    mSwapChain->rebuild(presentDevice, mSurface, mDriver->getScreenSize().Width, mDriver->getScreenSize().Height, mDriver->mCreateParams.Vsync, mColorFormat, mColorSpace, true, mDepthFormat);
}

void irr::video::CWinVulkanPlatform::resizeSwapBuffers()
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
            DEVMODE displayDeviceMode;

            memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
            displayDeviceMode.dmSize = sizeof(DEVMODE);
            displayDeviceMode.dmBitsPerPel = 32;
            displayDeviceMode.dmPelsWidth = mDriver->getScreenSize().Width;
            displayDeviceMode.dmPelsHeight = mDriver->getScreenSize().Height;
            displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            //if (mDisplayFrequency)
            //{
            //    displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
            //    displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
            //
            //    if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
            //    {
            //        assert("ChangeDisplaySettings with user display frequency failed.");
            //    }
            //}

            if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
            {
                assert("ChangeDisplaySettings failed.");
            }
        }
    }

    mDriver->_getPrimaryDevice()->waitIdle();
    mSwapChain->rebuild(mDriver->_getPrimaryDevice(), mSurface, mDriver->getScreenSize().Width, mDriver->getScreenSize().Height, mDriver->mCreateParams.Vsync, mColorFormat, mColorSpace, true, mDepthFormat);
}
