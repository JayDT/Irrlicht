#ifndef __RENDER_RENDERCONTEXT_H__
#define __RENDER_RENDERCONTEXT_H__

#include <NsApp/IrrNoesis.h>
#include <NsCore/Noesis.h>
#include <NsCore/BaseComponent.h>
#include <NsRender/RenderDevice.h>
#include <include/irrlicht.h>
#include <vector>
#include <array>

namespace irr
{
    class IrrlichtDevice;
    namespace video
    {
        struct IShader;
        struct VertexDeclaration;
    }
}

namespace Noesis
{

    class RenderTarget;
    template<class T> class Ptr;

}

namespace NoesisApp
{

    class Image;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IrrRenderContext implementation is in charge of the initialization of a rendering device
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class NS_IRR_NOESIS_API IrrRenderContext final : public Noesis::RenderDevice
    {
        struct NsShaderDesc
        {
            irr::Ptr<irr::video::IShader> shader;
            std::array<uint8_t, 4> texturePosition;
        };

    private:
        irr::Ptr<irr::IrrlichtDevice> mContext;
        irr::video::SColor mClearColor;

        Noesis::DeviceCaps mCaps;
        Noesis::RenderTarget* mBoundRenderTarget;
        Noesis::Batch const* mSharedBuffer;

        std::vector<NsShaderDesc> mShaders;
        std::vector<irr::Ptr<irr::video::VertexDeclaration>> mVertDecl;

        std::vector<uint8_t> mVertices;
        std::vector<uint16_t> mIndices;
		
        irr::video::SMaterial mMaterial;

    public:

        IrrRenderContext();
		virtual ~IrrRenderContext() = default;

		void SetIrrlichtDevice(irr::IrrlichtDevice* context) noexcept;

        /// Brief description provided by the implementation
        const wchar_t* Description() const;

        /// When looking for the best render context, those with higher score are tried first
        uint32_t Score() const;

        /// Returns whether the implementation is valid
        bool Validate() const;

        /// Initializes the rendering context with the given window and multisampling samples
        void Init(void* window, uint32_t& samples, bool vsync, bool sRGB);

        /// Returns the rendering device maintained by this context
        Noesis::RenderDevice* GetDevice() const;

        /// Called prior to rendering
        void BeginRender();

        /// Called after the rendering
        void EndRender();

        /// Should be called when the window is resized
        void Resize();

        /// Returns the milliseconds taken by the last frame executed in the GPU
        float GetGPUTimeMs() const;

        /// Sets the clear color (in sRGB space)
        void SetClearColor(float r, float g, float b, float a);

        /// Binds the render targets associated with the window swap chain and clears it
        void SetDefaultRenderTarget(uint32_t width, uint32_t height);

        /// Grabs an image with the content of current render target
        Noesis::Ptr<NoesisApp::Image> CaptureRenderTarget(Noesis::RenderTarget* surface) const;

		irr::IrrlichtDevice* GetIrrDevice() const;

        NS_DECLARE_REFLECTION(IrrRenderContext, BaseComponent)

        // Inherited via RenderDevice
        virtual const Noesis::DeviceCaps& GetCaps() const override;
        virtual Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(const char* label, uint32_t width, uint32_t height, uint32_t sampleCount) override;
        virtual Noesis::Ptr<Noesis::RenderTarget> CloneRenderTarget(const char* label, Noesis::RenderTarget* surface) override;
        virtual Noesis::Ptr<Noesis::Texture> CreateTexture(const char* label, uint32_t width, uint32_t height, uint32_t numLevels, Noesis::TextureFormat::Enum format) override;
        virtual void UpdateTexture(Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) override;
        virtual void BeginRender(bool offscreen) override;
        virtual void SetRenderTarget(Noesis::RenderTarget* surface) override;
        virtual void BeginTile(const Noesis::Tile& tile, uint32_t surfaceWidth, uint32_t surfaceHeight) override;
        virtual void EndTile() override;
        virtual void ResolveRenderTarget(Noesis::RenderTarget* surface, const Noesis::Tile* tiles, uint32_t numTiles) override;
        virtual void* MapVertices(uint32_t bytes) override;
        virtual void UnmapVertices() override;
        virtual void* MapIndices(uint32_t bytes) override;
        virtual void UnmapIndices() override;
        virtual void DrawBatch(const Noesis::Batch& batch) override;

    private:

        // Can be use Dx11-12 and Vulkan
        void CreateHlslLayout();
        // Can be use OpenGL and Vulkan
        void CreateGlslLayout();
    };

	NS_IRR_NOESIS_API Noesis::Ptr<IrrRenderContext> CreateRenderContext(const char* name);
}

#endif
