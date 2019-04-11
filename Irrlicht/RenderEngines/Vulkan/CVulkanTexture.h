#ifndef __C_VULKAN_TEXTURE_H_INCLUDED__
#define __C_VULKAN_TEXTURE_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "CVulkanUtility.h"
#include "CVulkanResources.h"
#include "CVulkanDevice.h"

#include "ITexture.h"
#include "IImage.h"

namespace irr
{
    namespace video
    {
        class CVulkanDriver;
        class VulkanDevice;
        class VulkanBuffer;
        class VulkanCmdBuffer;
        class VulkanImageSubresource;

        /** @addtogroup Vulkan
        *  @{
        */

        /** Descriptor used for initializing a VulkanImage. */
        struct VULKAN_IMAGE_DESC
        {
            VkImage image; /**< Internal Vulkan image object */
            VmaAllocation allocation; /** Information about the memory allocated for this image. */
            VkImageLayout layout; /**< Initial layout of the image. */
            E_TEXTURE_TYPE type; /**< Type of the image. */
            VkFormat format; /**< Pixel format of the image. */
            uint32_t numFaces; /**< Number of faces (array slices, or cube-map faces). */
            uint32_t numMipLevels; /**< Number of mipmap levels per face. */
            uint32_t usage; /** Determines how will the image be used. */
        };

        struct MappedImageData
        {
            ECOLOR_FORMAT format;
            uint32_t rowPitch;
            uint32_t slicePitch;
            uint32_t size;
            uint8_t* data;
        };

        /** Contains information about a single Vulkan image resource bound/used on this command buffer. */
        struct ImageInfo
        {
            uint32_t subresourceInfoIdx = 0;
            uint32_t numSubresourceInfos = 0;
        };

        /** Contains information about a range of Vulkan image sub-resources bound/used on this command buffer. */
        struct ImageSubresourceInfo
        {
            VkImageSubresourceRange range;

            // Only relevant for layout transitions
            VkImageLayout initialLayout;
            VkImageLayout currentLayout;
            VkImageLayout requiredLayout;
            VkImageLayout finalLayout;

            bool isFBAttachment : 1;
            bool isShaderInput : 1;
            bool hasTransitioned : 1;
            bool hasExternalTransition : 1;
            bool isReadOnly : 1;
            bool isInitialReadOnly : 1;

            /**
            * True if the buffer was at some point written to by the shader during the current render pass, and barrier
            * wasn't issued yet.
            */
            bool needsBarrier : 1;
        };

        /** Wrapper around a Vulkan image object that manages its usage and lifetime. */
        class VulkanImage : public CVulkanDeviceResource
        {
        public:
            /**
            * @param[in]	owner		Resource manager that keeps track of lifetime of this resource.
            * @param[in]	desc		Describes the image to assign.
            * @param[in]	ownsImage	If true, this object will take care of releasing the image and its memory, otherwise
            *							it is expected they will be released externally.
            */
            VulkanImage(CVulkanDriver* owner, const VULKAN_IMAGE_DESC& desc, bool ownsImage = true);
            virtual ~VulkanImage();

            /** Returns the internal handle to the Vulkan object. */
            VkImage getHandle() const { return mImage; }

            /** Returns the preferred (not necessarily current) layout of the image. */
            VkImageLayout getOptimalLayout() const;

            /**
            * Returns an image view that covers all faces and mip maps of the texture.
            *
            * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
            *							attachment flags are set on the view.
            */
            VkImageView getView(bool framebuffer) const;

            /**
            * Returns an image view that covers the specified faces and mip maps of the texture.
            *
            * @param[in]	surface		Surface that describes which faces and mip levels to retrieve the view for.
            * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
            *							attachment flags are set on the view.
            */
            VkImageView getView(const TextureSurface& surface, bool framebuffer) const;

            /** Get aspect flags that represent the contents of this image. */
            VkImageAspectFlags getAspectFlags() const;

            /** Retrieves a subresource range covering all the sub-resources of the image. */
            VkImageSubresourceRange getRange() const;

            /** Retrieves a subresource range covering all the specified sub-resource range of the image. */
            VkImageSubresourceRange getRange(const TextureSurface& surface) const;

            /**
            * Retrieves a separate resource for a specific image face & mip level. This allows the caller to track subresource
            * usage individually, instead for the entire image.
            */
            VulkanImageSubresource* getSubresource(uint32_t face, uint32_t mipLevel);

            /**
            * Returns a pointer to internal image memory for the specified sub-resource. Must be followed by unmap(). Caller
            * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
            * GPU.
            *
            * @param[in]	face		Index of the face to map.
            * @param[in]	mipLevel	Index of the mip level to map.
            * @param[in]	output		Output object containing the pointer to the sub-resource data.
            */
            void map(uint32_t face, uint32_t mipLevel, MappedImageData& output) const;

            /**
            * Returns a pointer to internal image memory for the entire resource. Must be followed by unmap(). Caller
            * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
            * GPU.
            */
            uint8_t* map(uint32_t offset, uint32_t size) const;

            /** Unmaps a buffer previously mapped with map(). */
            void unmap();

            /**
            * Queues a command on the provided command buffer. The command copies the contents of the current image
            * subresource to the destination buffer.
            */
            void copy(VulkanCmdBuffer* cb, VulkanBuffer* destination, const VkExtent3D& extent,
                const VkImageSubresourceLayers& range, VkImageLayout layout);

            /**
            * Determines a set of access flags based on the current image and provided image layout. This method makes
            * certain assumptions about image usage, so it might not be valid in all situations.
            *
            * @param[in]	layout		Layout the image is currently in.
            * @param[in]	readOnly	True if the image is only going to be read without writing, allows the system to
            *							set less general access flags. If unsure, set to false.
            */
            VkAccessFlags getAccessFlags(VkImageLayout layout, bool readOnly = false);

            /**
            * Generates a set of image barriers that are grouped depending on the current layout of individual sub-resources
            * in the specified range. The method will try to reduce the number of generated barriers by grouping as many
            * sub-resources as possibly.
            */
            void getBarriers(const VkImageSubresourceRange& range,std::vector<VkImageMemoryBarrier>& barriers);


            const VkFormat GetImageFormat() const { return mImageViewCI.format; }

            ImageInfo mImageInfo;

        private:
            /** Creates a new view of the provided part (or entirety) of surface. */
            VkImageView createView(const TextureSurface&, VkImageAspectFlags aspectMask) const;

            /** Contains information about view for a specific surface(s) of this image. */
            struct ImageViewInfo
            {
                TextureSurface surface;
                bool framebuffer;
                VkImageView view;
            };

            VkImage mImage;
            VmaAllocation mAllocation;
            VkImageView mMainView;
            VkImageView mFramebufferMainView;
            s32 mUsage;
            bool mOwnsImage;

            uint32_t mNumFaces;
            uint32_t mNumMipLevels;
            irr::Ptr<VulkanImageSubresource>* mSubresources;

            mutable VkImageViewCreateInfo mImageViewCI;
            mutable std::vector<ImageViewInfo> mImageInfos;

            // Inherited via CVulkanDeviceResource
            virtual void OnDeviceLost(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceRestored(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceDestroy(CVulkanDriver* device) _IRR_OVERRIDE_ {}
        };

        /** Represents a single sub-resource (face & mip level) of a larger image object. */
        class VulkanImageSubresource : public CVulkanDeviceResource
        {
        public:
            VulkanImageSubresource(CVulkanDriver* owner, VkImageLayout layout);
            virtual ~VulkanImageSubresource() {}
            /**
            * Returns the layout the subresource is currently in. Note that this is only used to communicate layouts between
            * different command buffers, and will only be updated only after command buffer submit() call. In short this means
            * you should only care about this value on the core thread.
            */
            VkImageLayout getLayout() const { return mImageSubresourceInfo.currentLayout; }

            /** Notifies the resource that the current subresource layout has changed. */
            //void setLayout(VkImageLayout layout) { mLayout = layout; }

            ImageSubresourceInfo mImageSubresourceInfo;

        private:
            //VkImageLayout mLayout;

            // Inherited via CVulkanDeviceResource
            virtual void OnDeviceLost(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceRestored(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceDestroy(CVulkanDriver* device) _IRR_OVERRIDE_ {}
        };

        class CVulkanTexture : public ITexture//, public CVulkanDeviceResource
        {
        public:

            //! constructor
            CVulkanTexture(IImage* image, CVulkanDriver* driver, u32 flags, const io::path& name,
                u32 arraySlices = 1, void* mipmapData = 0, u32 sampleCount = 1, u32 sampleQuality = 0, E_TEXTURE_TYPE type = E_TEXTURE_TYPE::e2D);

            //! rendertarget constructor
            CVulkanTexture(CVulkanDriver* driver, const core::dimension2d<u32>& size, const io::path& name,
                const ECOLOR_FORMAT format = ECF_UNKNOWN, u32 arraySlices = 1, u32 sampleCount = 1, u32 sampleQuality = 0, E_TEXTURE_TYPE type = E_TEXTURE_TYPE::e2D);

            CVulkanTexture(CVulkanDriver* driver, VulkanImage* image, const core::dimension2d<u32>& size, const ECOLOR_FORMAT format = ECF_UNKNOWN);

            //! destructor
            virtual ~CVulkanTexture();

            virtual void clearImage() override;

            //! lock function
            virtual void* lock(E_TEXTURE_LOCK_MODE mode = ETLM_READ_WRITE, u32 mipmapLevel = 0, u32 arraySlice = 0) override;
            
            virtual void updateTexture(u32 level, u32 x, u32 y, u32 width, u32 height, const void* data) override;

            //! unlock function
            virtual void unlock() override;

            //! Returns original size of the texture.
            virtual const core::dimension2d<u32>& getOriginalSize() const override;

            //! Returns (=size) of the texture.
            virtual const core::dimension2d<u32>& getSize() const override;

            //! returns driver type of texture (=the driver, who created the texture)
            virtual E_DRIVER_TYPE getDriverType() const override;

            //! returns color format of texture
            virtual ECOLOR_FORMAT getColorFormat() const override;

            virtual u32 getPitch() const override;

            //! returns if texture has mipmap levels
            bool hasMipMaps() const override;

            virtual u32 getNumberOfArraySlices() const;

            bool createMipMaps(u32 level = 1);

            //! returns if it is a render target
            virtual bool isRenderTarget() const override;

            VulkanImage* GetVkImages(u8 deviceIdx) const { return mImages[deviceIdx]; }

            TextureSurface GetSurface() const
            {
                return std::move(TextureSurface(0, NumberOfMipLevels, 0, NumberOfArraySlices));
            }

        private:

            //! creates hardware render target
            void createRenderTarget(const ECOLOR_FORMAT format = ECF_UNKNOWN);

            //! creates the hardware texture
            bool createTexture(u32 flags, IImage * image, E_TEXTURE_TYPE type);
            VulkanImage* createImage(VulkanDevice& device, u32 flags, ECOLOR_FORMAT format);

            //! copies the image to the texture
            void copyImage(VulkanTransferBuffer* cb, VulkanImage* srcImage, VulkanImage* dstImage, VkImageLayout srcFinalLayout, VkImageLayout dstFinalLayout);
            bool copyTexture(IImage * image);

            //! create texture buffer needed for lock/unlock
            bool createTextureBuffer(bool readable, uint64_t size = 0);

            //! create views to bound texture to pipeline
            bool createViews();

            // Inherited via CVulkanDeviceResource
            //virtual void OnDeviceLost(CVulkanDriver * device) override;
            //virtual void OnDeviceRestored(CVulkanDriver * device) override;

            // Inherited via ITexture
            virtual void regenerateMipMapLevels(IImage* image = nullptr) override;

            friend class CVulkanDriver;

            bool InitializeColorFormat(u32 flags, ECOLOR_FORMAT colorFormat);

            CVulkanDriver* Driver;
            VulkanDevice* Device;
            irr::Ptr<IImage> Image;
            core::dimension2d<u32> TextureSize;
            core::dimension2d<u32> ImageSize;
            u32 Pitch;
            u32 UsageFlags;
            u8 QualityOfSample;
            u8 NumberOfSamples;
            u8 NumberOfMipLevels;
            u8 NumberOfArraySlices;

            E_TEXTURE_LOCK_MODE MappedMode;
            u8 mMappedDeviceIdx;
            u8 mMappedGlobalQueueIdx;
            u8 mMappedMipLevelIdx;
            u8 mMappedArraySliceIdx;

            /** Contains information about view for a specific surface(s) of this image. */
            std::array<irr::Ptr<VulkanImage>, _MAX_DEVICES> mImages;
            std::array<ECOLOR_FORMAT, _MAX_DEVICES> mInternalFormats;
            E_TEXTURE_TYPE mTextureType;
            GpuDeviceFlags mDeviceMask;
            VkImageCreateInfo mImageCI;

            irr::Ptr<VulkanBuffer> mStagingBuffer;
            bool mStagingBufferReadable = false;
            bool mSupportsGPUWrites = false;
            bool mDirectlyMappable = false;
            bool mIsMapped = false;
        };
    }
}

#endif