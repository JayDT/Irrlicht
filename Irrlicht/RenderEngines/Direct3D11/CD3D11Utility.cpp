#include "CD3D11Utility.h"
#include "CD3D11Driver.h"

#if IRR_DIRECTX_INSTRUMENTED_ENABLED
void SetDebugObjectName(ID3D11DeviceChild* resource, const char* str, ...)
{
	char name[128];

	va_list args;
	va_start(args, str);
	vsnprintf(name, sizeof(name), str, args);
	va_end(args);

	resource->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
}
#endif


DXGI_FORMAT irr::video::DirectXUtil::getD3DFormatFromColorFormat(ECOLOR_FORMAT format)
{
	switch (format)
	{
	case ECF_DXT1:
		return DXGI_FORMAT_BC1_UNORM;

	case ECF_DXT3:
		return DXGI_FORMAT_BC2_UNORM;

	case ECF_DXT5:
		return DXGI_FORMAT_BC3_UNORM;

	case ECF_A1R5G5B5:
		return DXGI_FORMAT_B5G5R5A1_UNORM;

	case ECF_R5G6B5:
		return DXGI_FORMAT_B5G6R5_UNORM;

	case ECF_R8G8B8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

    case ECF_B8G8R8:
        return DXGI_FORMAT_UNKNOWN;

	case ECF_RGBA8:
	case ECF_A8R8G8B8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case ECF_B8G8R8A8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

    case ECF_B8G8R8X8:
        return DXGI_FORMAT_B8G8R8X8_UNORM;

	case ECF_R16F:
		return DXGI_FORMAT_R16_FLOAT;

	case ECF_G16R16F:
		return DXGI_FORMAT_R16G16_FLOAT;

	case ECF_A16B16G16R16F:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;

	case ECF_R32F:
		return DXGI_FORMAT_R32_FLOAT;

	case ECF_G32R32F:
		return DXGI_FORMAT_R32G32_FLOAT;

	case ECF_A32B32G32R32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case ECF_R8:
		return DXGI_FORMAT_R8_UNORM;

	case ECF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;

	case ECF_D32_S8X24:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

	case ECF_D32:
		return DXGI_FORMAT_D32_FLOAT;

	case ECF_D24S8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

	case ECF_D16:
		return DXGI_FORMAT_D16_UNORM;
	}

	return DXGI_FORMAT_UNKNOWN;
}

irr::video::ECOLOR_FORMAT irr::video::DirectXUtil::getColorFormatFromD3DFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_BC1_UNORM:
		return ECF_DXT1;

	case DXGI_FORMAT_BC2_UNORM:
		return ECF_DXT3;

	case DXGI_FORMAT_BC3_UNORM:
		return ECF_DXT5;

	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return ECF_A1R5G5B5;

	case DXGI_FORMAT_B5G6R5_UNORM:
		return ECF_R5G6B5;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return ECF_B8G8R8A8;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return ECF_A8R8G8B8;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return ECF_B8G8R8X8;

	case DXGI_FORMAT_R16_FLOAT:
		return ECF_R16F;

	case DXGI_FORMAT_R16G16_FLOAT:
		return ECF_G16R16F;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return ECF_A16B16G16R16F;

	case DXGI_FORMAT_R32_FLOAT:
		return ECF_R32F;

	case DXGI_FORMAT_R32G32_FLOAT:
		return ECF_G32R32F;

	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return ECF_A32B32G32R32F;

	case DXGI_FORMAT_R8_UNORM:
		return ECF_R8;

	case DXGI_FORMAT_R8G8_UNORM:
		return ECF_R8G8;

	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return ECF_D32_S8X24;

	case DXGI_FORMAT_D32_FLOAT:
		return ECF_D32;

	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return ECF_D24S8;

	case DXGI_FORMAT_D16_UNORM:
		return ECF_D16;
	}

	return ECOLOR_FORMAT(0);
}

DXGI_FORMAT irr::video::DirectXUtil::getD3DFormatToTypeless(DXGI_FORMAT format, bool checkSupport)
{
	if (format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
		return DXGI_FORMAT_R32G8X24_TYPELESS;

	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		return DXGI_FORMAT_R24G8_TYPELESS;

	else if (format == DXGI_FORMAT_D32_FLOAT)
		return DXGI_FORMAT_R32_TYPELESS;

	else if (format == DXGI_FORMAT_D16_UNORM)
		return DXGI_FORMAT_R16_TYPELESS;

	return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT irr::video::DirectXUtil::getD3DFormatFromTypeless(DXGI_FORMAT format)
{
	if (format == DXGI_FORMAT_R32G8X24_TYPELESS)
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

	else if (format == DXGI_FORMAT_R24G8_TYPELESS)
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

	else if (format == DXGI_FORMAT_R32_TYPELESS)
		return DXGI_FORMAT_D32_FLOAT;

	else if (format == DXGI_FORMAT_R16_TYPELESS)
		return DXGI_FORMAT_D16_UNORM;

	return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
}

D3D_PRIMITIVE_TOPOLOGY irr::video::DirectXUtil::getTopology(scene::E_PRIMITIVE_TYPE primType)
{
	switch (primType)
	{
	case scene::EPT_POINT_SPRITES:
	case scene::EPT_POINTS:
		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	case scene::EPT_LINE_STRIP:
		return  D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

	case scene::EPT_LINE_LOOP:
	case scene::EPT_LINES:
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	case scene::EPT_TRIANGLE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	case scene::EPT_TRIANGLES:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	case scene::EPT_TRIANGLE_FAN:
	default:
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

irr::u32 irr::video::DirectXUtil::getBitsPerPixel(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

	case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
	case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
		return 32;

	case DXGI_FORMAT_D16_UNORM_S8_UINT:
	case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		return 24;

#endif // _XBOX_ONE && _TITLE
	default:
		return 0;
	}
}

irr::u32 irr::video::DirectXUtil::getNumberOfComponents(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 4;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return 3;

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
		return 2;

	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_R1_UNORM:
		return 1;

	default:
		return 0;
	}
}

irr::u32 irr::video::DirectXUtil::getIndexAmount(scene::E_PRIMITIVE_TYPE primType, u32 primitiveCount)
{
	switch (primType)
	{
	case scene::EPT_LINE_STRIP:
		return primitiveCount * 2;

	case scene::EPT_LINE_LOOP:
	case scene::EPT_LINES:
		return primitiveCount * 2;

	case scene::EPT_TRIANGLE_STRIP:
		return primitiveCount + 2;

	case scene::EPT_TRIANGLES:
		return primitiveCount * 3;

	default:
		return 0;
	}
}

D3D11_TEXTURE_ADDRESS_MODE irr::video::DirectXUtil::getTextureWrapMode(const u8 clamp)
{
	switch (clamp)
	{
	case ETC_REPEAT:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	case ETC_CLAMP:
	case ETC_CLAMP_TO_EDGE:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case ETC_MIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case ETC_CLAMP_TO_BORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	case ETC_MIRROR_CLAMP:
	case ETC_MIRROR_CLAMP_TO_EDGE:
	case ETC_MIRROR_CLAMP_TO_BORDER:
		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
	default:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

D3D11_COMPARISON_FUNC irr::video::DirectXUtil::getDepthFunction(E_COMPARISON_FUNC func)
{
	switch (func)
	{
	case ECFN_LESSEQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
	case ECFN_EQUAL:
		return D3D11_COMPARISON_EQUAL;
	case ECFN_LESS:
		return D3D11_COMPARISON_LESS;
	case ECFN_NOTEQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
	case ECFN_GREATEREQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
	case ECFN_GREATER:
		return D3D11_COMPARISON_GREATER;
	case ECFN_ALWAYS:
		return D3D11_COMPARISON_ALWAYS;
	case ECFN_NEVER:
	default:
		return D3D11_COMPARISON_NEVER;
	}
}

D3D11_STENCIL_OP irr::video::DirectXUtil::getStencilOp(E_STENCIL_OPERATION func)
{
	switch (func)
	{
	case E_STENCIL_OPERATION::ESO_ZERO:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
	case E_STENCIL_OPERATION::ESO_DECREMENT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR;
	case E_STENCIL_OPERATION::ESO_DECREMENT_WRAP:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR_SAT;
	case E_STENCIL_OPERATION::ESO_INCREMENT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR;
	case E_STENCIL_OPERATION::ESO_INCREMENT_WRAP:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR_SAT;
	case E_STENCIL_OPERATION::ESO_INVERT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INVERT;
	case E_STENCIL_OPERATION::ESO_REPLACE:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	default:
	case E_STENCIL_OPERATION::ESO_KEEP:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	}
}

D3D11_COLOR_WRITE_ENABLE irr::video::DirectXUtil::getColorWriteEnable(E_COLOR_PLANE plane)
{
	return (D3D11_COLOR_WRITE_ENABLE)
		(
		((plane & ECP_RED) ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
			((plane & ECP_GREEN) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
			((plane & ECP_BLUE) ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
			((plane & ECP_ALPHA) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0)
			);
}

DXGI_FORMAT irr::video::DirectXUtil::getIndexType(E_INDEX_TYPE iType)
{
	return (iType == video::EIT_16BIT) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}

irr::video::MSAA::Enum irr::video::DirectXUtil::ToMSAA(u32 sampleCount)
{
	uint32_t samples = std::max(1U, std::min(sampleCount, 16U));

	MSAA::Enum mssa = MSAA::x1;
	while (samples >>= 1)
	{
		mssa = (MSAA::Enum)((uint32_t)mssa + 1);
	}

	return mssa;
}

D3D11_BLEND irr::video::DirectXUtil::getD3DBlend(E_BLEND_FACTOR factor)
{
	D3D11_BLEND r = D3D11_BLEND_ZERO;
	switch (factor)
	{
	case EBF_ZERO:					r = D3D11_BLEND_ZERO; break;
	case EBF_ONE:					r = D3D11_BLEND_ONE; break;
	case EBF_DST_COLOR:				r = D3D11_BLEND_DEST_COLOR; break;
	case EBF_ONE_MINUS_DST_COLOR:	r = D3D11_BLEND_INV_DEST_COLOR; break;
	case EBF_SRC_COLOR:				r = D3D11_BLEND_SRC_COLOR; break;
	case EBF_ONE_MINUS_SRC_COLOR:	r = D3D11_BLEND_INV_SRC_COLOR; break;
	case EBF_SRC_ALPHA:				r = D3D11_BLEND_SRC_ALPHA; break;
	case EBF_ONE_MINUS_SRC_ALPHA:	r = D3D11_BLEND_INV_SRC_ALPHA; break;
	case EBF_DST_ALPHA:				r = D3D11_BLEND_DEST_ALPHA; break;
	case EBF_ONE_MINUS_DST_ALPHA:	r = D3D11_BLEND_INV_DEST_ALPHA; break;
	case EBF_SRC_ALPHA_SATURATE:	r = D3D11_BLEND_SRC_ALPHA_SAT; break;
	}
	return r;
}

D3D11_BLEND_OP irr::video::DirectXUtil::getD3DBlendOp(E_BLEND_OPERATION operation)
{
	switch (operation)
	{
	case EBO_NONE:			return D3D11_BLEND_OP_ADD;
	case EBO_ADD:			return D3D11_BLEND_OP_ADD;
	case EBO_SUBTRACT:		return D3D11_BLEND_OP_SUBTRACT;
	case EBO_REVSUBTRACT:	return D3D11_BLEND_OP_REV_SUBTRACT;
	case EBO_MIN:			return D3D11_BLEND_OP_MIN;
	case EBO_MAX:			return D3D11_BLEND_OP_MAX;
	case EBO_MIN_FACTOR:	return D3D11_BLEND_OP_MIN;
	case EBO_MAX_FACTOR:	return D3D11_BLEND_OP_MAX;
	case EBO_MIN_ALPHA:		return D3D11_BLEND_OP_MIN;
	case EBO_MAX_ALPHA:		return D3D11_BLEND_OP_MAX;
	}

	return D3D11_BLEND_OP_ADD;
}

irr::video::SDepthSurface11::~SDepthSurface11()
{
	if (Driver)
		Driver->removeDepthSurface(this);
}
