// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_NULL_H_INCLUDED__
#define __C_VIDEO_NULL_H_INCLUDED__

#include <unordered_map>

#include "IVideoDriver.h"
#include "IFileSystem.h"
#include "IImagePresenter.h"
#include "IGPUProgrammingServices.h"
#include "irrArray.h"
#include "irrString.h"
#include "irrMap.h"
#include "IAttributes.h"
#include "IMesh.h"
#include "IMeshBuffer.h"
#include "IMeshSceneNode.h"
#include "CFPSCounter.h"
#include "S3DVertex.h"
#include "SVertexIndex.h"
#include "SLight.h"
#include "SExposedVideoData.h"
#include "matrix4.h"
#include "IHardwareBuffer.h"
#include "SVertexElement.h"
#include "SVertexIndex.h"
#include "SVertexManipulator.h"
#include "RenderEngines/General/CIrrShader.h"

#include "standard/client/DataSource_Standard.h"

#ifdef _MSC_VER
#pragma warning( disable: 4996)
#endif

namespace irr
{
namespace io
{
    class IWriteFile;
    class IReadFile;
} // end namespace io

namespace scene
{
    class IMesh;
    class IMeshBuffer;
    class ISceneNode;
};

namespace video
{
    class IHardwareBuffer;
    struct IShader;
    class IImageLoader;
    class IImageWriter;

    class  IRRLICHT_API  CNullDriver : public IVideoDriver, public IGPUProgrammingServices
    {
    public:

        //! constructor
        CNullDriver(io::IFileSystem* io, const core::dimension2d<u32>& screenSize);

        //! destructor
        virtual ~CNullDriver();

        virtual bool beginScene(bool backBuffer=true, bool zBuffer=true,
                SColor color=SColor(255,0,0,0),
                const SExposedVideoData& videoData=SExposedVideoData(),
                core::rect<s32>* sourceRect=0) _IRR_OVERRIDE_;

        virtual bool endScene() _IRR_OVERRIDE_;

        //! Disable a feature of the driver.
        virtual void disableFeature(E_VIDEO_DRIVER_FEATURE feature, bool flag=true) _IRR_OVERRIDE_;

        //! queries the features of the driver, returns true if feature is available
        virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

        //! Get attributes of the actual video driver
        const io::IAttributes& getDriverAttributes() const;

        //! sets transformation
        virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat) _IRR_OVERRIDE_;

        //! Retrieve the number of image loaders
        virtual u32 getImageLoaderCount() const;

        //! Retrieve the given image loader
        virtual IImageLoader* getImageLoader(u32 n) _IRR_OVERRIDE_;

        //! Retrieve the number of image writers
        virtual u32 getImageWriterCount() const;

        //! Retrieve the given image writer
        virtual IImageWriter* getImageWriter(u32 n) _IRR_OVERRIDE_;

        //! sets a material
        virtual void setMaterial(const SMaterial& material) _IRR_OVERRIDE_;

        //! loads a Texture
        virtual ITexture* getTexture(const io::path& filename) _IRR_OVERRIDE_;

        //! loads a Texture
        virtual ITexture* getTexture(io::IReadFile* file) _IRR_OVERRIDE_;

        //! Returns a texture by index
        virtual ITexture* getTextureByIndex(u32 index) _IRR_OVERRIDE_;

        //! Returns amount of textures currently loaded
        virtual u32 getTextureCount() const;

        //! Renames a texture
        virtual void renameTexture(ITexture* texture, const io::path& newName) _IRR_OVERRIDE_;

        //! creates a Texture
        virtual ITexture* addTexture(const core::dimension2d<u32>& size, const io::path& name, ECOLOR_FORMAT format = ECF_A8R8G8B8) _IRR_OVERRIDE_;

        virtual bool setRenderTargetEx(IRenderTarget* target, u16 clearFlag, SColor clearColor = SColor(255, 0, 0, 0),
            f32 clearDepth = 1.f, u8 clearStencil = 0, core::array<core::recti>* scissors = nullptr) _IRR_OVERRIDE_;

        virtual bool setRenderTarget(ITexture* texture, u16 clearFlag, SColor clearColor = SColor(255, 0, 0, 0),
            f32 clearDepth = 1.f, u8 clearStencil = 0) _IRR_OVERRIDE_;

        //! sets a viewport
        virtual void setViewPort(const core::rect<s32>& area) _IRR_OVERRIDE_;

        //! gets the area of the current viewport
        virtual const core::rect<s32>& getViewPort() const;

        //! draws a vertex primitive list
        virtual void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType=EVT_STANDARD, scene::E_PRIMITIVE_TYPE pType=scene::EPT_TRIANGLES, E_INDEX_TYPE iType=EIT_16BIT) _IRR_OVERRIDE_;

        //! draws a vertex primitive list in 2d
        virtual void draw2DVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType=EVT_STANDARD, scene::E_PRIMITIVE_TYPE pType=scene::EPT_TRIANGLES, E_INDEX_TYPE iType=EIT_16BIT) _IRR_OVERRIDE_;

        //! Draws a 3d line.
        virtual void draw3DLine(const core::vector3df& start,
            const core::vector3df& end, SColor color = SColor(255,255,255,255)) _IRR_OVERRIDE_;

        //! Draws a 3d triangle.
        virtual void draw3DTriangle(const core::triangle3df& triangle,
            SColor color = SColor(255,255,255,255)) _IRR_OVERRIDE_;

        //! Draws a 3d axis aligned box.
        virtual void draw3DBox(const core::aabbox3d<f32>& box,
            SColor color = SColor(255,255,255,255)) _IRR_OVERRIDE_;

        //! draws an 2d image
        virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos) _IRR_OVERRIDE_;

        //! draws a set of 2d images, using a color and the alpha
        /** channel of the texture if desired. The images are drawn
        beginning at pos and concatenated in one line. All drawings
        are clipped against clipRect (if != 0).
        The subtextures are defined by the array of sourceRects
        and are chosen by the indices given.
        \param texture: Texture to be drawn.
        \param pos: Upper left 2d destination position where the image will be drawn.
        \param sourceRects: Source rectangles of the image.
        \param indices: List of indices which choose the actual rectangle used each time.
        \param kerningWidth: offset on position
        \param clipRect: Pointer to rectangle on the screen where the image is clipped to.
        This pointer can be 0. Then the image is not clipped.
        \param color: Color with which the image is colored.
        Note that the alpha component is used: If alpha is other than 255, the image will be transparent.
        \param useAlphaChannelOfTexture: If true, the alpha channel of the texture is
        used to draw the image. */
        virtual void draw2DImageBatch(const video::ITexture* texture,
                const core::position2d<s32>& pos,
                const core::array<core::rect<s32> >& sourceRects,
                const core::array<s32>& indices,
                s32 kerningWidth = 0,
                const core::rect<s32>* clipRect = 0,
                SColor color=SColor(255,255,255,255),
                bool useAlphaChannelOfTexture=false) _IRR_OVERRIDE_;

        //! Draws a set of 2d images, using a color and the alpha channel of the texture.
        /** All drawings are clipped against clipRect (if != 0).
        The subtextures are defined by the array of sourceRects and are
        positioned using the array of positions.
        \param texture Texture to be drawn.
        \param pos Array of upper left 2d destinations where the images
        will be drawn.
        \param sourceRects Source rectangles of the image.
        \param clipRect Pointer to rectangle on the screen where the
        images are clipped to.
        If this pointer is 0 then the image is not clipped.
        \param color Color with which the image is drawn.
        Note that the alpha component is used. If alpha is other than
        255, the image will be transparent.
        \param useAlphaChannelOfTexture: If true, the alpha channel of
        the texture is used to draw the image. */
        virtual void draw2DImageBatch(const video::ITexture* texture,
                const core::array<core::position2d<s32> >& positions,
                const core::array<core::rect<s32> >& sourceRects,
                const core::rect<s32>* clipRect=0,
                SColor color=SColor(255,255,255,255),
                bool useAlphaChannelOfTexture=false) _IRR_OVERRIDE_;

        //! Draws a 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
        virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
            const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
            SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false) _IRR_OVERRIDE_;

        //! Draws a part of the texture into the rectangle.
        virtual void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
            const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
            const video::SColor* const colors=0, bool useAlphaChannelOfTexture=false) _IRR_OVERRIDE_;

        //! Draws a 2d rectangle
        virtual void draw2DRectangle(SColor color, const core::rect<s32>& pos, const core::rect<s32>* clip = 0, bool filled = true) _IRR_OVERRIDE_;

        //! Draws a 2d rectangle with a gradient.
        virtual void draw2DRectangle(const core::rect<s32>& pos,
            SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
            const core::rect<s32>* clip = 0, bool filled = true) _IRR_OVERRIDE_;

        //! Draws the outline of a 2d rectangle
        virtual void draw2DRectangleOutline(const core::recti& pos, SColor color=SColor(255,255,255,255)) _IRR_OVERRIDE_;

        //! Draws a 2d line.
        virtual void draw2DLine(const core::position2d<s32>& start,
                    const core::position2d<s32>& end,
                    SColor color=SColor(255,255,255,255)) _IRR_OVERRIDE_;

        //! Draws a pixel
        virtual void drawPixel(u32 x, u32 y, const SColor & color) _IRR_OVERRIDE_;

        //! Draws a non filled concyclic reqular 2d polyon.
        virtual void draw2DPolygon(core::position2d<s32> center,
            f32 radius, video::SColor Color, s32 vertexCount) _IRR_OVERRIDE_;

        virtual void setFog(SColor color=SColor(0,255,255,255),
                E_FOG_TYPE fogType=EFT_FOG_LINEAR,
                f32 start=50.0f, f32 end=100.0f, f32 density=0.01f,
                bool pixelFog=false, bool rangeFog=false) _IRR_OVERRIDE_;

        virtual void getFog(SColor& color, E_FOG_TYPE& fogType,
                f32& start, f32& end, f32& density,
                bool& pixelFog, bool& rangeFog) _IRR_OVERRIDE_;

        //! get color format of the current color buffer
        virtual ECOLOR_FORMAT getColorFormat() const;

        //! get screen size
        virtual const core::dimension2d<u32>& getScreenSize() const;

        //! get current render target
        IRenderTarget* getCurrentRenderTarget() const;

        //! get render target size
        virtual const core::dimension2d<u32>& getCurrentRenderTargetSize() const;

        // get current frames per second value
        virtual s32 getFPS() const;

        //! returns amount of primitives (mostly triangles) were drawn in the last frame.
        //! very useful method for statistics.
        virtual u32 getPrimitiveCountDrawn( u32 param = 0 ) const;

        //! deletes all dynamic lights there are
        virtual void deleteAllDynamicLights() _IRR_OVERRIDE_;

        //! adds a dynamic light, returning an index to the light
        //! \param light: the light data to use to create the light
        //! \return An index to the light, or -1 if an error occurs
        virtual s32 addDynamicLight(const SLight& light) _IRR_OVERRIDE_;

        //! Turns a dynamic light on or off
        //! \param lightIndex: the index returned by addDynamicLight
        //! \param turnOn: true to turn the light on, false to turn it off
        virtual void turnLightOn(s32 lightIndex, bool turnOn) _IRR_OVERRIDE_;

        //! returns the maximal amount of dynamic lights the device can handle
        virtual u32 getMaximalDynamicLightAmount() const;

        //! \return Returns the name of the video driver. Example: In case of the DIRECT3D8
        //! driver, it would return "Direct3D8.1".
        virtual const wchar_t* getName() const;

        //! Sets the dynamic ambient light color. The default color is
        //! (0,0,0,0) which means it is dark.
        //! \param color: New color of the ambient light.
        virtual void setAmbientLight(const SColorf& color) _IRR_OVERRIDE_;

        //! Adds an external image loader to the engine.
        virtual void addExternalImageLoader(IImageLoader* loader) _IRR_OVERRIDE_;

        //! Adds an external image writer to the engine.
        virtual void addExternalImageWriter(IImageWriter* writer) _IRR_OVERRIDE_;

        //! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
        //! this: Frist, draw all geometry. Then use this method, to draw the shadow
        //! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
        virtual void drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail=true, u32 debugDataVisible=0) _IRR_OVERRIDE_;

        //! Fills the stencil shadow with color. After the shadow volume has been drawn
        //! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
        //! to draw the color of the shadow.
        virtual void drawStencilShadow(bool clearStencilBuffer=false,
            video::SColor leftUpEdge = video::SColor(0,0,0,0),
            video::SColor rightUpEdge = video::SColor(0,0,0,0),
            video::SColor leftDownEdge = video::SColor(0,0,0,0),
            video::SColor rightDownEdge = video::SColor(0,0,0,0)) _IRR_OVERRIDE_;

        //! Returns current amount of dynamic lights set
        //! \return Current amount of dynamic lights set
        virtual u32 getDynamicLightCount() const;

        //! Returns light data which was previously set with IVideDriver::addDynamicLight().
        //! \param idx: Zero based index of the light. Must be greater than 0 and smaller
        //! than IVideoDriver()::getDynamicLightCount.
        //! \return Light data.
        virtual const SLight& getDynamicLight(u32 idx) const;

        //! Removes a texture from the texture cache and deletes it, freeing lot of
        //! memory.
        virtual void removeTexture(ITexture* texture) _IRR_OVERRIDE_;

        //! Removes all texture from the texture cache and deletes them, freeing lot of
        //! memory.
        virtual void removeAllTextures() _IRR_OVERRIDE_;

        //! Creates a render target texture.
        virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32>& size,
            const io::path& name, const ECOLOR_FORMAT format = ECF_UNKNOWN, u8 sampleCount = 0) _IRR_OVERRIDE_;

        //! Creates an 1bit alpha channel of the texture based of an color key.
        virtual void makeColorKeyTexture(video::ITexture* texture, video::SColor color, bool zeroTexels) const;

        //! Creates an 1bit alpha channel of the texture based of an color key position.
        virtual void makeColorKeyTexture(video::ITexture* texture, core::position2d<s32> colorKeyPixelPos, bool zeroTexels) const;

        //! Creates a normal map from a height map texture.
        //! \param amplitude: Constant value by which the height information is multiplied.
        virtual void makeNormalMapTexture(video::ITexture* texture, f32 amplitude=1.0f) const;

        //! Returns the maximum amount of primitives (mostly vertices) which
        //! the device is able to render with one drawIndexedTriangleList
        //! call.
        virtual u32 getMaximalPrimitiveCount() const;

        //! Enables or disables a texture creation flag.
        virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled) _IRR_OVERRIDE_;

        //! Returns if a texture creation flag is enabled or disabled.
        virtual bool getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) const;

        //! Creates a software image from a file.
        virtual IImage* createImageFromFile(const io::path& filename) _IRR_OVERRIDE_;

        //! Creates a software image from a file.
        virtual IImage* createImageFromFile(io::IReadFile* file) _IRR_OVERRIDE_;

        //! Creates a software image from a byte array.
        /** \param useForeignMemory: If true, the image will use the data pointer
        directly and own it from now on, which means it will also try to delete [] the
        data when the image will be destructed. If false, the memory will by copied. */
        virtual IImage* createImageFromData(ECOLOR_FORMAT format,
            const core::dimension2d<u32>& size, void *data,
            bool ownForeignMemory=true, bool deleteForeignMemory = true) _IRR_OVERRIDE_;

        //! Creates an empty software image.
        virtual IImage* createImage(ECOLOR_FORMAT format, const core::dimension2d<u32>& size) _IRR_OVERRIDE_;


        //! Creates a software image from another image.
        virtual IImage* createImage(ECOLOR_FORMAT format, IImage *imageToCopy) _IRR_OVERRIDE_;

        //! Creates a software image from part of another image.
        virtual IImage* createImage(IImage* imageToCopy,
                const core::position2d<s32>& pos,
                const core::dimension2d<u32>& size) _IRR_OVERRIDE_;

        //! Creates a software image from part of a texture.
        virtual IImage* createImage(ITexture* texture,
                const core::position2d<s32>& pos,
                const core::dimension2d<u32>& size) _IRR_OVERRIDE_;

        //! Draws a mesh buffer
        virtual void drawMeshBuffer(const scene::IMeshBuffer* mb, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr) _IRR_OVERRIDE_;

        virtual void drawMeshBuffer3DBox(const scene::IMeshBuffer* mb) {}

        //! Draws the normals of a mesh buffer
        virtual void drawMeshBufferNormals(const scene::IMeshBuffer* mb, f32 length=10.f, SColor color=0xffffffff) _IRR_OVERRIDE_;

        //! Draws a mesh buffer
        virtual void CreateHardwareBuffer(const scene::IMeshBuffer* mb) _IRR_OVERRIDE_;

        bool setActiveTexture(u32 stage, const video::ITexture* texture) { return false;  }
        virtual bool setRenderStates3DMode() { return false; }

        // Rework Shader
        //template<typename TValue>
        //ShaderDataBufferElementObject<TValue> CreateShaderVariableObject(video::IShader* shader, const char* variable)
        //{
        //    auto desc = shader->GetGPUVariableDesc(name);
        //    if (!desc)
        //        return nullptr;
        //
        //    return new ShaderDataBufferElementObject<TValue>(desc);
        //}

    protected:

        //! sets the needed renderstates
        virtual void setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel) {}

        //! Gets hardware buffer link from a meshbuffer (may create or update buffer)
        virtual IHardwareBuffer *getBufferLink(const scene::IMeshBuffer* mb);

        //! updates hardware buffer if needed  (only some drivers can)
        virtual bool updateHardwareBuffer(IHardwareBuffer *HWBuffer) {return false;}

        //! Draw hardware buffer (only some drivers can)
        virtual void drawHardwareBuffer(IHardwareBuffer *HWBuffer, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr) {}

        //! Delete hardware buffer
        virtual void deleteHardwareBuffer(IHardwareBuffer *HWBuffer);

        //! Create hardware buffer from mesh (only some drivers can)
        virtual IHardwareBuffer *createHardwareBuffer(const scene::IMeshBuffer* mb) {return 0;}

        void ThrowIfNotCoreThread();
        void ThrowIfCoreThread();

    public:
        //! Update all hardware buffers, remove unused ones
        virtual void updateAllHardwareBuffers();

        //! Remove hardware buffer
        virtual void removeHardwareBuffer(const scene::IMeshBuffer* mb);

        //! Remove all hardware buffers
        virtual void removeAllHardwareBuffers();

        //! is vbo recommended on this mesh?
        virtual bool isHardwareBufferRecommend(const scene::IMeshBuffer* mb);

        //! Create occlusion query.
        /** Use node for identification and mesh for occlusion test. */
        virtual void addOcclusionQuery(scene::ISceneNode* node,
                const scene::IMesh* mesh=0);

        //! Remove occlusion query.
        virtual void removeOcclusionQuery(scene::ISceneNode* node);

        //! Remove all occlusion queries.
        virtual void removeAllOcclusionQueries();

        //! Run occlusion query. Draws mesh stored in query.
        /** If the mesh shall not be rendered visible, use
        overrideMaterial to disable the color and depth buffer. */
        virtual void runOcclusionQuery(scene::ISceneNode* node, bool visible=false);

        //! Run all occlusion queries. Draws all meshes stored in queries.
        /** If the meshes shall not be rendered visible, use
        overrideMaterial to disable the color and depth buffer. */
        virtual void runAllOcclusionQueries(bool visible=false);

        //! Update occlusion query. Retrieves results from GPU.
        /** If the query shall not block, set the flag to false.
        Update might not occur in this case, though */
        virtual void updateOcclusionQuery(scene::ISceneNode* node, bool block=true);

        //! Update all occlusion queries. Retrieves results from GPU.
        /** If the query shall not block, set the flag to false.
        Update might not occur in this case, though */
        virtual void updateAllOcclusionQueries(bool block=true);

        //! Return query result.
        /** Return value is the number of visible pixels/fragments.
        The value is a safe approximation, i.e. can be larger than the
        actual value of pixels. */
        virtual u32 getOcclusionQueryResult(scene::ISceneNode* node) const;

        //! Only used by the engine internally.
        /** Used to notify the driver that the window was resized. */
        virtual void OnResize(const core::dimension2d<u32>& size);

        //! Adds a new material renderer to the video device.
        virtual s32 addMaterialRenderer(IMaterialRenderer* renderer,
                const char* name = 0);

        //! Returns driver and operating system specific data about the IVideoDriver.
        virtual const SExposedVideoData& getExposedVideoData();

        //! Returns type of video driver
        virtual E_DRIVER_TYPE getDriverType() const;

        //! Returns the transformation set by setTransform
        virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const;

        //! Returns pointer to the IGPUProgrammingServices interface.
        virtual IGPUProgrammingServices* getGPUProgrammingServices();

        //! Adds a new material renderer to the VideoDriver, using pixel and/or
        //! vertex shaders to render geometry.
        virtual s32 addShaderMaterial(const c8* vertexShaderProgram = 0,
            const c8* pixelShaderProgram = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData=0);

        //! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the
        //! programs from files.
        virtual s32 addShaderMaterialFromFiles(io::IReadFile* vertexShaderProgram = 0,
            io::IReadFile* pixelShaderProgram = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData=0);

        //! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the
        //! programs from files.
        virtual s32 addShaderMaterialFromFiles(const io::path& vertexShaderProgramFileName,
            const io::path& pixelShaderProgramFileName,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData=0);

        //! Returns pointer to material renderer or null
        virtual IMaterialRenderer* getMaterialRenderer(u32 idx);

        //! Returns amount of currently available material renderers.
        virtual u32 getMaterialRendererCount() const;

        //! Returns name of the material renderer
        virtual const char* getMaterialRendererName(u32 idx) const;

        //! Adds a new material renderer to the VideoDriver, based on a high level shading
        //! language. Currently only HLSL in D3D9 is supported.
        virtual s32 addHighLevelShaderMaterial(
            const c8* vertexShaderProgram,
            const c8* vertexShaderEntryPointName = 0,
            E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
            const c8* pixelShaderProgram = 0,
            const c8* pixelShaderEntryPointName = 0,
            E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
            const c8* geometryShaderProgram = 0,
            const c8* geometryShaderEntryPointName = "main",
            E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
            scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
            scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
            u32 verticesOut = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData = 0, E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);

        //! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
        //! but tries to load the programs from files.
        virtual s32 addHighLevelShaderMaterialFromFiles(
            const io::path& vertexShaderProgramFile,
            const c8* vertexShaderEntryPointName = "main",
            E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
            const io::path& pixelShaderProgramFile = "",
            const c8* pixelShaderEntryPointName = "main",
            E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
            const io::path& geometryShaderProgramFileName="",
            const c8* geometryShaderEntryPointName = "main",
            E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
            scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
            scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
            u32 verticesOut = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData = 0, E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);

        //! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
        //! but tries to load the programs from files.
        virtual s32 addHighLevelShaderMaterialFromFiles(
            io::IReadFile* vertexShaderProgram,
            const c8* vertexShaderEntryPointName = "main",
            E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
            io::IReadFile* pixelShaderProgram = 0,
            const c8* pixelShaderEntryPointName = "main",
            E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
            io::IReadFile* geometryShaderProgram= 0,
            const c8* geometryShaderEntryPointName = "main",
            E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
            scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
            scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
            u32 verticesOut = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData = 0, E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);

        //! Returns a pointer to the mesh manipulator.
        virtual scene::IMeshManipulator* getMeshManipulator();

        //! Clears the ZBuffer.
        virtual void clearZBuffer(f32 clearDepth = 1.f, u8 clearStencil = 0);

        //! Returns an image created from the last rendered frame.
        virtual IImage* createScreenShot(video::ECOLOR_FORMAT format=video::ECF_UNKNOWN, video::E_RENDER_TARGET target=video::ERT_FRAME_BUFFER);

        //! Writes the provided image to disk file
        virtual bool writeImageToFile(IImage* image, const io::path& filename, u32 param = 0);

        //! Writes the provided image to a file.
        virtual bool writeImageToFile(IImage* image, io::IWriteFile * file, u32 param = 0);

        //! Sets the name of a material renderer.
        virtual void setMaterialRendererName(s32 idx, const char* name);

        //! Creates material attributes list from a material, usable for serialization and more.
        virtual io::IAttributes* createAttributesFromMaterial(const video::SMaterial& material,
            io::SAttributeReadWriteOptions* options=0);

        //! Fills an SMaterial structure from attributes.
        virtual void fillMaterialStructureFromAttributes(video::SMaterial& outMaterial, io::IAttributes* attributes);

        //! looks if the image is already loaded
        virtual video::ITexture* findTexture(const io::path& filename);

        //! Set/unset a clipping plane.
        //! There are at least 6 clipping planes available for the user to set at will.
        //! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
        //! \param plane: The plane itself.
        //! \param enable: If true, enable the clipping plane else disable it.
        virtual bool setClipPlane(u32 index, const core::plane3df& plane, bool enable=false);

        //! Enable/disable a clipping plane.
        //! There are at least 6 clipping planes available for the user to set at will.
        //! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
        //! \param enable: If true, enable the clipping plane else disable it.
        virtual void enableClipPlane(u32 index, bool enable);

        //! Returns the graphics card vendor name.
        virtual core::stringc getVendorInfo() {return "Not available on this driver.";}

        //! Set the minimum number of vertices for which a hw buffer will be created
        /** \param count Number of vertices to set as minimum. */
        virtual void setMinHardwareBufferVertexCount(u32 count);

        //! Get the global Material, which might override local materials.
        /** Depending on the enable flags, values from this Material
        are used to override those of local materials of some
        meshbuffer being rendered. */
        virtual SOverrideMaterial& getOverrideMaterial();

        //! Get the 2d override material for altering its values
        virtual SMaterial& getMaterial2D();

        //! Enable the 2d override material
        virtual void enableMaterial2D(bool enable=true);

        //! Only used by the engine internally.
        virtual void setAllowZWriteOnTransparent(bool flag) { AllowZWriteOnTransparent = flag; }
        bool isAllowZWriteOnTransparent() const { return AllowZWriteOnTransparent; }

        //! Returns the maximum texture size supported.
        virtual core::dimension2du getMaxTextureSize() const;

        //! Color conversion convenience function
        /** Convert an image (as array of pixels) from source to destination
        array, thereby converting the color format. The pixel size is
        determined by the color formats.
        \param sP Pointer to source
        \param sF Color format of source
        \param sN Number of pixels to convert, both array must be large enough
        \param dP Pointer to destination
        \param dF Color format of destination
        */
        virtual void convertColor(const void* sP, ECOLOR_FORMAT sF, s32 sN,
                void* dP, ECOLOR_FORMAT dF) const;

        //! deprecated method
        virtual ITexture* createRenderTargetTexture(const core::dimension2d<u32>& size,
                const c8* name=0);

        virtual bool checkDriverReset() {return false;}

        //! adds a surface, not loaded or created by the Irrlicht Engine
        void addTexture(video::ITexture* surface);

        IShader* GetActiveShader() override final
        {
            return ActiveGPUProgram;
        }

        video::VertexDeclaration* GetVertexDeclaration(irr::u32 id) override final;

        virtual video::VertexDeclaration* createVertexDeclaration() { return nullptr; }

        virtual bool SetShader(IShader* gpuProrgam)
        {
            _IRR_DEBUG_BREAK_IF(blockgpuprogramchange);
            if (ActiveGPUProgram == gpuProrgam)
                return ActiveGPUProgram != nullptr;

            ActiveGPUProgram = gpuProrgam;
            useShader(ActiveGPUProgram);

            return ActiveGPUProgram != nullptr;
        }

        //! opens the file and loads it into the surface
        video::ITexture* loadTextureFromFile(io::IReadFile* file, const io::path& hashName = "");

        //! returns a device dependent texture from a software surface (IImage)
        //! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
        virtual video::ITexture* createDeviceDependentTexture(IImage* surface, const io::path& name);

        s32 AddShaderModul(IShader* shader, s32 id = -1) override
        {
            if (id == -1 || id > ShaderModuls.size())
            {
                ShaderModuls.push_back(shader);
                return ShaderModuls.size() - 1;
            }

            ShaderModuls[id] = shader;
            return id;
        }

        IShader* GetShaderModul(s32 id) const override
        {
            return ShaderModuls[id];
        }

        // temporary
        virtual s32 getShaderVariableID(IShader*, const c8* name) { return -1; }
        virtual void useShader(IShader*) {}
        virtual void deleteShader(IShader*) {}

        //! Create render target.
        virtual IRenderTarget* addRenderTarget() { return nullptr; }

        //! Remove render target.
        virtual void removeRenderTarget(IRenderTarget* renderTarget);

        //! Remove all render targets.
        virtual void removeAllRenderTargets();

        // Inherited via CNullDriver
        virtual ITexture * addTextureCubemap(const io::path & name, IImage * imagePosX, IImage * imageNegX, IImage * imagePosY, IImage * imageNegY, IImage * imagePosZ, IImage * imageNegZ) override;
        virtual ITexture * addTextureArray(const io::path & name, irr::core::array<IImage*> images) override;

        E_RENDER_MODE GetCurrentRenderMode() const { return CurrentRenderMode; }
        SColorf const& GetAmbientLight() const { return AmbientLight; }

    protected:

        // Init Driver Globals
        bool initDriver();

        //! deletes all textures
        void deleteAllTextures();

        //! Creates a texture from a loaded IImage.
        virtual ITexture* addTexture(const io::path& name, IImage* image);

        //! checks triangle count and print warning if wrong
        bool checkPrimitiveCount(u32 prmcnt) const;

        // adds a material renderer and drops it afterwards. To be used for internal creation
        s32 addAndDropMaterialRenderer(IMaterialRenderer* m);

        //! deletes all material renderers
        void deleteMaterialRenders();

        // prints renderer version
        void printVersion();

        //! normal map lookup 32 bit version
        inline f32 nml32(int x, int y, int pitch, int height, s32 *p) const
        {
            if (x < 0) x = pitch-1; if (x >= pitch) x = 0;
            if (y < 0) y = height-1; if (y >= height) y = 0;
            return (f32)(((p[(y * pitch) + x])>>16) & 0xff);
        }

        //! normal map lookup 16 bit version
        inline f32 nml16(int x, int y, int pitch, int height, s16 *p) const
        {
            if (x < 0) x = pitch-1; if (x >= pitch) x = 0;
            if (y < 0) y = height-1; if (y >= height) y = 0;

            return (f32) getAverage ( p[(y * pitch) + x] );
        }

        struct SMaterialRenderer
        {
            core::stringc Name;
            IMaterialRenderer* Renderer;
        };

        struct SDummyTexture : public ITexture
        {
            SDummyTexture(const io::path& name) : ITexture(name), size(0,0) {};

            virtual void* lock(E_TEXTURE_LOCK_MODE mode=ETLM_READ_WRITE, u32 mipmapLevel=0, u32 arraySlice = 0) { return 0; };
            virtual void unlock(){}
            virtual const core::dimension2d<u32>& getOriginalSize() const { return size; }
            virtual const core::dimension2d<u32>& getSize() const { return size; }
            virtual E_DRIVER_TYPE getDriverType() const { return video::EDT_NULL; }
            virtual ECOLOR_FORMAT getColorFormat() const { return video::ECF_A1R5G5B5; };
            virtual u32 getPitch() const { return 0; }
            virtual void regenerateMipMapLevels(IImage* image = nullptr) {};
            core::dimension2d<u32> size;

            // Inherited via ITexture
            virtual void updateTexture(u32 level, u32 x, u32 y, u32 width, u32 height, const void* data) override;
        };

        std::map<irr::u32, VertexDeclaration*> VertexDeclarations;
        std::unordered_map<std::string, irr::video::ITexture*> Textures;

        core::array<SOccQuery*> OcclusionQueries;

        core::array<video::IImageLoader*> SurfaceLoader;
        core::array<video::IImageWriter*> SurfaceWriter;
        core::array<SLight> Lights;
        core::array<SMaterialRenderer> MaterialRenderers;
        core::array<IShader*> ShaderModuls;

        // Shared objects used with simplified IVideoDriver::setRenderTarget method with ITexture* param.
        IRenderTarget* SharedRenderTarget;
        core::array<ITexture*> SharedDepthTextures;

        IRenderTarget* CurrentRenderTarget;
        core::dimension2d<u32> CurrentRenderTargetSize;

        std::set<IRenderTarget*> RenderTargets;

        //core::map< const scene::IMeshBuffer* , IHardwareBuffer* > HWBufferMap;

        io::IFileSystem* FileSystem;

        //! mesh manipulator
        scene::IMeshManipulator* MeshManipulator;

        core::rect<s32> ViewPort;
        core::dimension2d<u32> ScreenSize;
        core::matrix4 TransformationMatrix;

        CFPSCounter FPSCounter;
        IShader* ActiveGPUProgram;

        u32 PrimitivesDrawn;
        u32 MinVertexCountForVBO;

        u32 TextureCreationFlags;

        f32 FogStart;
        f32 FogEnd;
        f32 FogDensity;
        SColor FogColor;
        E_RENDER_MODE CurrentRenderMode;
        SColorf AmbientLight;

        SExposedVideoData ExposedData;

        io::IAttributes* DriverAttributes;

        SOverrideMaterial OverrideMaterial;
        SMaterial OverrideMaterial2D;
        SMaterial InitMaterial2D;
        bool OverrideMaterial2DEnabled;

        E_FOG_TYPE FogType;
        bool PixelFog = false;
        bool RangeFog = false;
        bool AllowZWriteOnTransparent = false;

        bool FeatureEnabled[video::EVDF_COUNT];
    public:
        bool blockgpuprogramchange = false;

        // Inherited via IVideoDriver
        virtual void setScissorRect(const core::rect<s32>& rect) override;
};

} // end namespace video
} // end namespace irr

#endif
