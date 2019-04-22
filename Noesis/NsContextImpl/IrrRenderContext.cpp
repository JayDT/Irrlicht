#include <NsImpl/IrrRenderContext.h>
#include <NsImpl/IrrImage.h>
#include <NsCore/Error.h>
#include <NsCore/Ptr.h>
#include <NsCore/Kernel.h>
#include <NsCore/Symbol.h>
#include <NsCore/NsFactory.h>
#include <NsCore/ReflectionImplement.h>
#include <NsCore/Package.h>
#include <NsCore/Category.h>
#include <NsRender/Texture.h>
#include <NsRender/RenderTarget.h>
#include <NsCore/Log.h>
#include <EASTL/sort.h>

#include <standard/client/DataSource_Standard.h>
#include <include/IMeshBuffer.h>
#include <include/SColor.h>
#include <include/SMaterial.h>
#include "RenderEngines/General/CIrrShader.h"

namespace HlslShader
{
#include "shaders/ShadersDx.h"
};

namespace GlslShader
{
#include "shaders/ShadersGL140.h"
};

using namespace Noesis;
using namespace NoesisApp;

#ifndef DYNAMIC_VB_SIZE
#define DYNAMIC_VB_SIZE 512 * 1024
#endif

#ifndef DYNAMIC_IB_SIZE
#define DYNAMIC_IB_SIZE 128 * 1024
#endif

namespace
{
    enum VFElements
    {
        VFPos = 0,
        VFColor = 1,
        VFTex0 = 2,
        VFTex1 = 4,
        VFCoverage = 8
    };

    enum TextureSlot
    {
        Pattern,
        Ramps,
        Image,
        Glyphs,

        Count
    };

    struct Program
    {
        int8_t vShaderIdx;
        int8_t pShaderIdx;
    };

    // Map from batch shader-ID to vertex and pixel shader objects
    const Program Programs[] =
    {
        { 0, 0 },    // RGBA
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { 0, 1 },    // Mask
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { 1, 2 },    // PathSolid
        { 2, 3 },    // PathLinear
        { 2, 4 },    // PathRadial
        { 2, 5 },    // PathPattern
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { 3, 6 },    // PathAASolid
        { 4, 7 },    // PathAALinear
        { 4, 8 },    // PathAARadial
        { 4, 9 },    // PathAAPattern
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { 5, 10 },   // ImagePaintOpacitySolid
        { 6, 11 },   // ImagePaintOpacityLinear
        { 6, 12 },   // ImagePaintOpacityRadial
        { 6, 13 },   // ImagePaintOpacityPattern
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { 5, 14 },   // TextSolid
        { 6, 15 },   // TextLinear
        { 6, 16 },   // TextRadial
        { 6, 17 },   // TextPattern
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class IrrNoesisShaderVertexCallBack : public irr::video::IShaderConstantSetCallBack
    {
    public:

        IrrNoesisShaderVertexCallBack(const Noesis::Batch* const& sharedBuffer)
            : mVertexCBHash(0), mSharedBuffer(sharedBuffer) {}

        virtual ~IrrNoesisShaderVertexCallBack() = default;

        void OnPrepare(irr::video::IConstantBuffer* buffer) override
        {
            mVar = irr::Ptr(buffer);
        }

        void OnSetMaterial(irr::video::IConstantBuffer* buffer, const irr::video::SMaterial& material) override
        {
        }

        void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, irr::scene::IMesh* mesh, irr::scene::ISceneNode* node) override
        {
            if (mVar && mVertexCBHash != mSharedBuffer->projMtxHash)
                mVar->setRawValue((irr::u8*)mSharedBuffer->projMtx, 0, 16 * sizeof(float));
        }

    private:

        irr::u32 mVertexCBHash;
        const Noesis::Batch* const& mSharedBuffer;
        irr::Ptr<irr::video::IShaderVariable> mVar;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class IrrNoesisShaderPixelCallBack : public irr::video::IShaderConstantSetCallBack
    {
    public:

        IrrNoesisShaderPixelCallBack(const Noesis::Batch* const& sharedBuffer)
            : mPixelCBHash(0), mSharedBuffer(sharedBuffer) {}

        virtual ~IrrNoesisShaderPixelCallBack() = default;

        void OnPrepare(irr::video::IConstantBuffer* buffer) override
        {
            mVar = irr::Ptr(buffer);
        }

        void OnSetMaterial(irr::video::IConstantBuffer* buffer, const irr::video::SMaterial& material) override
        {
        }

        void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, irr::scene::IMesh* mesh, irr::scene::ISceneNode* node) override
        {
            uint32_t hash = mSharedBuffer->rgbaHash ^ mSharedBuffer->radialGradHash ^ mSharedBuffer->opacityHash;
            if (mVar && mPixelCBHash != hash)
            {
                irr::u32 offset = 0;
                if (mSharedBuffer->rgba != 0)
                {
                    mVar->setRawValue((irr::u8*)mSharedBuffer->rgba, offset, 4 * sizeof(float));
                    offset += 4 * sizeof(float);
                }

                if (mSharedBuffer->radialGrad != 0)
                {
                    mVar->setRawValue((irr::u8*)mSharedBuffer->radialGrad, offset, 8 * sizeof(float));
                    offset += 8 * sizeof(float);
                }

                if (mSharedBuffer->opacity != 0)
                {
                    mVar->setRawValue((irr::u8*)mSharedBuffer->opacity, offset, sizeof(float));
                }
            }
        }

    private:
        irr::u32 mPixelCBHash;
        const Noesis::Batch* const& mSharedBuffer;
		irr::Ptr<irr::video::IShaderVariable> mVar;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class IrrNoesisShaderVertexReflectedCallBack : public irr::video::IShaderConstantSetCallBack
    {
    public:

        IrrNoesisShaderVertexReflectedCallBack(const Noesis::Batch* const& sharedBuffer)
            : mVertexCBHash(0), mSharedBuffer(sharedBuffer) {}

        virtual ~IrrNoesisShaderVertexReflectedCallBack() = default;

        void convertProjectionMatrix(irr::core::matrix4& matrix)
        {
            // Flip Y axis
            matrix(1, 1) = -matrix(1, 1);
            matrix(1, 3) = matrix(1, 3) * -1.f;

            // Convert depth range from [-1,1] to [0,1]
            matrix(2, 0) = matrix(2, 0) * 0.5f;
            matrix(2, 1) = matrix(2, 1) * 0.5f;
            matrix(2, 2) = matrix(2, 2) * 0.5f;
            matrix(3, 2) = matrix(3, 2) * 0.5f;
        }

        void OnPrepare(irr::video::IConstantBuffer* buffer) override
        {
            mMtx = irr::Ptr<irr::video::IShaderMatrixVariable>(buffer->getVariableByName("projectionMtx")->AsMatrix());
        }

        void OnSetMaterial(irr::video::IConstantBuffer* buffer, const irr::video::SMaterial& material) override
        {
        }

        void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, irr::scene::IMesh* mesh, irr::scene::ISceneNode* node) override
        {
            if (mMtx && mVertexCBHash != mSharedBuffer->projMtxHash)
            {
                convertProjectionMatrix(*(irr::core::matrix4*)mSharedBuffer->projMtx);
                mMtx->setRawValue((irr::u8*)mSharedBuffer->projMtx, 0, 16 * sizeof(float));
            }
        }

    private:

        irr::u32 mVertexCBHash;
        const Noesis::Batch* const& mSharedBuffer;
        irr::Ptr<irr::video::IShaderMatrixVariable> mMtx;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class IrrNoesisShaderPixelReflectedCallBack : public irr::video::IShaderConstantSetCallBack
    {
    public:

        IrrNoesisShaderPixelReflectedCallBack(const Noesis::Batch* const& sharedBuffer)
            : mPixelCBHash(0), mSharedBuffer(sharedBuffer) {}

        virtual ~IrrNoesisShaderPixelReflectedCallBack() = default;

        void OnPrepare(irr::video::IConstantBuffer* buffer) override
        {
            if (!buffer)
                return;

            if (buffer->getVariableByName("rgba"))
                mRgba = irr::Ptr<irr::video::IShaderVariable>(buffer->getVariableByName("rgba")->AsScalar());
            if (buffer->getVariableByName("radialGrad"))
                mRadialGrad = irr::Ptr<irr::video::IShaderVariable>(buffer->getVariableByName("radialGrad")->AsScalar());
            if (buffer->getVariableByName("opacity"))
                mOpacity = irr::Ptr<irr::video::IShaderVariable>(buffer->getVariableByName("opacity")->AsScalar());
        }

        void OnSetMaterial(irr::video::IConstantBuffer* buffer, const irr::video::SMaterial& material) override
        {
        }

        void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, irr::scene::IMesh* mesh, irr::scene::ISceneNode* node) override
        {
            uint32_t hash = mSharedBuffer->rgbaHash ^ mSharedBuffer->radialGradHash ^ mSharedBuffer->opacityHash;
            if (mPixelCBHash != hash)
            {
                if (mRgba && mSharedBuffer->rgba != 0)
                {
                    mRgba->setRawValue((irr::u8*)mSharedBuffer->rgba, 0, 4 * sizeof(float));
                }
            
                if (mRadialGrad && mSharedBuffer->radialGrad != 0)
                {
                    mRadialGrad->setRawValue((irr::u8*)mSharedBuffer->radialGrad, 0, 8 * sizeof(float));
                }
            
                if (mOpacity && mSharedBuffer->opacity != 0)
                {
                    mOpacity->setRawValue((irr::u8*)mSharedBuffer->opacity, 0, sizeof(float));
                }
            }
        }

    private:
        irr::u32 mPixelCBHash;
        const Noesis::Batch* const& mSharedBuffer;
        irr::Ptr<irr::video::IShaderVariable> mRgba;
        irr::Ptr<irr::video::IShaderVariable> mRadialGrad;
        irr::Ptr<irr::video::IShaderVariable> mOpacity;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    irr::video::ECOLOR_FORMAT ToIrr(TextureFormat::Enum format, bool linearRendering)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:
            {
                return irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8;
            }
            //case TextureFormat::BGRX8:
            //{
            //    return irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8;
            //}
            case TextureFormat::R8:
            {
                return irr::video::ECOLOR_FORMAT::ECF_R8;
            }
            default: NS_ASSERT_UNREACHABLE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TextureFormat::Enum ToNoesis(irr::video::ECOLOR_FORMAT format, bool linearRendering)
    {
        switch (format)
        {
            case irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8:
            {
                return TextureFormat::RGBA8;
            }
            //case :
            //{
            //    return TextureFormat::BGRX8;
            //}
            case irr::video::ECOLOR_FORMAT::ECF_R8:
            {
                return TextureFormat::R8;
            }
            default: NS_ASSERT_UNREACHABLE;
        }
    }


	////////////////////////////////////////////////////////////////////////////////////////////////////
	void ToIrr(irr::video::SMaterialLayer& layer, MinMagFilter::Enum minmagFilter, MipFilter::Enum mipFilter)
	{
		switch (minmagFilter)
		{
		case MinMagFilter::Nearest:
		{
			switch (mipFilter)
			{
			case MipFilter::Disabled:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = false;
				layer.BilinearFilter = false;
				break;
			}
			case MipFilter::Nearest:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = false;
				layer.BilinearFilter = false;
				break;
			}
			case MipFilter::Linear:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = false;
				layer.BilinearFilter = true;
				break;
			}
			default:
			{
				NS_ASSERT_UNREACHABLE;
			}
			}
		}
		case MinMagFilter::Linear:
		{
			switch (mipFilter)
			{
			case MipFilter::Disabled:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = false;
				layer.BilinearFilter = true;
				break;
			}
			case MipFilter::Nearest:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = false;
				layer.BilinearFilter = true;
				break;
			}
			case MipFilter::Linear:
			{
				layer.AnisotropicFilter = false;
				layer.TrilinearFilter = true;
				layer.BilinearFilter = true;
				break;
			}
			default:
			{
				NS_ASSERT_UNREACHABLE;
			}
			}
		}
		default:
		{
			NS_ASSERT_UNREACHABLE;
		}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void ToIrr(irr::video::SMaterialLayer& layer, WrapMode::Enum mode)
	{
		switch (mode)
		{
		case WrapMode::ClampToEdge:
		{
			layer.TextureWrapU = irr::video::ETC_CLAMP_TO_EDGE;
			layer.TextureWrapV = irr::video::ETC_CLAMP_TO_EDGE;
			break;
		}
		case WrapMode::ClampToZero:
		{
			bool hasBorder = true; // featureLevel >= D3D_FEATURE_LEVEL_9_3;
			layer.TextureWrapU = hasBorder ? irr::video::ETC_CLAMP_TO_BORDER : irr::video::ETC_CLAMP_TO_EDGE;
			layer.TextureWrapV = hasBorder ? irr::video::ETC_CLAMP_TO_BORDER : irr::video::ETC_CLAMP_TO_EDGE;
			break;
		}
		case WrapMode::Repeat:
		{
			layer.TextureWrapU = irr::video::ETC_REPEAT;
			layer.TextureWrapV = irr::video::ETC_REPEAT;
			break;
		}
		case WrapMode::MirrorU:
		{
			layer.TextureWrapU = irr::video::ETC_MIRROR;
			layer.TextureWrapV = irr::video::ETC_REPEAT;
			break;
		}
		case WrapMode::MirrorV:
		{
			layer.TextureWrapU = irr::video::ETC_REPEAT;
			layer.TextureWrapV = irr::video::ETC_MIRROR;
			break;
		}
		case WrapMode::Mirror:
		{
			layer.TextureWrapU = irr::video::ETC_MIRROR;
			layer.TextureWrapV = irr::video::ETC_MIRROR;
			break;
		}
		default:
		{
			NS_ASSERT_UNREACHABLE;
		}
		}
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
class NS_IRR_NOESIS_API IrrTexture final : public Noesis::Texture
{
public:
    IrrTexture(irr::video::ITexture* view_, uint32_t width_, uint32_t height_, uint32_t levels_,
        Noesis::TextureFormat::Enum format_, bool isInverted_)
        : view(view_), format(format_), isInverted(isInverted_) {}

    virtual ~IrrTexture() = default;

    uint32_t GetWidth() const override { return view->getSize().Width; }
    uint32_t GetHeight() const override { return view->getSize().Height; }
    TextureFormat::Enum GetFormat() const { return format; };
    bool HasMipMaps() const override { return view->hasMipMaps(); }
    bool IsInverted() const override { return isInverted; }

    const TextureFormat::Enum format;
    const bool isInverted;

    irr::Ptr<irr::video::ITexture> view;

	NS_DECLARE_REFLECTION(IrrTexture, Texture)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(IrrTexture)
{
	NsMeta<TypeId>("IrrTexture");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
class IrrRenderTarget final : public Noesis::RenderTarget
{
public:
    IrrRenderTarget(uint32_t width_, uint32_t height_, uint8_t sample_count_)
        : width(width_), height(height_), sample_count(sample_count_) {}

    virtual ~IrrRenderTarget() = default;

    Noesis::Texture * GetTexture() override { return texture; }

    const uint32_t width;
    const uint32_t height;
    const uint8_t sample_count;

    irr::Ptr<irr::video::IRenderTarget> renderTarget;
    Noesis::Ptr<IrrTexture> texture;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
NoesisApp::IrrRenderContext::IrrRenderContext()
{

}

void NoesisApp::IrrRenderContext::SetIrrlichtDevice(irr::IrrlichtDevice* context) noexcept
{
	mContext = irr::Ptr(context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const wchar_t* NoesisApp::IrrRenderContext::Description() const
{
    return mContext->getVideoDriver()->getName();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t NoesisApp::IrrRenderContext::Score() const
{
    return 200;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool NoesisApp::IrrRenderContext::Validate() const
{
    return mContext->getVideoDriver() ? true : false;
}

void CreateVertexStage(uint8_t format, const char* label, irr::video::VertexDeclaration* vertDecl)
{
    vertDecl->addElement(irr::video::E_VERTEX_ELEMENT_SEMANTIC::EVES_POSITION, irr::video::E_VERTEX_ELEMENT_TYPE::EVET_FLOAT2, -1, 0);

    if (format & VFElements::VFColor)
        vertDecl->addElement(irr::video::E_VERTEX_ELEMENT_SEMANTIC::EVES_COLOR, irr::video::E_VERTEX_ELEMENT_TYPE::EVET_COLOUR, -1, 0);

    if (format & VFElements::VFTex0)
        vertDecl->addElement(irr::video::E_VERTEX_ELEMENT_SEMANTIC::EVES_TEXTURE_COORD, irr::video::E_VERTEX_ELEMENT_TYPE::EVET_FLOAT2, -1, 0);

    if (format & VFElements::VFTex1)
        vertDecl->addElement(irr::video::E_VERTEX_ELEMENT_SEMANTIC::EVES_TEXTURE_COORD, irr::video::E_VERTEX_ELEMENT_TYPE::EVET_FLOAT2, -1, 1);

    if (format& VFElements::VFCoverage)
        vertDecl->addElement(irr::video::E_VERTEX_ELEMENT_SEMANTIC::EVES_TEXTURE_COORD, irr::video::E_VERTEX_ELEMENT_TYPE::EVET_FLOAT1, -1, 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::Init(void* window, uint32_t& samples, bool vsync, bool sRGB)
{
	mCaps = {};

	mCaps.centerPixelOffset = 0.0f;
	mCaps.dynamicVerticesSize = DYNAMIC_VB_SIZE;
	mCaps.dynamicIndicesSize = DYNAMIC_IB_SIZE;
	mCaps.linearRendering = sRGB;

	mVertices.reserve(mCaps.dynamicVerticesSize);
	mIndices.reserve(mCaps.dynamicIndicesSize);

	//mCaps.supportedTextureFormats[TextureFormat::RGBA8] = true;
	//mCaps.supportedTextureFormats[TextureFormat::BGRX8] = true;
	//mCaps.supportedTextureFormats[TextureFormat::R8] = true;

    if (GetIrrDevice()->getVideoDriver()->getDriverType() == irr::video::E_DRIVER_TYPE::EDT_DIRECT3D11 ||
        GetIrrDevice()->getVideoDriver()->getDriverType() == irr::video::E_DRIVER_TYPE::EDT_DIRECT3D9)
        CreateHlslLayout();
    else
        CreateGlslLayout();

	mMaterial.ZWriteEnable = false;
	mMaterial.FrontfaceCulling = false;
	mMaterial.BackfaceCulling = false;
	mMaterial.StencilFront.FailOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
	mMaterial.StencilBack.FailOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
	mMaterial.StencilFront.DepthFailOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
	mMaterial.StencilBack.DepthFailOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
	mMaterial.StencilFront.Mask = 0xFF;
	mMaterial.StencilFront.WriteMask = 0xFF;

#if defined(NS_PLATFORM_WINDOWS_DESKTOP) || defined(NS_PLATFORM_WINRT)
	NS_LOG_DEBUG(" Adapter: %ls", [this]()
	{
		return GetIrrDevice()->getVideoDriver()->getName();
	}());
#endif
}

// Can be use Dx11-12 and Vulkan
void NoesisApp::IrrRenderContext::CreateHlslLayout()
{
#define SHADER(n) {#n, n, sizeof(n)}

    struct Shader
    {
        const char* label;
        const BYTE* code;
        uint32_t size;

        std::shared_ptr<System::IO::IFileReader> ToStream() const
        {
            auto stream = std::make_shared<System::IO::MemoryStreamReader>((byte*)code, size, false);
			stream->FileName = label;
			return stream;
        }
    };

    const Shader pShaders[] =
    {
        SHADER(HlslShader::RGBA_FS),
        SHADER(HlslShader::Mask_FS),
        SHADER(HlslShader::PathSolid_FS),
        SHADER(HlslShader::PathLinear_FS),
        SHADER(HlslShader::PathRadial_FS),
        SHADER(HlslShader::PathPattern_FS),
        SHADER(HlslShader::PathAASolid_FS),
        SHADER(HlslShader::PathAALinear_FS),
        SHADER(HlslShader::PathAARadial_FS),
        SHADER(HlslShader::PathAAPattern_FS),
        SHADER(HlslShader::ImagePaintOpacitySolid_FS),
        SHADER(HlslShader::ImagePaintOpacityLinear_FS),
        SHADER(HlslShader::ImagePaintOpacityRadial_FS),
        SHADER(HlslShader::ImagePaintOpacityPattern_FS),
        SHADER(HlslShader::TextSolid_FS),
        SHADER(HlslShader::TextLinear_FS),
        SHADER(HlslShader::TextRadial_FS),
        SHADER(HlslShader::TextPattern_FS)
    };

    const Shader vShaders[] =
    {
        SHADER(HlslShader::Pos_VS),
        SHADER(HlslShader::PosColor_VS),
        SHADER(HlslShader::PosTex0_VS),
        SHADER(HlslShader::PosColorCoverage_VS),
        SHADER(HlslShader::PosTex0Coverage_VS),
        SHADER(HlslShader::PosColorTex1_VS),
        SHADER(HlslShader::PosTex0Tex1_VS)
    };

    const uint8_t formats[] =
    {
        VFPos,
        VFPos | VFColor,
        VFPos | VFTex0,
        VFPos | VFColor | VFCoverage,
        VFPos | VFTex0 | VFCoverage,
        VFPos | VFColor | VFTex1,
        VFPos | VFTex0 | VFTex1
    };

    mVertDecl.resize(NS_COUNTOF(formats));

    for (uint32_t i = 0; i < NS_COUNTOF(formats); i++)
    {
        mVertDecl[i] = irr::Ptr<irr::video::VertexDeclaration>(mContext->getVideoDriver()->GetVertexDeclaration(0x1000000 | i));
        CreateVertexStage(formats[i], vShaders[i].label, mVertDecl[i]);
        mVertDecl[i]->initialize();
    }


    mShaders.resize(NS_COUNTOF(Programs));

    // ToDo: replace this hax
    for (uint32_t i = 0; i < NS_COUNTOF(Programs); i++)
    {
        if (Programs[i].vShaderIdx == -1)
            continue;

        auto vSteam = vShaders[Programs[i].vShaderIdx].ToStream();
        auto pSteam = pShaders[Programs[i].pShaderIdx].ToStream();

        irr::video::ShaderInitializerEntry shaderCI;
    
        shaderCI.AddShaderStage(vSteam.get(), irr::video::E_SHADER_TYPES::EST_VERTEX_SHADER, "main", "vs_4_0", irr::video::E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT, true);
        shaderCI.AddShaderStage(pSteam.get(), irr::video::E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", "ps_4_0", irr::video::E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT, true);
        irr::video::CNullShader* shader = static_cast<irr::video::CNullShader*>(mContext->getVideoDriver()->getGPUProgrammingServices()->createShader(&shaderCI));
        mShaders[i].shader = irr::Ptr<irr::video::IShader>(shader);
        mShaders[i].texturePosition = { 0, 1, 2, 3 };

		{
			auto buffer = shader->AddUnknownBuffer(irr::video::E_SHADER_TYPES::EST_VERTEX_SHADER, sizeof(irr::core::matrix4));
			auto callback = irr::MakePtr<IrrNoesisShaderVertexCallBack>(mSharedBuffer);
			buffer->setShaderDataCallback(callback);
			callback->OnPrepare(buffer);
		}

		{
			auto buffer = shader->AddUnknownBuffer(irr::video::E_SHADER_TYPES::EST_FRAGMENT_SHADER, 12 * sizeof(float));
			auto callback = irr::MakePtr<IrrNoesisShaderPixelCallBack>(mSharedBuffer);
			buffer->setShaderDataCallback(callback);
			callback->OnPrepare(buffer);
		}
    }

#undef SHADER
}

// Can be use OpenGL and Vulkan
void NoesisApp::IrrRenderContext::CreateGlslLayout()
{
#define GL_SHADER(n) {#n, n, sizeof(n)}

    struct Shader
    {
        const char* label;
        const char* code;
        uint32_t size;

        std::shared_ptr<System::IO::IFileReader> ToStream() const
        {
            return std::make_shared<System::IO::MemoryStreamReader>((byte*)code, size, false);
        }
    };

    const Shader pShaders[] =
    {
        GL_SHADER(GlslShader::RGBA_FS),
        GL_SHADER(GlslShader::Mask_FS),
        GL_SHADER(GlslShader::PathSolid_FS),
        GL_SHADER(GlslShader::PathLinear_FS),
        GL_SHADER(GlslShader::PathRadial_FS),
        GL_SHADER(GlslShader::PathPattern_FS),
        GL_SHADER(GlslShader::PathAASolid_FS),
        GL_SHADER(GlslShader::PathAALinear_FS),
        GL_SHADER(GlslShader::PathAARadial_FS),
        GL_SHADER(GlslShader::PathAAPattern_FS),
        GL_SHADER(GlslShader::ImagePaintOpacitySolid_FS),
        GL_SHADER(GlslShader::ImagePaintOpacityLinear_FS),
        GL_SHADER(GlslShader::ImagePaintOpacityRadial_FS),
        GL_SHADER(GlslShader::ImagePaintOpacityPattern_FS),
        GL_SHADER(GlslShader::TextSolid_FS),
        GL_SHADER(GlslShader::TextLinear_FS),
        GL_SHADER(GlslShader::TextRadial_FS),
        GL_SHADER(GlslShader::TextPattern_FS)
    };

    const Shader vShaders[] =
    {
        GL_SHADER(GlslShader::Pos_VS),
        GL_SHADER(GlslShader::PosColor_VS),
        GL_SHADER(GlslShader::PosTex0_VS),
        GL_SHADER(GlslShader::PosColorCoverage_VS),
        GL_SHADER(GlslShader::PosTex0Coverage_VS),
        GL_SHADER(GlslShader::PosColorTex1_VS),
        GL_SHADER(GlslShader::PosTex0Tex1_VS)
    };

    const uint8_t formats[] =
    {
        VFPos,
        VFPos | VFColor,
        VFPos | VFTex0,
        VFPos | VFColor | VFCoverage,
        VFPos | VFTex0 | VFCoverage,
        VFPos | VFColor | VFTex1,
        VFPos | VFTex0 | VFTex1
    };

    mVertDecl.resize(NS_COUNTOF(formats));

    for (uint32_t i = 0; i < NS_COUNTOF(formats); i++)
    {
        mVertDecl[i] = irr::Ptr<irr::video::VertexDeclaration>(mContext->getVideoDriver()->GetVertexDeclaration(0x1000000 | i));
        CreateVertexStage(formats[i], vShaders[i].label, mVertDecl[i]);
        mVertDecl[i]->initialize();
    }

    mShaders.resize(NS_COUNTOF(Programs));

    // ToDo: replace this hax
    for (uint32_t i = 0; i < NS_COUNTOF(Programs); i++)
    {
        if (Programs[i].vShaderIdx == -1)
            continue;

        auto vSteam = vShaders[Programs[i].vShaderIdx].ToStream();
        auto pSteam = pShaders[Programs[i].pShaderIdx].ToStream();

        irr::video::ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(vSteam.get(), irr::video::E_SHADER_TYPES::EST_VERTEX_SHADER, "main", "", irr::video::E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT, true);
        shaderCI.AddShaderStage(pSteam.get(), irr::video::E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", "", irr::video::E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT, true);
        shaderCI.Callback["MatrixBuffer"] = new IrrNoesisShaderVertexReflectedCallBack(mSharedBuffer);
        shaderCI.Callback["ColorBuffer"] = new IrrNoesisShaderPixelReflectedCallBack(mSharedBuffer);
        irr::video::CNullShader * shader = static_cast<irr::video::CNullShader*>(mContext->getVideoDriver()->getGPUProgrammingServices()->createShader(&shaderCI));
        mShaders[i].shader = irr::Ptr<irr::video::IShader>(shader);

        auto pattern = mShaders[i].shader->getVariableByName("pattern");
        auto ramps   = mShaders[i].shader->getVariableByName("ramps");
        auto image   = mShaders[i].shader->getVariableByName("image");
        auto glyphs  = mShaders[i].shader->getVariableByName("glyphs");

        uint8_t slot = 0;
        if (pattern != 0)
            mShaders[i].texturePosition[TextureSlot::Pattern] = slot++;

        if (ramps != 0)
            mShaders[i].texturePosition[TextureSlot::Ramps] = slot++;

        if (image != 0)
            mShaders[i].texturePosition[TextureSlot::Image] = slot++;

        if (glyphs != 0)
            mShaders[i].texturePosition[TextureSlot::Glyphs] = slot;
    }
#undef GL_SHADER
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::RenderDevice* NoesisApp::IrrRenderContext::GetDevice() const
{
    return const_cast<Noesis::RenderDevice*>(static_cast<const Noesis::RenderDevice*>(this));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::BeginRender()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::EndRender()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::Resize()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float NoesisApp::IrrRenderContext::GetGPUTimeMs() const
{
    return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::SetClearColor(float r, float g, float b, float a)
{
    mClearColor.setRed(r);
    mClearColor.setGreen(g);
    mClearColor.setBlue(b);
    mClearColor.setAlpha(a);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::SetDefaultRenderTarget(uint32_t width, uint32_t height)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<NoesisApp::Image> NoesisApp::IrrRenderContext::CaptureRenderTarget(Noesis::RenderTarget* surface) const
{
    return Noesis::Ptr<NoesisApp::Image>();
}

irr::IrrlichtDevice* NoesisApp::IrrRenderContext::GetIrrDevice() const
{
	return mContext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const Noesis::DeviceCaps& NoesisApp::IrrRenderContext::GetCaps() const
{
    return mCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<Noesis::RenderTarget> NoesisApp::IrrRenderContext::CreateRenderTarget(const char* label, uint32_t width, uint32_t height, uint32_t sampleCount)
{
	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    Noesis::Ptr<IrrRenderTarget> surface = Noesis::MakePtr<IrrRenderTarget>(width, height, sampleCount);

	irr::Ptr<irr::video::ITexture> renderTargetTex = *mContext->getVideoDriver()->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(width, height), "RTT1", ToIrr(TextureFormat::RGBA8, false));
	irr::Ptr<irr::video::ITexture> renderTargetDepth = *mContext->getVideoDriver()->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(width, height), "DepthStencil", irr::video::ECF_D32_S8X24);

    Noesis::Ptr<IrrTexture> texture = Noesis::MakePtr<IrrTexture>(renderTargetTex, width, height, 1, TextureFormat::RGBA8, false);

    surface->renderTarget = *mContext->getVideoDriver()->addRenderTarget();
    surface->texture = texture;
    surface->renderTarget->setTexture(renderTargetTex, renderTargetDepth);

	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return surface;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<RenderTarget> NoesisApp::IrrRenderContext::CloneRenderTarget(const char* label, Noesis::RenderTarget* surface)
{
	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
	
	IrrRenderTarget* irrSurface = static_cast<IrrRenderTarget*>(surface);

	Noesis::Ptr<IrrRenderTarget> cloneSurface = Noesis::MakePtr<IrrRenderTarget>(irrSurface->width, irrSurface->height, irrSurface->sample_count);
    IrrTexture* cloneTexture = static_cast<IrrTexture*>(irrSurface->GetTexture());
	
	irr::Ptr<irr::video::ITexture> renderTargetTex = *mContext->getVideoDriver()->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(irrSurface->width, irrSurface->height), "CLONE_RTT1", ToIrr(cloneTexture->GetFormat(), false));
	irr::Ptr<irr::video::ITexture> renderTargetDepth = *mContext->getVideoDriver()->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(irrSurface->width, irrSurface->height), "CLONE_DepthStencil", irr::video::ECF_D32_S8X24);
	
	Noesis::Ptr<IrrTexture> texture = Noesis::MakePtr<IrrTexture>(renderTargetTex, irrSurface->width, irrSurface->height, 1, cloneTexture->GetFormat(), false);
	
	cloneSurface->renderTarget = *mContext->getVideoDriver()->addRenderTarget();
	cloneSurface->texture = texture;
	cloneSurface->renderTarget->setTexture(renderTargetTex, renderTargetDepth);

	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return cloneSurface;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<Texture> NoesisApp::IrrRenderContext::CreateTexture(const char* label, uint32_t width, uint32_t height, uint32_t numLevels, Noesis::TextureFormat::Enum format)
{
	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

	irr::Ptr<irr::video::ITexture> irrTex = *mContext->getVideoDriver()->addTexture(irr::core::dimension2d<irr::u32>(width, height), label, ToIrr(format, false));

    Noesis::Ptr<IrrTexture> texture = Noesis::MakePtr<IrrTexture>(irrTex, width, height, 1, format, false);

	mContext->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::UpdateTexture(Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
    IrrTexture* irrTex = static_cast<IrrTexture*>(texture);
    irrTex->view->updateTexture(level, x, y, width, height, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::BeginRender(bool offscreen)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::SetRenderTarget(Noesis::RenderTarget* surface)
{
    IrrRenderTarget* irrSurface = static_cast<IrrRenderTarget*>(surface);
	
    if (irrSurface)
        mContext->getVideoDriver()->setRenderTargetEx(irrSurface->renderTarget, irr::video::ECBF_COLOR | irr::video::ECBF_DEPTH | irr::video::ECBF_STENCIL, irr::video::SColor(0, 0, 0, 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::BeginTile(const Noesis::Tile& tile, uint32_t surfaceWidth, uint32_t surfaceHeight)
{
    uint32_t x = tile.x;
    uint32_t y = (uint32_t)surfaceHeight - (tile.y + tile.height);

    irr::core::rect<irr::s32> rect;
    rect.UpperLeftCorner.X = x;
    rect.UpperLeftCorner.Y = y;
    rect.LowerRightCorner.X = x + tile.width;
    rect.LowerRightCorner.Y = y + tile.height;

    mContext->getVideoDriver()->setScissorRect(rect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::EndTile()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::ResolveRenderTarget(Noesis::RenderTarget* surface, const Noesis::Tile* tiles, uint32_t numTiles)
{
	irr::core::array<irr::core::rect<irr::s32>> _tiles;
	_tiles.set_used(numTiles);
	for (uint32_t i = 0; i < numTiles; ++i)
	{
		auto& tile = tiles[i];

		irr::core::rect<irr::s32>& rect = _tiles[i];
		rect.UpperLeftCorner.X = tile.x;
		rect.UpperLeftCorner.Y = tile.y;
		rect.LowerRightCorner.X = tile.x + tile.width;
		rect.LowerRightCorner.Y = tile.y + tile.height;
	}

	mContext->getVideoDriver()->setRenderTargetEx(0, irr::video::ECBF_COLOR | irr::video::ECBF_DEPTH | irr::video::ECBF_STENCIL, irr::video::SColor(0, 0, 0, 0), 1.f, 0, &_tiles);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* NoesisApp::IrrRenderContext::MapVertices(uint32_t bytes)
{
	mVertices.resize(bytes);
    return mVertices.data();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::UnmapVertices()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* NoesisApp::IrrRenderContext::MapIndices(uint32_t bytes)
{
	mIndices.resize(bytes / sizeof(uint16_t));
    return mIndices.data();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::UnmapIndices()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrRenderContext::DrawBatch(const Noesis::Batch& batch)
{
    NS_ASSERT(batch.shader.v < NS_COUNTOF(Programs));
    NS_ASSERT(Programs[batch.shader.v].vShaderIdx != -1);
    NS_ASSERT(Programs[batch.shader.v].pShaderIdx != -1);

    mSharedBuffer = &batch;

    const RenderState& renderState = batch.renderState;

    mMaterial.ScissorEnable = renderState.f.scissorEnable;
    mMaterial.Wireframe = renderState.f.wireframe;

	mMaterial.ZBuffer = irr::video::E_COMPARISON_FUNC::ECFN_NEVER;
    mMaterial.ColorMask = renderState.f.colorEnable ? 0xF : 0;

    mMaterial.BlendOperation = renderState.f.blendMode;
	mMaterial.MaterialType = renderState.f.blendMode ? irr::video::E_MATERIAL_TYPE::EMT_ONETEXTURE_BLEND : irr::video::E_MATERIAL_TYPE::EMT_SOLID;
	mMaterial.uMaterialTypeParam = irr::video::pack_textureBlendFunc(irr::video::EBF_ONE, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR, irr::video::EBF_ONE, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::E_BLEND_OPERATION::EBO_ADD, irr::video::E_BLEND_OPERATION::EBO_ADD);

    mMaterial.StencilTest = renderState.f.stencilMode;
    mMaterial.StencilFront.Comparsion = irr::video::E_COMPARISON_FUNC::ECFN_EQUAL;
    mMaterial.StencilBack.Comparsion = irr::video::E_COMPARISON_FUNC::ECFN_EQUAL;

	mMaterial.StencilFront.Reference = batch.stencilRef;

    switch (renderState.f.stencilMode)
    {
        case 0:
        case 1:
            mMaterial.StencilFront.PassOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
            mMaterial.StencilBack.PassOp = irr::video::E_STENCIL_OPERATION::ESO_KEEP;
            break;
        case 2:
            mMaterial.StencilFront.PassOp = irr::video::E_STENCIL_OPERATION::ESO_INCREMENT;
            mMaterial.StencilBack.PassOp = irr::video::E_STENCIL_OPERATION::ESO_INCREMENT;
            break;
        case 3:
            mMaterial.StencilFront.PassOp = irr::video::E_STENCIL_OPERATION::ESO_DECREMENT;
            mMaterial.StencilBack.PassOp = irr::video::E_STENCIL_OPERATION::ESO_DECREMENT;
            break;
        case 4:
            mMaterial.StencilFront.Comparsion = irr::video::E_COMPARISON_FUNC::ECFN_ALWAYS;
            mMaterial.StencilFront.PassOp = irr::video::E_STENCIL_OPERATION::ESO_ZERO;
            mMaterial.StencilBack.Comparsion = irr::video::E_COMPARISON_FUNC::ECFN_ALWAYS;
            mMaterial.StencilBack.PassOp = irr::video::E_STENCIL_OPERATION::ESO_ZERO;
            break;
    }

    if (batch.pattern != 0)
    {
        uint8_t slot = mShaders[batch.shader.v].texturePosition[TextureSlot::Pattern];
        mMaterial.setTexture(slot, static_cast<IrrTexture*>(batch.pattern)->view);
		mMaterial.TextureLayer[slot].AnisotropicFilter = 1;
		ToIrr(mMaterial.TextureLayer[slot], MinMagFilter::Enum(batch.patternSampler.f.minmagFilter), MipFilter::Enum(batch.patternSampler.f.mipFilter));
		ToIrr(mMaterial.TextureLayer[slot], WrapMode::Enum(batch.patternSampler.f.wrapMode));
    }

    if (batch.ramps != 0)
    {
        uint8_t slot = mShaders[batch.shader.v].texturePosition[TextureSlot::Ramps];
        mMaterial.setTexture(slot, static_cast<IrrTexture*>(batch.ramps)->view);
		mMaterial.TextureLayer[slot].AnisotropicFilter = 1;
		ToIrr(mMaterial.TextureLayer[slot], MinMagFilter::Enum(batch.rampsSampler.f.minmagFilter), MipFilter::Enum(batch.rampsSampler.f.mipFilter));
		ToIrr(mMaterial.TextureLayer[slot], WrapMode::Enum(batch.rampsSampler.f.wrapMode));
	}

    if (batch.image != 0)
    {
        uint8_t slot = mShaders[batch.shader.v].texturePosition[TextureSlot::Image];
        mMaterial.setTexture(slot, static_cast<IrrTexture*>(batch.image)->view);
		mMaterial.TextureLayer[slot].AnisotropicFilter = 1;
		ToIrr(mMaterial.TextureLayer[slot], MinMagFilter::Enum(batch.imageSampler.f.minmagFilter), MipFilter::Enum(batch.imageSampler.f.mipFilter));
		ToIrr(mMaterial.TextureLayer[slot], WrapMode::Enum(batch.imageSampler.f.wrapMode));
	}

    if (batch.glyphs != 0)
    {
        uint8_t slot = mShaders[batch.shader.v].texturePosition[TextureSlot::Glyphs];
        mMaterial.setTexture(slot, static_cast<IrrTexture*>(batch.glyphs)->view);
		mMaterial.TextureLayer[slot].AnisotropicFilter = 1;
		ToIrr(mMaterial.TextureLayer[slot], MinMagFilter::Enum(batch.glyphsSampler.f.minmagFilter), MipFilter::Enum(batch.glyphsSampler.f.mipFilter));
		ToIrr(mMaterial.TextureLayer[slot], WrapMode::Enum(batch.glyphsSampler.f.wrapMode));
	}

    auto _driver = mContext->getVideoDriver();
    irr::video::E_VERTEX_TYPE vType = irr::video::E_VERTEX_TYPE(0x1000000 | Programs[batch.shader.v].vShaderIdx);

    _driver->setMaterial(mMaterial);
    _driver->SetShader(mShaders[batch.shader.v].shader);
    _driver->drawVertexPrimitiveList(&mVertices[batch.vertexOffset], batch.numVertices, &mIndices[batch.startIndex], batch.numIndices / 3, vType, irr::scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, irr::video::E_INDEX_TYPE::EIT_16BIT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Render, IrrRenderContext)
{
	NS_REGISTER_COMPONENT(IrrRenderContext)
	//NS_REGISTER_COMPONENT(IrrTexture)
	//NS_REGISTER_COMPONENT(IrrRenderTarget)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(IrrRenderContext)
{
	NsMeta<TypeId>("IrrRenderContext");
	NsMeta<Category>("RenderContext");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Ptr<IrrRenderContext> NoesisApp::CreateRenderContext(const char* name)
{
	ComponentFactory* factory = NsGetKernel()->GetComponentFactory();

	char id[256];
	snprintf(id, 256, "%sRenderContext", name);

	return Noesis::DynamicPtrCast<IrrRenderContext>(factory->CreateComponent(NsSymbol(id)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//NS_REGISTER_REFLECTION(Render, IrrRenderTarget)
//{
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//NS_INIT_PACKAGE(Render, IrrRenderTarget)
//{
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//NS_SHUTDOWN_PACKAGE(Render, IrrRenderTarget)
//{
//}
