#pragma once

#ifndef __C_VIDEO_VULKAN_H_INCLUDED__
#define __C_VIDEO_VULKAN_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "SIrrCreationParameters.h"

#include <unordered_map>
#include <set>
#include <string>
#include <map>
#include <algorithm>

#include "CNullDriver.h"
#include "IMaterialRendererServices.h"
#include "CVulkanUtility.h"

#include <set>

#if _DEBUG
#define DEVEL_LOG(msg,code)	os::Printer::log(msg, code)	
#endif

#define VULKAN_DEBUG_MODE 0

namespace irr
{
    namespace video
    {
        class CVulkanFixedFunctionMaterialRenderer;
        class CVulkanMaterialRenderer_ONETEXTURE_BLEND;
        class CVulkanVertexDeclaration;
        class CVulkanHardwareBuffer;
        struct CVulkanDeviceResource;
        class CVulkanGLSLProgram;
        class VulkanDevice;
        class VulkanCommandBuffer;
        class VulkanSemaphore;
        class CVulkanPlatform;
        class VulkanSwapChain;
        class CVulkanTexture;
        class VulkanGraphicsPipelineState;

        class CVulkanDriver : public CNullDriver, IMaterialRendererServices
        {
        public:
            friend class CVulkanFixedFunctionMaterialRenderer;
            friend class CVulkanTexture;
            friend class GenericVulkanContext;

            //! constructor
            CVulkanDriver(const SIrrlichtCreationParameters& params, io::IFileSystem* io);

            //! destructor
            virtual ~CVulkanDriver();

            void ReleaseDriver();

            bool _getErrorsFromQueue() const;

            //! applications must call this method before performing any rendering. returns false if failed.
            virtual bool beginScene(bool backBuffer = true, bool zBuffer = true,
                SColor color = SColor(255, 0, 0, 0),
                const SExposedVideoData& videoData = SExposedVideoData(),
                core::rect<s32>* sourceRect = 0);

            //! applications must call this method after performing any rendering. returns false if failed.
            virtual bool endScene();

            //! queries the features of the driver, returns true if feature is available
            virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

            //! sets transformation
            virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat);

            //! sets a material
            virtual void setMaterial(const SMaterial& material) override;

            // Inherited via CNullDriver
            virtual ITexture * addTextureCubemap(const io::path & name, IImage * imagePosX, IImage * imageNegX, IImage * imagePosY, IImage * imageNegY, IImage * imagePosZ, IImage * imageNegZ) override;
            virtual ITexture * addTextureArray(const io::path & name, irr::core::array<IImage*> images) override;

            virtual IRenderTarget* addRenderTarget();

            virtual bool setRenderTargetEx(IRenderTarget* target, u16 clearFlag, SColor clearColor = SColor(255, 0, 0, 0),
                f32 clearDepth = 1.f, u8 clearStencil = 0);

            virtual bool setRenderTarget(ITexture* texture, u16 clearFlag = ECBF_COLOR | ECBF_DEPTH, SColor clearColor = SColor(255, 0, 0, 0),
                f32 clearDepth = 1.f, u8 clearStencil = 0);

            //! sets a viewport
            virtual void setViewPort(const core::rect<s32>& area);

            //! gets the area of the current viewport
            virtual const core::rect<s32>& getViewPort() const;

            bool updateVertexHardwareBuffer(CVulkanHardwareBuffer *HWBuffer, E_HARDWARE_BUFFER_TYPE Type);
            bool updateIndexHardwareBuffer(CVulkanHardwareBuffer *HWBuffer);

            //! updates hardware buffer if needed
            virtual bool updateHardwareBuffer(IHardwareBuffer *HWBuffer);

            //! Create hardware buffer from mesh
            virtual IHardwareBuffer *createHardwareBuffer(const scene::IMeshBuffer* mb);

            //! Delete hardware buffer (only some drivers can)
            virtual void deleteHardwareBuffer(IHardwareBuffer *HWBuffer);

            //! is vbo recommended on this mesh? for DirectX 11 ALWAYS YES!!!!!!!!!!!
            // DirectX 11 doesn't use methods like drawPrimitiveUp (DX9) or glVertex (OpenGL)
            virtual bool isHardwareBufferRecommend(const scene::IMeshBuffer* mb) { return true; }

            void InitDrawStates(CVulkanHardwareBuffer* mb, scene::E_PRIMITIVE_TYPE pType = scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES);

            void drawMeshBuffer(const scene::IMeshBuffer* mb, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/) override;

            //! Draw hardware buffer
            virtual void drawHardwareBuffer(IHardwareBuffer *HWBuffer, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr);

            //! draw
            virtual void drawHardwareBuffer(IHardwareBuffer* vertices,
                IHardwareBuffer* indices, E_VERTEX_TYPE vType = EVT_STANDARD,
                scene::E_PRIMITIVE_TYPE pType = scene::EPT_TRIANGLES,
                E_INDEX_TYPE iType = EIT_16BIT, u32 numInstances = 0);

            //! Draw automatically using stream output buffers
            virtual void drawAuto(IHardwareBuffer* vertices, E_VERTEX_TYPE vType = EVT_STANDARD,
                scene::E_PRIMITIVE_TYPE pType = scene::EPT_TRIANGLES);

            //! draws a vertex primitive list
            virtual void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType);

            //! draws a vertex primitive list in 2d
            virtual void draw2DVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType);

            //! initialises the Vulkan API
            bool initDriver(void* param);

            //! \return Returns the name of the video driver. Example: In case of the Vulkan
            //! driver, it would return "Vulkan".
            virtual const wchar_t* getName() const;

            //! deletes all dynamic lights there are
            virtual void deleteAllDynamicLights();

            //! adds a dynamic light, returning an index to the light
            //! \param light: the light data to use to create the light
            //! \return An index to the light, or -1 if an error occurs
            virtual s32 addDynamicLight(const SLight& light);

            //! Turns a dynamic light on or off
            //! \param lightIndex: the index returned by addDynamicLight
            //! \param turnOn: true to turn the light on, false to turn it off
            virtual void turnLightOn(s32 lightIndex, bool turnOn);

            //! returns the maximal amount of dynamic lights the device can handle
            virtual u32 getMaximalDynamicLightAmount() const;

            //! Sets the dynamic ambient light color. The default color is
            //! (0,0,0,0) which means it is dark.
            //! \param color: New color of the ambient light.
            virtual void setAmbientLight(const SColorf& color);

            //! Draws a shadow volume into the stencil buffer.
            virtual void drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail = true, u32 debugDataVisible = 0);

            //! Fills the stencil shadow with color.
            virtual void drawStencilShadow(bool clearStencilBuffer = false,
                video::SColor leftUpEdge = video::SColor(0, 0, 0, 0),
                video::SColor rightUpEdge = video::SColor(0, 0, 0, 0),
                video::SColor leftDownEdge = video::SColor(0, 0, 0, 0),
                video::SColor rightDownEdge = video::SColor(0, 0, 0, 0));

            //! Returns the maximum amount of primitives (mostly vertices) which
            //! the device is able to render with one drawIndexedTriangleList
            //! call.
            virtual u32 getMaximalPrimitiveCount() const;

            //! Enables or disables a texture creation flag.
            virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled);

            //! Sets the fog mode.
            virtual void setFog(SColor color, E_FOG_TYPE fogType, f32 start,
                f32 end, f32 density, bool pixelFog, bool rangeFog);

            //! Only used by the internal engine. Used to notify the driver that
            //! the window was resized.
            virtual void OnResize(const core::dimension2d<u32>& size);

            //! Can be called by an IMaterialRenderer to make its work easier.
            virtual void setBasicRenderStates(const SMaterial& material, const SMaterial& lastMaterial,
                bool resetAllRenderstates);

            //! Returns type of video driver
            virtual E_DRIVER_TYPE getDriverType() const;

            //! Returns the transformation set by setTransform
            virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const;

            //! Sets a vertex shader constant.
            virtual void setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount = 1);

            //! Sets a pixel shader constant.
            virtual void setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount = 1);

            //! Sets a constant for the vertex shader based on a name.
            virtual bool setVertexShaderConstant(const c8* name, const f32* floats, int count);

            //! Sets a constant for the pixel shader based on a name.
            virtual bool setPixelShaderConstant(const c8* name, const f32* floats, int count);

            //! Bool interface for the above.
            virtual bool setVertexShaderConstant(const c8* name, const bool* bools, int count)
            {
                return false;
            }

            //! Int interface for the above.
            virtual bool setVertexShaderConstant(const c8* name, const s32* ints, int count)
            {
                return false;
            }

            //! Bool interface for the above.
            virtual bool setPixelShaderConstant(const c8* name, const bool* bools, int count)
            {
                return false;
            }

            //! Int interface for the above.
            virtual bool setPixelShaderConstant(const c8* name, const s32* ints, int count)
            {
                return false;
            }

            //! Set hardware buffer for stream output stage
            virtual bool setStreamOutputBuffer(IHardwareBuffer* buffer);

            //! Returns a pointer to the IVideoDriver interface. (Implementation for
            //! IMaterialRendererServices)
            virtual IVideoDriver* getVideoDriver();

            //! Creates a render target texture.
            virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32>& size,
                const io::path& name, const ECOLOR_FORMAT format = ECF_UNKNOWN);

            //! Clears the ZBuffer.
            virtual void clearZBuffer(f32 clearDepth = 1.f, u8 clearStencil = 0);

            //! Returns an image created from the last rendered frame.
            virtual IImage* createScreenShot(video::ECOLOR_FORMAT format = video::ECF_UNKNOWN, video::E_RENDER_TARGET target = video::ERT_FRAME_BUFFER);

            //! Set/unset a clipping plane.
            virtual bool setClipPlane(u32 index, const core::plane3df& plane, bool enable = false);

            //! Enable/disable a clipping plane.
            virtual void enableClipPlane(u32 index, bool enable);

            //! Used by CVulkanMaterialRenderer to get clip plane and status
            virtual void getClipPlane(u32 index, core::plane3df& plane, bool& enable);

            //! Returns the graphics card vendor name.
            virtual core::stringc getVendorInfo() { return VendorName; }

            //! Enable the 2d override material
            virtual void enableMaterial2D(bool enable = true);

            //! Get the current color format of the color buffer
            /** \return Color format of the color buffer. */
            virtual ECOLOR_FORMAT getColorFormat() const;

            //! Returns the maximum texture size supported.
            virtual core::dimension2du getMaxTextureSize() const;

            const ITexture* getCurrentTexture(irr::u8 idx) { return CurrentTexture[idx]; }

            //! Create a implementation dependant hardware buffer
            virtual IHardwareBuffer* createHardwareBuffer(E_HARDWARE_BUFFER_TYPE type, E_HARDWARE_BUFFER_ACCESS accessType,
                u32 size, u32 flags = 0, const void* initialData = 0);

            video::VertexDeclaration* createVertexDeclaration();

            //! Register a new vertex type
            virtual E_VERTEX_TYPE registerVertexType(core::array<SVertexElement>& elements);

            //! Check multisample quality levels
            virtual u32 queryMultisampleLevels(ECOLOR_FORMAT format, u32 numSamples) const;
            bool SyncShaderConstant(CVulkanHardwareBuffer* HWBuffer, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr);
            IShader * createShader(ShaderInitializerEntry * shaderCreateInfo) override;
            virtual void useShader(IShader* gpuProgram);

            SMaterial& GetMaterial() { return Material; }
            SMaterial& GetLastMaterial() { return LastMaterial; }

            void HandleDeviceLost();

            void AddResourceObject(CVulkanDeviceResource* res)
            {
                ResourceList.insert(res);
            }

            void RemoveResourceObject(CVulkanDeviceResource* res)
            {
                ResourceList.erase(res);
            }

            bool IsDebugMode() const { return OnDebugMode; }

            /**
            * Returns a set of command buffer semaphores depending on the provided sync mask.
            *
            * @param[in]	deviceIdx	Index of the device to get the semaphores for.
            * @param[in]	syncMask	Mask that has a bit enabled for each command buffer to retrieve the semaphore for.
            *							If the command buffer is not currently executing, semaphore won't be returned.
            * @param[out]	semaphores	List containing all the required semaphores. Semaphores are tightly packed at the
            *							beginning of the array. Must be able to hold at least _MAX_UNIQUE_QUEUES entries.
            * @param[out]	count		Number of semaphores provided in the @p semaphores array.
            */
            void getSyncSemaphores(u32 deviceIdx, u32 syncMask, VulkanSemaphore** semaphores, u32& count);

            /** Returns the primary device that supports swap chain present operations. */
            VulkanDevice* _getPrimaryDevice() const { return mPrimaryDevices; }
            VulkanDevice* _getDevice(u8 idx) const { return mDevices[idx]; }
            vk::DispatchLoaderDynamic& _getVulkanDispatcher() { return VulkanDispatcherExt; }
            u32 _getNumDevices() const { return mNumDevices; }
            VkInstance _getInstance() { return mInstance; }
            VulkanSwapChain* _getSwapChain();

            SIrrlichtCreationParameters mCreateParams; // ToDo: drop this

            CVulkanTexture* GetDummyTexture() { return blankTexture; }
            CVulkanHardwareBuffer* GetDummyStorageBuffer() { return mDummyStorageBuffer; }
            VkSampler GetDummySampler() { return mDummySampler; }

            // ToDo: dedicate command buffer
            VulkanCommandBuffer* GetCommandBuffer() { return mMainCommandBuffer; }

            u32 GetFrameID() const { return mFrameID; }

        private:

            void initialize(void* param);

            SMaterial Material, LastMaterial;
            IImage* blankImage = nullptr;
            CVulkanTexture* blankTexture = nullptr;
            const ITexture* CurrentTexture[MATERIAL_MAX_TEXTURES];
            core::matrix4 Matrices[ETS_COUNT];			// matrizes of the 3d mode we need to restore when we switch back from the 2d mode.
            std::wstring DriverAndFeatureName;

            // Vulkan objects
            VkInstance mInstance = nullptr;

#if VULKAN_DEBUG_MODE
            VkDebugReportCallbackEXT mDebugCallback;
#endif

            VulkanDevice* mDevices[_MAX_DEVICES];
            VulkanDevice* mPrimaryDevices;
            VkSampler mDummySampler = VK_NULL_HANDLE;
            CVulkanHardwareBuffer* DynamicHardwareBuffer[(s32)E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE];
            CVulkanHardwareBuffer* mDummyStorageBuffer = nullptr;
            VulkanCommandBuffer* mMainCommandBuffer = nullptr;
            CVulkanPlatform* mPlatform;
            u32 mNumDevices;
            vk::DispatchLoaderDynamic VulkanDispatcherExt;
            std::map<CVulkanGLSLProgram*, std::map<CVulkanVertexDeclaration*, VulkanGraphicsPipelineState*>> mPipelines;

            irr::video::CVulkanGLSLProgram* m_defaultShader[(s32)E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE];

            core::dimension2d<u32> CurrentRendertargetSize;

            // Just one clip plane for now
            core::array<core::plane3df> ClipPlanes;

            core::rect<s32>* SceneSourceRect = nullptr;

            // Storing Resources
            std::set<CVulkanDeviceResource*> ResourceList;

            //! All the lights that have been requested; a hardware limited
            //! number of them will be used at once.
            struct RequestedLight
            {
                RequestedLight(SLight const & lightData)
                    : LightData(lightData), HardwareLightIndex(-1), DesireToBeOn(true) { }

                SLight	LightData;
                s32	HardwareLightIndex;
                bool	DesireToBeOn;
            };
            core::array<RequestedLight> RequestedLights;
            u32 MaxActiveLights;

            core::stringc VendorName;
            u16 VendorID;

            u32 MaxTextureUnits;
            u32 MaxUserClipPlanes;
            f32 MaxLightDistance;
            s32 LastSetLight;
            u32 mFrameID;

            ECOLOR_FORMAT ColorFormat;
            E_DEVICE_TYPE DeviceType;

            bool ClipPlaneEnabled[3];
            bool ResetRenderStates : 1;         // bool to make all renderstates be reseted if set.
            bool Transformation3DChanged : 1;
            bool AlphaToCoverageSupport : 1;
            bool OnDebugMode : 1;

        public:
            bool ResetBlending : 1;

        protected:

            void createMaterialRenderers();

            void draw2D3DVertexPrimitiveList(const void* vertices,
                u32 vertexCount, const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType, bool is3D);

            //! sets the needed renderstates
            bool setRenderStates3DMode();

            //! sets the needed renderstates
            void setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel) override;

            bool setActiveTexture(u32 stage, const video::ITexture* texture);

            //! returns a device dependent texture from a software surface (IImage)
            //! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
            virtual video::ITexture* createDeviceDependentTexture(IImage* surface, const io::path& name);

            //! Check if a proper depth buffer for the RTT is available, otherwise create it.
            void checkDepthBuffer(ITexture* tex);

            //! returns the current size of the screen or rendertarget
            virtual const core::dimension2d<u32>& getCurrentRenderTargetSize() const;

            //! reallocate dynamic buffers
            virtual bool reallocateDynamicBuffers(u32 vertexBufferSize, u32 indexBufferSize);

            //! upload dynamic vertex and index data to GPU
            virtual bool uploadVertexData(const void* vertices, u32 vertexCount,
                const void* indexList, u32 indexCount,
                E_VERTEX_TYPE vType, E_INDEX_TYPE iType);

            //! handle screen resize
            void reset();
        };
    }
}

#endif // __C_VIDEO_VULKAN_H_INCLUDED__