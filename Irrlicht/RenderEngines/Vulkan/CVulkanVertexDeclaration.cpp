#include "IrrCompileConfig.h"

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE

#include "CVulkanDriver.h"
#include "CVulkanVertexDeclaration.h"
#include "CVulkanShader.h"

#include "os.h"

using namespace irr;
using namespace video;

VkFormat getVkFormat(E_VERTEX_ELEMENT_TYPE type)
{
    switch (type)
    {
        case EVET_FLOAT1:
            return VkFormat::VK_FORMAT_R32_SFLOAT;
        case EVET_FLOAT2:
            return VkFormat::VK_FORMAT_R32G32_SFLOAT;
        case EVET_FLOAT3:
            return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
        case EVET_FLOAT4:
            return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;

        case EVET_UINT1:
            return VkFormat::VK_FORMAT_R32_UINT;
        case EVET_UINT2:
            return VkFormat::VK_FORMAT_R32G32_UINT;
        case EVET_UINT3:
            return VkFormat::VK_FORMAT_R32G32B32_UINT;
        case EVET_UINT4:
            return VkFormat::VK_FORMAT_R32G32B32A32_UINT;

        case EVET_INT1:
            return VkFormat::VK_FORMAT_R32_SINT;
        case EVET_INT2:
            return VkFormat::VK_FORMAT_R32G32_SINT;
        case EVET_INT3:
            return VkFormat::VK_FORMAT_R32G32B32_SINT;
        case EVET_INT4:
            return VkFormat::VK_FORMAT_R32G32B32A32_SINT;

        case EVET_SHORT2:
            return VkFormat::VK_FORMAT_R16G16_UINT;
        case EVET_SHORT4:
            return VkFormat::VK_FORMAT_R16G16B16A16_UINT;

        case EVET_UBYTE4_NORM:
        case EVET_COLOUR:
            return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        case EVET_UBYTE4:
            return VkFormat::VK_FORMAT_R8G8B8A8_UINT;
    }

    return VkFormat::VK_FORMAT_R32_SFLOAT;
}

size_t getVkFormatBitSize(E_VERTEX_ELEMENT_TYPE type)
{
    switch (type)
    {
        case EVET_FLOAT1:
            return 32;
        case EVET_FLOAT2:
            return 32 * 2;
        case EVET_FLOAT3:
            return 32 * 3;
        case EVET_FLOAT4:
            return 32 * 4;

        case EVET_UINT1:
        case EVET_INT1:
            return 32;
        case EVET_UINT2:
        case EVET_INT2:
            return 32 * 2;
        case EVET_UINT3:
        case EVET_INT3:
            return 32 * 3;
        case EVET_UINT4:
        case EVET_INT4:
            return 32 * 4;

        case EVET_SHORT2:
            return 16 * 2;
        case EVET_SHORT4:
            return 16 * 4;

        case EVET_UBYTE4_NORM:
        case EVET_COLOUR:
        case EVET_UBYTE4:
            return 8 * 4;
    }

    return 32;
}

CVulkanVertexDeclaration::CVulkanVertexDeclaration(CVulkanDriver* driver)
    : CVulkanDeviceResource(driver)
    , VertexPitch(0)
{
}

CVulkanVertexDeclaration::~CVulkanVertexDeclaration()
{
}

void irr::video::CVulkanVertexDeclaration::initialize()
{
    BindingDesc.clear();
    VertexDeclaration.clear();
    for (u32 i = 0; i < mVertexElements.size(); i++)
    {
        if (BindingDesc.size() <= mVertexElements[i].SlotIndex)
            BindingDesc.resize(mVertexElements[i].SlotIndex + 1);

        auto& BindDecl = BindingDesc[mVertexElements[i].SlotIndex];
        BindDecl.binding = mVertexElements[i].SlotIndex;
        BindDecl.inputRate = mVertexElements[i].PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

        if (VertexPitch.size() < mVertexElements[i].SlotIndex + 1)
            VertexPitch.resize(mVertexElements[i].SlotIndex + 1);

        VertexDeclaration.emplace_back();
        VkVertexInputAttributeDescription& desc = VertexDeclaration.back();

        desc.location = i;
        desc.binding = mVertexElements[i].SlotIndex;
        desc.offset = mVertexElements[i].Offset == -1 ? VertexPitch[desc.binding] : mVertexElements[i].Offset;
        desc.format = getVkFormat(mVertexElements[i].Type);

        VertexPitch[desc.binding] += getVkFormatBitSize(mVertexElements[i].Type) / 8;
    }

    for (u32 i = 0; i < BindingDesc.size(); ++i)
    {
        auto& BindDecl = BindingDesc[i];
        BindDecl.stride = VertexPitch[BindDecl.binding];
    }

    mCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    mCreateInfo.pNext = nullptr;
    mCreateInfo.flags = 0;
    mCreateInfo.pVertexBindingDescriptions = BindingDesc.data();
    mCreateInfo.vertexBindingDescriptionCount = BindingDesc.size();
    mCreateInfo.pVertexAttributeDescriptions = VertexDeclaration.data();
    mCreateInfo.vertexAttributeDescriptionCount = VertexDeclaration.size();
}

const VkPipelineVertexInputStateCreateInfo& irr::video::CVulkanVertexDeclaration::getVertexDeclaration()
{
    if (BindingDesc.empty())
        initialize();

    return mCreateInfo;
}

std::vector<VkDescriptorSetLayout> irr::video::CVulkanVertexDeclaration::GetDescriptors()
{
    if (BindingDesc.empty())
        initialize();

    return Descriptors;
}

irr::u32 irr::video::CVulkanVertexDeclaration::GetVertexPitch(irr::u8 inputSlot) const
{
    if (BindingDesc.empty())
        const_cast<CVulkanVertexDeclaration*>(this)->initialize();

    return VertexPitch[inputSlot];
}

void irr::video::CVulkanVertexDeclaration::OnDeviceLost(CVulkanDriver * device)
{
}

void irr::video::CVulkanVertexDeclaration::OnDeviceRestored(CVulkanDriver * device)
{
}

void irr::video::CVulkanVertexDeclaration::OnDeviceDestroy(CVulkanDriver* device)
{
    drop();
}