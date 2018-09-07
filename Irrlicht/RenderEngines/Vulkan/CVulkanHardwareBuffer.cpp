#include "IrrCompileConfig.h"

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE

#include "CVulkanDriver.h"
#include "CVulkanHardwareBuffer.h"
#include "CVulkanShader.h"
#include "CVulkanDevice.h"
#include "CVulkanTexture.h"
#include "CVulkanGpuPipelineState.h"
#include "CVulkanGpuParams.h"
#include "CVulkanTexture.h"
#include "os.h"

using namespace irr;
using namespace video;

static bool memory_type_from_properties(vk::PhysicalDeviceMemoryProperties const& memory_properties, uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex);

VulkanBuffer::VulkanBuffer(CVulkanDriver* owner, VkBuffer buffer, VkBufferView view, VmaAllocation allocation,
    uint32_t rowPitch, uint32_t slicePitch)
    : CVulkanDeviceResource(owner)
    , mBuffer(buffer)
    , mView(view)
    , mAllocation(allocation)
    , mRowPitch(rowPitch)
{
    if (rowPitch != 0)
        mSliceHeight = slicePitch / rowPitch;
    else
        mSliceHeight = 0;

    VulkanDevice* device = Driver->_getPrimaryDevice();
    
    device->getAllocationInfo(mAllocation, memory, memoryOffset);
}

VulkanBuffer::~VulkanBuffer()
{
    VulkanDevice* device = Driver->_getPrimaryDevice();

    if (mView != VK_NULL_HANDLE)
        vkDestroyBufferView(device->getLogical(), mView, VulkanDevice::gVulkanAllocator);

    vkDestroyBuffer(device->getLogical(), mBuffer, VulkanDevice::gVulkanAllocator);
    device->freeMemory(mAllocation);
}

UINT8* VulkanBuffer::map(VulkanDevice * device, VkDeviceSize offset, VkDeviceSize length) const
{
    UINT8* data;
    VkResult result = vkMapMemory(device->getLogical(), memory, memoryOffset + offset, length, 0, (void**)&data);
    assert(result == VK_SUCCESS);

    return data;
}

void VulkanBuffer::unmap(VulkanDevice * device)
{
    vkUnmapMemory(device->getLogical(), memory);
}

void VulkanBuffer::copy(VulkanCmdBuffer* cb, VulkanBuffer* destination, VkDeviceSize srcOffset,
    VkDeviceSize dstOffset, VkDeviceSize length)
{
    VkBufferCopy region;
    region.size = length;
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;

    vkCmdCopyBuffer(cb->getHandle(), mBuffer, destination->getHandle(), 1, &region);
}

void VulkanBuffer::copy(VulkanCmdBuffer* cb, VulkanImage* destination, const VkExtent3D& extent,
    const VkImageSubresourceLayers& range, VkImageLayout layout)
{
    VkBufferImageCopy region;
    region.bufferRowLength = mRowPitch;
    region.bufferImageHeight = mSliceHeight;
    region.bufferOffset = 0;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent = extent;
    region.imageSubresource = range;

    vkCmdCopyBufferToImage(cb->getHandle(), mBuffer, destination->getHandle(), layout, 1, &region);
}

void VulkanBuffer::update(VulkanCmdBuffer* cb, UINT8* data, VkDeviceSize offset, VkDeviceSize length)
{
    vkCmdUpdateBuffer(cb->getHandle(), mBuffer, offset, length, (uint32_t*)data);
}

void irr::video::VulkanBuffer::OnDeviceLost(CVulkanDriver * device)
{
}

void irr::video::VulkanBuffer::OnDeviceRestored(CVulkanDriver * device)
{
}

CVulkanHardwareBuffer::CVulkanHardwareBuffer(CVulkanDriver* driver, scene::IMeshBuffer *meshBuffer, video::IShaderDataBuffer* instanceBuffer, u32 flags, E_VERTEX_TYPE vtype)
    : IHardwareBuffer(meshBuffer, instanceBuffer)
    , CVulkanDeviceResource(driver)
    , Flags(flags)
{
}

irr::video::CVulkanHardwareBuffer::CVulkanHardwareBuffer(CVulkanDriver * driver, E_HARDWARE_BUFFER_TYPE type, E_HARDWARE_BUFFER_ACCESS accessType, u32 size, u32 flags, const void * initialData)
    : IHardwareBuffer(nullptr, nullptr)
    , CVulkanDeviceResource(driver)
    , Flags(flags)
{
}

irr::video::CVulkanHardwareBuffer::~CVulkanHardwareBuffer()
{
    if (CommandBuffer)
        CommandBuffer->drop();

    for (auto param : mGpuParams)
        param->drop();

    for (VulkanGraphicsPipelineState* pipeline : Pipelines)
        if (pipeline)
            pipeline->drop();

    for (BufferDesc& buffer : VertexBufferStreams)
    {
        if (buffer.Buffer)
            buffer.Buffer->drop();
        if (buffer.mStagingBuffer)
            buffer.mStagingBuffer->drop();
        if (buffer.mStagingMemory)
            free(buffer.mStagingMemory);

        if (buffer.BufferCache)
        {
            for (BufferCacheDesc& cacheBuffer : *buffer.BufferCache)
            {
                if (cacheBuffer.Buffer)
                    cacheBuffer.Buffer->drop();
            }
        }
    }

    Pipelines.clear();
    //VertexBufferStreams.clear();
}

void * irr::video::CVulkanHardwareBuffer::lock(E_HARDWARE_BUFFER_TYPE type, u32 length, bool readOnly)
{
    if (!VertexBufferStreams[(u32)type].initialize)
        return nullptr;

    CVulkanHardwareBuffer::BufferDesc& desc = VertexBufferStreams[(u32)type];

    VulkanBuffer* buffer = desc.Buffer;

    if (buffer == nullptr)
        return nullptr;

    if (length > desc.bufferCI.size)
        return nullptr;

    if (length == 0)
        return nullptr;

    mIsMapped = true;
    mMappedDeviceIdx = 0;
    mMappedGlobalQueueIdx = 0;
    mMappedSize = length;
    mMappedReadOnly = readOnly;

    VulkanDevice* device = mMappedDeviceIdx == 0 ? Driver->_getPrimaryDevice() : Driver->_getDevice(mMappedDeviceIdx);

    GpuQueueType queueType;
    UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

    VkAccessFlags accessFlags;
    if (mMappedReadOnly)
        accessFlags = VK_ACCESS_HOST_READ_BIT;
    else
        accessFlags = VK_ACCESS_HOST_WRITE_BIT;

    // If memory is host visible try mapping it directly
    if (desc.mDirectlyMappable)
    {
        // Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
        if (accessFlags == VK_ACCESS_HOST_WRITE_BIT)
        {
            if (buffer->isBound())
            {
                buffer->drop();
    
                if (desc.AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM || desc.AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC)
                    buffer = GetCacheBuffer(*device, desc, desc.bufferCI, desc.AccessType, desc.Stride, true);
                else
                    buffer = CreateBuffer(*device, desc.bufferCI, desc.AccessType, desc.Stride, true);
    
                desc.Buffer = buffer;
            }
    
            return buffer->map(device, desc.Offset, length);
        }
    
        // We need to read the buffer contents
        if (accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT))
        {
            // Check is the GPU currently reading or writing from the buffer
            UINT32 useMask;

            // We need to wait until (potential) read/write operations complete
            VulkanTransferBuffer* transferCB = device->getTransferBuffer(queueType, localQueueIdx);
    
            // Ensure flush() will wait for all queues currently using to the buffer (if any) to finish
            // If only reading, wait for all writes to complete, otherwise wait on both writes and reads
            if (readOnly)
                useMask = buffer->getUseInfo(VulkanUseFlag::Write);
            else
                useMask = buffer->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);
    
            transferCB->appendMask(useMask);
    
            // Make any writes visible before mapping
            if (desc.mSupportsGPUWrites)
            {
                // Issue a barrier so :
                //  - If reading: the device makes the written memory available for read (read-after-write hazard)
                //  - If writing: ensures our writes properly overlap with GPU writes (write-after-write hazard)
                transferCB->memoryBarrier(buffer->getHandle(),
                    VK_ACCESS_SHADER_WRITE_BIT,
                    accessFlags,
                    // Last stages that could have written to the buffer:
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_HOST_BIT
                );
            }
    
            // Submit the command buffer and wait until it finishes
            transferCB->flush(true);
    
            // If writing and some CB has an operation queued that will be using the current contents of the buffer, 
            // create a new  buffer so we don't modify the previous use of the buffer
            if (accessFlags == (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT) && buffer->isBound()) //(options == GBL_READ_WRITE && buffer->isBound())
            {
                VulkanBuffer* newBuffer = GetCacheBuffer(*device, desc, desc.bufferCI, desc.AccessType, desc.Stride, true);
    
                // Copy contents of the current buffer to the new one
                UINT8* src = buffer->map(device, desc.Offset, length);
                UINT8* dst = newBuffer->map(device, desc.Offset, length);
    
                memcpy(dst, src, length);
    
                buffer->unmap(device);
                newBuffer->unmap(device);
    
                buffer->drop();
                buffer = newBuffer;
                desc.Buffer = buffer;
            }
    
            return buffer->map(device, desc.Offset, length);
        }
    
        // Otherwise, we're doing write only, in which case it's best to use the staging buffer to avoid waiting
        // and blocking, so fall through
    }
    
    // Can't use direct mapping, so use a staging buffer or memory
    
    // We might need to copy the current contents of the buffer to the staging buffer. Even if the user doesn't plan on
    // reading, it is still required as we will eventually copy all of the contents back to the original buffer,
    // and we can't write potentially uninitialized data. The only exception is when the caller specifies the buffer
    // contents should be discarded in which he guarantees he will overwrite the entire locked area with his own
    // contents.

    bool needRead = accessFlags & VK_ACCESS_HOST_READ_BIT; //options != GBL_WRITE_ONLY_DISCARD_RANGE && options != GBL_WRITE_ONLY_DISCARD;

    // See if we can use the cheaper staging memory, rather than a staging buffer
    if (!needRead && /*offset % 4 == 0 &&*/ length % 4 == 0 && length <= 65536)
    {
        if (!desc.mStagingMemory || desc.mStagingMemorySize < length)
        {
            if (desc.mStagingMemory)
                free(desc.mStagingMemory);
            desc.mStagingMemory = (UINT8*)malloc(length);
            desc.mStagingMemorySize = length;
        }
        return desc.mStagingMemory;
    }

    // Create a staging buffer
    VkBufferCreateInfo StagingBufferCI = desc.bufferCI;
    desc.mStagingBuffer = CreateBuffer(*device, StagingBufferCI, E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC, 0, needRead, true);

    if (needRead)
    {
        VulkanTransferBuffer* transferCB = device->getTransferBuffer(queueType, localQueueIdx);

        // Similar to above, if buffer supports GPU writes or is currently being written to, we need to wait on any
        // potential writes to complete
        UINT32 writeUseMask = buffer->getUseInfo(VulkanUseFlag::Write);
        if (desc.mSupportsGPUWrites || writeUseMask != 0)
        {
            // Ensure flush() will wait for all queues currently writing to the buffer (if any) to finish
            transferCB->appendMask(writeUseMask);
        }

        // Queue copy command
        buffer->copy(transferCB->getCB(), desc.mStagingBuffer, 0, 0, length);

        // Ensure data written to the staging buffer is visible
        transferCB->memoryBarrier(desc.mStagingBuffer->getHandle(),
            VK_ACCESS_TRANSFER_WRITE_BIT,
            accessFlags,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT
        );

        // Submit the command buffer and wait until it finishes
        transferCB->flush(true);
        assert(!buffer->isUsed());
    }

    return desc.mStagingBuffer->map(device, 0, length);
}

void irr::video::CVulkanHardwareBuffer::unlock(E_HARDWARE_BUFFER_TYPE type)
{
    // Possibly map() failed with some error
    if (!mIsMapped)
        return;

    if (!VertexBufferStreams[(u32)type].initialize)
        return;

    CVulkanHardwareBuffer::BufferDesc& desc = VertexBufferStreams[(u32)type];

    VulkanBuffer* buffer = desc.Buffer;

    if (buffer == nullptr)
        return;

    // Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute 
    // a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

    if (desc.mStagingMemory == nullptr && desc.mStagingBuffer == nullptr) // We directly mapped the buffer
    {
        VulkanDevice* device = mMappedDeviceIdx == 0 ? Driver->_getPrimaryDevice() : Driver->_getDevice(mMappedDeviceIdx);
        desc.Buffer->unmap(device);
        buffer->NotifySoftBound(VulkanUseFlag::Write);
    }
    else
    {
        VulkanDevice* device = nullptr;

        if (desc.mStagingBuffer != nullptr)
        {
            device = mMappedDeviceIdx == 0 ? Driver->_getPrimaryDevice() : Driver->_getDevice(mMappedDeviceIdx);
            desc.mStagingBuffer->unmap(device);
        }

        bool isWrite = !mMappedReadOnly; // != GBL_READ_ONLY;

        // We the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
        if (isWrite)
        {
            if (!device)
                device = mMappedDeviceIdx == 0 ? Driver->_getPrimaryDevice() : Driver->_getDevice(mMappedDeviceIdx);

            GpuQueueType queueType;
            UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

            VulkanTransferBuffer* transferCB = device->getTransferBuffer(queueType, localQueueIdx);

            {
                // If buffer is queued for some operation on a CB, then we need to make a copy of the buffer to
                // avoid modifying its use in the previous operation
                if (buffer->isBound()) // Skip this when discard
                {
                   VulkanBuffer* newBuffer = GetCacheBuffer(*device, desc, desc.bufferCI, desc.AccessType, desc.Stride, true);
                
                    // Avoid copying original contents if the staging buffer completely covers it
                    if (/*mMappedOffset > 0 ||*/ mMappedSize > desc.bufferCI.size)
                    {
                        buffer->copy(transferCB->getCB(), newBuffer, 0, 0, desc.bufferCI.size);
                
                        transferCB->getCB()->registerResource(buffer, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
                    }

                    if (buffer->getReferenceCount() == 1 && buffer->isBound())
                        Driver->GetCommandBuffer()->getInternal()->registerResource(buffer, VulkanUseFlag::Read);
                    else
                        buffer->drop();
                    buffer = newBuffer;
                    desc.Buffer = buffer;
                }
            }

            // Queue copy/update command
            if (desc.mStagingBuffer != nullptr)
            {
                desc.mStagingBuffer->copy(transferCB->getCB(), buffer, 0, desc.Offset, mMappedSize);
                transferCB->getCB()->registerResource(desc.mStagingBuffer, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
            }
            else // Staging memory
            {
                //vkCmdUpdateBuffer(transferCB->getCB()->getHandle(), buffer->getHandle(), desc.Offset, mMappedSize, desc.mStagingMemory);
                buffer->update(transferCB->getCB(), desc.mStagingMemory, desc.Offset, mMappedSize);
            }

            buffer->NotifySoftBound(VulkanUseFlag::Write);
            //transferCB->getCB()->registerResource(buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VulkanUseFlag::Write);
        }

        if (desc.mStagingBuffer != nullptr)
        {
            desc.mStagingBuffer->drop();
            desc.mStagingBuffer = nullptr;
        }
    }

    mIsMapped = false;
}

void irr::video::CVulkanHardwareBuffer::setBaseBuffer(E_HARDWARE_BUFFER_TYPE type)
{
    CVulkanHardwareBuffer::BufferDesc& desc = VertexBufferStreams[(u32)type];

    VulkanBuffer* buffer = desc.Buffer;

    if (buffer == nullptr)
        return;

    mBaseBuffer = &desc;
}

void irr::video::CVulkanHardwareBuffer::copyFromMemory(E_HARDWARE_BUFFER_TYPE type, const void * sysData, u32 offset, u32 length)
{
}

void irr::video::CVulkanHardwareBuffer::copyFromBuffer(E_HARDWARE_BUFFER_TYPE type, IHardwareBuffer * buffer, u32 srcOffset, u32 descOffset, u32 length)
{
}

u32 irr::video::CVulkanHardwareBuffer::size(E_HARDWARE_BUFFER_TYPE type) const
{
    if (!VertexBufferStreams[(u32)type].initialize)
        return 0;

    return VertexBufferStreams[(u32)type].bufferCI.size;
}

u32 irr::video::CVulkanHardwareBuffer::GetMemoryAccessType(E_HARDWARE_BUFFER_ACCESS AccessType)
{
    if (AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE || AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT)
        return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


    if (AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC)
        return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
}

void irr::video::CVulkanHardwareBuffer::SetCommandBuffer(VulkanCommandBuffer * cb)
{
    if (CommandBuffer)
        CommandBuffer->drop();

    CommandBuffer = cb;

    if (CommandBuffer)
        CommandBuffer->grab();
}

VulkanGraphicsPipelineState * irr::video::CVulkanHardwareBuffer::GetPipeline(u8 idx)
{
    return Pipelines[idx];
}

void irr::video::CVulkanHardwareBuffer::SetPipeline(u8 idx, VulkanGraphicsPipelineState * pl)
{
    if (Pipelines.size() <= idx)
        Pipelines.resize(size_t(idx + 1));

    if (Pipelines[idx])
        Pipelines[idx]->drop();

    Pipelines[idx] = pl;

    if (Pipelines[idx])
        Pipelines[idx]->grab();
}

void irr::video::CVulkanHardwareBuffer::SetGpuParams(u8 idx, VulkanGpuParams * param)
{
    if (mGpuParams.size() <= idx)
        mGpuParams.resize(size_t(idx + 1));

    if (mGpuParams[idx])
        mGpuParams[idx]->drop();

    mGpuParams[idx] = param;

    if (mGpuParams[idx])
        mGpuParams[idx]->grab();
}

E_DRIVER_TYPE irr::video::CVulkanHardwareBuffer::getDriverType() const
{
    return E_DRIVER_TYPE::EDT_VULKAN;
}

E_HARDWARE_BUFFER_TYPE irr::video::CVulkanHardwareBuffer::getType(E_HARDWARE_BUFFER_TYPE type) const
{
    return type;
}

u32 irr::video::CVulkanHardwareBuffer::getFlags() const
{
    return Flags;
}

void irr::video::CVulkanHardwareBuffer::Bind()
{
    if (auto desc = GetBufferDesc(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX))
    {
        GetCommandBuffer()->getInternal()->setIndexBuffer(desc->Buffer, (this->Flags & (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_32_BITS) ? E_INDEX_TYPE::EIT_32BIT : E_INDEX_TYPE::EIT_16BIT, desc->Offset);
    }

    u32 vertexBufferNum = 0;
    VkDeviceSize offsets[2] = { 0, 0 };
    // Store the vertex and instance buffers into an array
    VulkanBuffer* vertBuffers[2] = { nullptr, nullptr };

    if (auto desc = GetBufferDesc(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX))
    {
        ++vertexBufferNum;

        offsets[0] = desc->Offset;
        vertBuffers[0] = desc->Buffer;
    }

    if (auto desc = GetBufferDesc(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM))
    {
        ++vertexBufferNum;

        offsets[1] = desc->Offset;
        vertBuffers[1] = desc->Buffer;
    }

    // set vertex buffer
    if (vertexBufferNum)
        GetCommandBuffer()->getInternal()->setVertexBuffers(0, vertBuffers, vertexBufferNum, offsets);

    IHardwareBuffer::Bind();
}

void irr::video::CVulkanHardwareBuffer::Unbind()
{
    IHardwareBuffer::Unbind();
}

void irr::video::CVulkanHardwareBuffer::Initialize()
{
}

void irr::video::CVulkanHardwareBuffer::Finalize()
{
}

void irr::video::CVulkanHardwareBuffer::OnDeviceLost(CVulkanDriver * device)
{
}

void irr::video::CVulkanHardwareBuffer::OnDeviceRestored(CVulkanDriver * device)
{
}

VulkanBuffer * irr::video::CVulkanHardwareBuffer::GetCacheBuffer(VulkanDevice & device, CVulkanHardwareBuffer::BufferDesc& descriptor, VkBufferCreateInfo& bufferCI, E_HARDWARE_BUFFER_ACCESS AccessType, u32 stride, bool readable)
{
    if (!descriptor.BufferCache)
        descriptor.BufferCache = new std::vector<BufferCacheDesc>;

    if (!descriptor.BufferCache->empty())
    {
        size_t startIndex = descriptor.BufferCacheHint > descriptor.BufferCache->size() ? descriptor.BufferCache->size() - 1 : descriptor.BufferCacheHint++;
        if (descriptor.BufferCache->size() <= descriptor.BufferCacheHint)
            descriptor.BufferCacheHint = 0;
        do
        {
            const BufferCacheDesc& entry = (*descriptor.BufferCache)[descriptor.BufferCacheHint];
            if (!entry.Buffer->isBound() && entry.bufferCI.size >= bufferCI.size)
            {
                entry.Buffer->grab();
                return entry.Buffer;
            }

            ++descriptor.BufferCacheHint;
            if (descriptor.BufferCache->size() <= descriptor.BufferCacheHint)
                descriptor.BufferCacheHint = 0;
        } while (startIndex != descriptor.BufferCacheHint);
    }

    // Cannot find an empty set, allocate a new one
    VulkanBuffer* buffer = CreateBuffer(device, bufferCI, AccessType, stride, readable, false);
    descriptor.BufferCache->emplace_back();
    BufferCacheDesc& cacheEntry = descriptor.BufferCache->back();
    cacheEntry.Offset = descriptor.Offset;
    cacheEntry.Stride = descriptor.Stride;
    cacheEntry.bufferCI = bufferCI;
    cacheEntry.Buffer = buffer;
    cacheEntry.Buffer->grab();
    return buffer;
}

VulkanBuffer * irr::video::CVulkanHardwareBuffer::CreateBuffer(VulkanDevice & device, VkBufferCreateInfo& bufferCI, E_HARDWARE_BUFFER_ACCESS AccessType, u32 stride, bool readable, bool staging)
{
    VulkanDevice* Device = Driver->_getPrimaryDevice();

    if (staging)
    {
        if (readable)
        {
            if (!CVulkanTexture::mStagingReadableBufferCache.empty())
            {
                size_t startIndex = CVulkanTexture::mStagingReadableBufferCacheHint;
                do
                {
                    const CVulkanTexture::StagingBufferEntry& entry = CVulkanTexture::mStagingReadableBufferCache[CVulkanTexture::mStagingReadableBufferCacheHint];
                    if (!entry.mBuffer->isBound() && entry.mSize >= bufferCI.size)
                    {
                        VulkanBuffer * stagingBuffer = entry.mBuffer;
                        stagingBuffer->grab();
                        return stagingBuffer;
                    }

                    ++CVulkanTexture::mStagingReadableBufferCacheHint;
                    if (CVulkanTexture::mStagingReadableBufferCache.size() <= CVulkanTexture::mStagingReadableBufferCacheHint)
                        CVulkanTexture::mStagingReadableBufferCacheHint = 0;

                } while (startIndex != CVulkanTexture::mStagingReadableBufferCacheHint);
            }
        }
        else
        {
            if (!CVulkanTexture::mStagingBufferCache.empty())
            {
                size_t startIndex = CVulkanTexture::mStagingBufferCacheHint;
                do
                {
                    const CVulkanTexture::StagingBufferEntry& entry = CVulkanTexture::mStagingBufferCache[CVulkanTexture::mStagingBufferCacheHint];
                    if (!entry.mBuffer->isBound() && entry.mSize >= bufferCI.size)
                    {
                        VulkanBuffer * stagingBuffer = entry.mBuffer;
                        stagingBuffer->grab();
                        return stagingBuffer;
                    }

                    ++CVulkanTexture::mStagingBufferCacheHint;
                    if (CVulkanTexture::mStagingBufferCache.size() <= CVulkanTexture::mStagingBufferCacheHint)
                        CVulkanTexture::mStagingBufferCacheHint = 0;

                } while (startIndex != CVulkanTexture::mStagingBufferCacheHint);
            }
        }

        bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        // Staging buffers are used as a destination for reads
        if (readable)
            bufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    else if (readable)
        bufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    bufferCI.usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer buffer;
    VkResult result = vkCreateBuffer(Device->getLogical(), &bufferCI, VulkanDevice::gVulkanAllocator, &buffer);
    assert(result == VK_SUCCESS);

    VkMemoryPropertyFlags flags = GetMemoryAccessType(AccessType);
    VmaAllocation allocation = Device->allocateBufferMemory(buffer, flags);

    VulkanBuffer* vkbuffer =  new VulkanBuffer(Driver, buffer, VK_NULL_HANDLE, allocation, stride);

    if (staging)
    {
        if (readable)
            CVulkanTexture::mStagingReadableBufferCache.push_back(CVulkanTexture::StagingBufferEntry{ vkbuffer, bufferCI.size });
        else
            CVulkanTexture::mStagingBufferCache.push_back(CVulkanTexture::StagingBufferEntry{ vkbuffer, bufferCI.size });
        vkbuffer->grab();
    }

    return vkbuffer;
}

bool irr::video::CVulkanHardwareBuffer::UpdateBuffer(E_HARDWARE_BUFFER_TYPE Type, E_HARDWARE_BUFFER_ACCESS AccessType, const void * initialData, u32 size, u32 offset, u32 dataSize, u32 typeMask, s32 preferBuffer)
{
    //if (VertexBufferStreams.size() <= (int)Type)
    //    VertexBufferStreams.resize(int(Type) + 1);

    CVulkanHardwareBuffer::BufferDesc& desc = VertexBufferStreams[(u32)Type];

    if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX || Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM)
    {
        if (!desc.initialize)
        {
            desc.bufferCI.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

            desc.Stride = getVertexDeclarationStride((u32)Type - 1);
            desc.initialize = true;
            desc.Type = Type;
            desc.Descriptor.offset = desc.Offset;
            desc.Descriptor.range = dataSize;
        }
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_INDEX)
    {
        if (!desc.initialize)
        {
            desc.bufferCI.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            desc.Stride = (this->Flags & (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_32_BITS) ? sizeof(u32) : sizeof(u16);
            desc.initialize = true;
            desc.Type = Type;
            desc.Descriptor.offset = desc.Offset;
            desc.Descriptor.range = dataSize;
        }
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_SHADER_RESOURCE)
    {
        if (!desc.initialize)
        {
            desc.bufferCI.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            desc.Stride = size;
            desc.initialize = true;
            desc.Type = Type;
            desc.mSupportsGPUWrites = true;
            desc.Descriptor.offset = desc.Offset;
            desc.Descriptor.range = dataSize;
        }
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS)
    {
        if (!desc.initialize)
        {
            desc.bufferCI.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            desc.Stride = size;
            desc.initialize = true;
            desc.Type = Type;
            desc.mSupportsGPUWrites = true;
            desc.Descriptor.offset = desc.Offset;
            desc.Descriptor.range = dataSize;
        }
    }
    else
    {
        os::Printer::log("Buffer type not supported", ELL_ERROR);
        return false;
    }

    //bool debug = false;
    if (size > 0)
    {
        if (desc.bufferCI.size < size || desc.AccessType != AccessType || desc.Offset != offset)
        {
            if (typeMask)
            {
                if (typeMask & (1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX) | 1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM)))
                    desc.bufferCI.usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                if (typeMask & (1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX)))
                    desc.bufferCI.usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }

            VulkanDevice* Device = Driver->_getPrimaryDevice();

            bool isSubBufferMode = false;
            bool needResizeBuffer = true;
            if (AccessType == E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE)
            {
                isSubBufferMode = true;

                if (offset == 0)
                {
                    if (mBaseBuffer != &desc)
                    {
                        mBaseBuffer = &desc;
                        mBaseBuffer->SubBuffers.push_back(mBaseBuffer);
                    }
                }
                else if (offset > 0 && !desc.Buffer)
                {
                    mBaseBuffer->SubBuffers.push_back(&desc);
                    desc.BufferCache = mBaseBuffer->BufferCache;
                    desc.Buffer = mBaseBuffer->Buffer;
                }

                if (!mBaseBuffer)
                    return false; // This triggered when base buffer not initialized.

                needResizeBuffer = mBaseBuffer->bufferCI.size < size;
            }

            desc.bufferCI.size = size;
            desc.AccessType = AccessType;
            desc.Offset = offset;

            if (needResizeBuffer)
            {
                if (desc.BufferCache)
                {
                    for (BufferCacheDesc& cacheBuffer : *desc.BufferCache)
                    {
                        if (cacheBuffer.Buffer)
                        {
                            if (cacheBuffer.Buffer->isBound())
                                Driver->GetCommandBuffer()->getInternal()->registerResource(cacheBuffer.Buffer, VulkanUseFlag::Read);
                            cacheBuffer.Buffer->drop();
                        }
                    }

                    desc.BufferCache->clear();
                    desc.BufferCacheHint = 0;
                }

                VulkanBuffer* oldBuffer = desc.Buffer;
                u8* mappedPtr;
                if (isSubBufferMode && oldBuffer)
                    mappedPtr = mBaseBuffer->SubBuffers.size() > 1 ? (u8*)lock(mBaseBuffer->Type, mBaseBuffer->bufferCI.size, true) : nullptr;

                desc.mDirectlyMappable = (GetMemoryAccessType(AccessType) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
                if (desc.mDirectlyMappable)
                    desc.Buffer = GetCacheBuffer(*Device, desc, desc.bufferCI, desc.AccessType, desc.Stride, true);
                else
                    desc.Buffer = CreateBuffer(*Device, desc.bufferCI, AccessType, desc.Stride, desc.mDirectlyMappable);

                // Sub buffers always gpu visible we need command buffer for memmove
                if (isSubBufferMode && oldBuffer && mappedPtr)
                {
                    u32 _dBufferSize = dataSize - desc.Descriptor.range;

                    GpuQueueType queueType;
                    UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(0, queueType);

                    VulkanTransferBuffer* transferCB = Device->getTransferBuffer(queueType, localQueueIdx);

                    // Similar to above, if buffer supports GPU writes or is currently being written to, we need to wait on any
                    // potential writes to complete
                    UINT32 writeUseMask = oldBuffer->getUseInfo(VulkanUseFlag::Write);
                    if (desc.mSupportsGPUWrites || writeUseMask != 0)
                    {
                        // Ensure flush() will wait for all queues currently writing to the buffer (if any) to finish
                        transferCB->appendMask(writeUseMask);
                    }

                    for (auto buffer : mBaseBuffer->SubBuffers)
                    {
                        if (buffer == &desc)
                            continue;

                        // Calculate new offset
                        VkDeviceSize newOffset = buffer->Offset;
                        if (newOffset > desc.Offset)
                            newOffset += _dBufferSize;

                        // Queue copy/update command
                        mBaseBuffer->mStagingBuffer->copy(transferCB->getCB(), desc.Buffer, buffer->Offset, newOffset, buffer->Descriptor.range);

                        buffer->Buffer = desc.Buffer;
                        buffer->Offset = newOffset;
                        buffer->Descriptor.offset = buffer->Offset;
                        buffer->bufferCI.size = size;
                    }

                    transferCB->getCB()->registerResource(mBaseBuffer->mStagingBuffer, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
                    transferCB->getCB()->registerResource(desc.Buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VulkanUseFlag::Write);

                    // Submit the command buffer and wait until it finishes
                    transferCB->flush(true);

                    //debug = true;
                    // Release reading stanging buffer
                    unlock(mBaseBuffer->Type);
                }

                if (oldBuffer)
                    oldBuffer->drop();
            }

            desc.Descriptor.buffer = desc.Buffer->getHandle();
            desc.Descriptor.offset = desc.Offset;
            desc.Descriptor.range = isSubBufferMode ? dataSize : size;
        }

        if (initialData)
        {
            if (preferBuffer > -1)
                desc.BufferCacheHint = preferBuffer - 1;

            u8* mappedPtr = (u8*)lock(Type, dataSize ? dataSize : size);
            memcpy(mappedPtr, initialData, dataSize ? dataSize : size);
            unlock(Type);
            //if (debug)
            //{
            //    mappedPtr = (u8*)lock(mBaseBuffer->Type, mBaseBuffer->bufferCI.size, true);
            //    unlock(mBaseBuffer->Type);
            //}
        }
    }

    return true;
}

u32 irr::video::CVulkanHardwareBuffer::getVertexDeclarationStride(u8 inputSlot) const
{
    return GetBuffer() ? static_cast<CVulkanVertexDeclaration*>(GetBuffer()->GetVertexDeclaration())->GetVertexPitch(inputSlot) : static_cast<CVulkanVertexDeclaration*>(Driver->GetVertexDeclaration(E_VERTEX_TYPE::EVT_STANDARD))->GetVertexPitch(inputSlot);
}

irr::video::CVulkanHardwareBuffer::BufferDesc::BufferDesc()
{
    memset(this, 0, sizeof(BufferDesc));

    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
}
