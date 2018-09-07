#ifndef __C_VULKAN_VERTEX_DECLARATION_H_INCLUDED__
#define __C_VULKAN_VERTEX_DECLARATION_H_INCLUDED__

#include "RenderEngines/General/CIrrVertexDeclaration.h"
#include "CVulkanUtility.h"
#include "CVulkanResources.h"

#include <vector>

namespace irr
{
    namespace video
    {
        class CVulkanVertexDeclaration
            : public VertexDeclaration
            , protected CVulkanDeviceResource
        {
            std::vector<irr::u32> VertexPitch;

            std::vector<VkVertexInputBindingDescription> BindingDesc;
            std::vector<VkVertexInputAttributeDescription> VertexDeclaration;
            std::vector<VkDescriptorSetLayout> Descriptors;

            VkPipelineVertexInputStateCreateInfo mCreateInfo;

        public:
            CVulkanVertexDeclaration(CVulkanDriver* driver);
            virtual ~CVulkanVertexDeclaration();

            void initialize() override;

            const VkPipelineVertexInputStateCreateInfo& getVertexDeclaration();
            std::vector<VkDescriptorSetLayout> GetDescriptors();

            irr::u32 GetVertexPitch(irr::u8 inputSlot) const;

            virtual void OnDeviceLost(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceRestored(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceDestroy(CVulkanDriver* device) _IRR_OVERRIDE_;
        };
    }
}

#endif