#pragma once

#include "CVulkanUtility.h"
#include "CVulkanResources.h"

namespace irr
{
    namespace video
    {
        /** @addtogroup Vulkan
         *  @{
         */

        class VulkanBuffer;
        class VulkanCmdBuffer;
        class VulkanDescriptorSet;
        class CVulkanHardwareBuffer;
        class CVulkanGLSLProgram;

         /** Vulkan implementation of GpuParams, containing resource descriptors for all shader stages. */
        class VulkanGpuParams : public CVulkanDeviceResource
        {
        public:
            VulkanGpuParams(CVulkanDriver* driver, CVulkanGLSLProgram* shader, GpuDeviceFlags deviceMask);
            virtual ~VulkanGpuParams();

            /** @copydoc GpuParams::initialize */
            void initialize();
            void reset();

            /** @copydoc GpuParams::setParamBlockBuffer(u32, u32, const ParamsBufferType&) */
            void setParamBlockBuffer(u32 set, u32 slot, const CVulkanHardwareBuffer* paramBlockBuffer);

            /** @copydoc GpuParams::setTexture */
            void setTexture(u32 set, u32 slot, const ITexture* texture, const TextureSurface& surface);

            /** @copydoc GpuParams::setLoadStoreTexture */
            void setLoadStoreTexture(u32 set, u32 slot, const ITexture* texture, const TextureSurface& surface);

            /** @copydoc GpuParams::setBuffer */
            void setBuffer(u32 set, u32 slot, const CVulkanHardwareBuffer* buffer);
            s32 getBufferId(u32 set, u32 slot)
            {
                if (bufferBindingSlot.size() <= slot)
                    return -1;

                u32 bindingIdx = bufferBindingSlot[slot];
                if (!mPerDeviceData[0])
                    return -1;
                return mPerDeviceData[0][set].writeInfos[bindingIdx].bufferId;
            }

            /** @copydoc GpuParams::setSamplerState */
            void setSamplerState(u32 set, u32 slot, const VkSampler& sampler);

            /** Returns the total number of descriptor sets used by this object. */
            u32 getNumSets() const;

            /**
             * Prepares the internal descriptor sets for a bind operation on the provided command buffer. It generates and/or
             * updates and descriptor sets, and registers the relevant resources with the command buffer.
             *
             * Caller must perform external locking if some other thread could write to this object while it is being bound.
             * The same applies to any resources held by this object.
             *
             * @param[in]	buffer	Buffer on which the parameters will be bound to.
             * @param[out]	sets	Pre-allocated buffer in which the descriptor set handled will be written. Must be of
             *						getNumSets() size.
             *
             * @note	Thread safe.
             */
            bool UpdateDescriptors(VulkanCmdBuffer& buffer, VkDescriptorSet* sets);

            // Inherited via CVulkanDeviceResource
            virtual void OnDeviceLost(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceRestored(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceDestroy(CVulkanDriver* device) _IRR_OVERRIDE_ {}

            u8 GetTextureCount() const { return u8(TextureBindingSlot.size()); }

        protected:
            /** Contains data about writing to either buffer or a texture descriptor. */
            struct WriteInfo
            {
                const ITexture* texture = nullptr;
                union
                {
                    VkDescriptorImageInfo image;
                    VkDescriptorBufferInfo buffer;
                    VkBufferView bufferView;
                };
                s32 bufferId = -1;
            };

            /** All GPU param data related to a single descriptor set. */
            struct PerSetData
            {
                VulkanDescriptorSet* latestSet = nullptr;
                std::vector<VulkanDescriptorSet*> sets;

                std::vector<VkWriteDescriptorSet> writeSetInfos;
                std::vector<WriteInfo> writeInfos;

                u32 numElements = 0;
                bool mSetsDirty = true;
            };

            core::array<u32> bufferBindingSlot;
            core::array<u32> TextureBindingSlot;

            std::array<PerSetData*, _MAX_DEVICES> mPerDeviceData;
            GpuDeviceFlags mDeviceMask;
            u32 mNumDeviceData;

            CVulkanGLSLProgram* mShader = nullptr;

            //std::mutex mMutex;
        };

        /** @} */
    }
}