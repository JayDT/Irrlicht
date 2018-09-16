#include "CVulkanPlatform.h"
#include "CVulkanUtility.h"
#include "CVulkanDriver.h"
#include "CVulkanSwapChain.h"
#include "CVulkanQueue.h"
#include "CVulkanDevice.h"

///// https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/4-Using-spir-v-with-spirv-cross.pdf
///// https://forums.khronos.org/showthread.php/13099-SPIRV-automatic-uniform-detection
///// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkShaderModule.html

irr::video::CVulkanPlatform::CVulkanPlatform(CVulkanDriver * driver)
    : mDriver(driver)
{
}

irr::video::CVulkanPlatform::~CVulkanPlatform()
{
    if (mSwapChain)
        delete mSwapChain;
    mSwapChain = nullptr;
}

void irr::video::CVulkanPlatform::acquireBackBuffer()
{
    // We haven't presented the current back buffer yet, so just use that one
    if (!mRequiresNewBackBuffer)
        return;

    mSwapChain->acquireBackBuffer();
    mRequiresNewBackBuffer = false;
}

void irr::video::CVulkanPlatform::swapBuffers(std::uint32_t syncMask)
{
    //if (mShowOnSwap)
    //    setHidden(false);

    // Get a command buffer on which we'll submit
    VulkanDevice* presentDevice = mDriver->_getPrimaryDevice();

    // Assuming present queue is always graphics
    assert(presentDevice->getQueueFamily(GQT_GRAPHICS) == mPresentQueueFamily);

    // Find an appropriate queue to execute on
    VulkanQueue* queue = presentDevice->getQueue(GQT_GRAPHICS, 0);
    std::uint32_t queueMask = presentDevice->getQueueMask(GQT_GRAPHICS, 0);

    // Ignore myself
    syncMask &= ~queueMask;

    std::uint32_t deviceIdx = presentDevice->getIndex();

    std::uint32_t numSemaphores;
    mDriver->getSyncSemaphores(deviceIdx, syncMask, mSemaphoresTemp, numSemaphores);

    // Wait on present (i.e. until the back buffer becomes available), if we haven't already done so
    const SwapChainSurface& surface = mSwapChain->getBackBuffer();
    if (surface.needsWait)
    {
        mSemaphoresTemp[numSemaphores] = mSwapChain->getBackBuffer().sync;
        numSemaphores++;

        mSwapChain->notifyBackBufferWaitIssued();
    }

    queue->present(mSwapChain, mSemaphoresTemp, numSemaphores);
    mRequiresNewBackBuffer = true;

    VulkanDevice* device = presentDevice;
    if (device)
    {
        for (std::uint32_t i = 0; i < GQT_COUNT; i++)
        {
            std::uint32_t numQueues = device->getNumQueues((GpuQueueType)i);
            for (std::uint32_t j = 0; j < numQueues; j++)
            {
                VulkanQueue* queue = device->getQueue((GpuQueueType)i, j);
                queue->refreshStates(true, false);
            }
        }
    }
}
