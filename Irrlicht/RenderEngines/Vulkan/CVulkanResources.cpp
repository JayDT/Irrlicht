#include "CVulkanResources.h"
#include "CVulkanDriver.h"

using namespace irr;
using namespace irr::video;

irr::video::CVulkanDeviceResource::CVulkanDeviceResource(CVulkanDriver * driver)
    : Driver(driver)
    , mUsedFrameID(driver->GetFrameID() + 1)
{
    mQueueFamily = -1;
    mState = State::Normal;
    mNumUsedHandles = 0;
    mNumBoundHandles = 0;

    memset(mReadUses, 0, sizeof(mReadUses));
    memset(mWriteUses, 0, sizeof(mWriteUses));

    driver->AddResourceObject(this);
}

irr::video::CVulkanDeviceResource::~CVulkanDeviceResource()
{
    //std::unique_lock<std::mutex> lock(mMutex);

    Driver->RemoveResourceObject(this);
}

void irr::video::CVulkanDeviceResource::notifyBound()
{
    //std::unique_lock<std::mutex> lock(mMutex);
    assert(mState != State::Destroyed);

    grab();
    mNumBoundHandles++;
}

void irr::video::CVulkanDeviceResource::notifyUsed(u32 globalQueueIdx, u32 queueFamily, VulkanUseFlag useFlags)
{
    //std::unique_lock<std::mutex> lock(mMutex);
    assert(useFlags != VulkanUseFlag::eNone);

    bool isUsed = mNumUsedHandles > 0;
    if (isUsed && mState == State::Normal) // Used without support for concurrency
    {
        assert(mQueueFamily == queueFamily &&
            "Vulkan resource without concurrency support can only be used by one queue family at once.");
    }

    mNumUsedHandles++;
    mQueueFamily = queueFamily;

    assert(globalQueueIdx < MAX_UNIQUE_QUEUES);

    if (useFlags & VulkanUseFlag::eRead)
    {
        assert(mReadUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
        mReadUses[globalQueueIdx]++;
    }

    if (useFlags & VulkanUseFlag::eWrite)
    {
        assert(mWriteUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
        mWriteUses[globalQueueIdx]++;
    }
}

void irr::video::CVulkanDeviceResource::notifyDone(u32 globalQueueIdx, VulkanUseFlag useFlags)
{
    if (mNumBoundHandles == 0 && mNumUsedHandles == 0)
        return;

    //bool destroy;
    {
        //std::unique_lock<std::mutex> lock(mMutex);
        mNumUsedHandles--;
        mNumBoundHandles--;

        if (useFlags & VulkanUseFlag::eRead)
        {
            assert(mReadUses[globalQueueIdx] > 0);
            mReadUses[globalQueueIdx]--;
        }

        if (useFlags & VulkanUseFlag::eWrite)
        {
            assert(mWriteUses[globalQueueIdx] > 0);
            mWriteUses[globalQueueIdx]--;
        }

        bool isBound = mNumBoundHandles > 0;
        //destroy = !isBound && mState == State::Destroyed; // Queued for destruction
    }

    drop();

    // (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
    //if (destroy)
    //    mOwner->destroy(this);
}

void irr::video::CVulkanDeviceResource::notifyUnbound()
{
    if (mNumBoundHandles == 0)
        return;

    {
        //std::unique_lock<std::mutex> lock(mMutex);
        mNumBoundHandles--;

        //bool isBound = mNumBoundHandles > 0;
        //destroy = !isBound && mState == State::Destroyed; // Queued for destruction
    }

    drop();

    // (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
    //if (destroy)
    //    mOwner->destroy(this);
}


/**
* Checks is the resource currently bound to any command buffer.
*
* @note	Resource usage is only checked at certain points of the program. This means the resource could be
*			done on the device but this method may still report true. If you need to know the latest state
*			call VulkanCommandBufferManager::refreshStates() before checking for usage.
*/

bool irr::video::CVulkanDeviceResource::isBound() const
{
    /*std::unique_lock<std::mutex> lock(mMutex);*/
    return mNumBoundHandles > 0 || mUsedFrameID == Driver->GetFrameID();
}

u32 irr::video::CVulkanDeviceResource::getUseInfo(VulkanUseFlag useFlags) const
{
    u32 mask = 0;
    if (!mUseHandle.used)
        return mask;

    if (useFlags & VulkanUseFlag::eRead)
    {
        for (u32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
        {
            if (mReadUses[i] > 0)
                mask |= 1 << i;
        }
    }

    if (useFlags & VulkanUseFlag::eWrite)
    {
        for (u32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
        {
            if (mWriteUses[i] > 0)
                mask |= 1 << i;
        }
    }

    return mask;
}

void irr::video::CVulkanDeviceResource::NotifySoftBound(VulkanUseFlag flags)
{
    mUsedFrameID = Driver->GetFrameID();

    if (~mUseHandle.mPoolMask & 1)
    {
        mUseHandle.used = false;
        mUseHandle.flags = flags;
        assert(mUseHandle.flags != VulkanUseFlag::eNone);
    }
    else
    {
        mUseHandle.flags |= flags;
    }
}
