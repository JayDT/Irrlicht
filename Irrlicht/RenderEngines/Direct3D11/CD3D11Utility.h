#ifndef __D3D11_UTILITY_H__
#define __D3D11_UTILITY_H__

#include "include/IReferenceCounted.h"
#include "include/SColor.h"
#include "include/SMaterial.h"
#include "include/SVertexIndex.h"
#include "include/EPrimitiveTypes.h"
#include "include/dimension2d.h"
#include "CD3D11Resources.h"

#include <d3d11_1.h>

#if _DEBUG
#define DEVEL_LOG(msg,code)	os::Printer::log(msg, code)	
#endif

#define IRR_DIRECTX_DEBUG 0
#define IRR_DIRECTX_SHADER_DEBUG IRR_DIRECTX_DEBUG | 0
#define IRR_DIRECTX_INSTRUMENTED_ENABLED 1

#if IRR_DIRECTX_INSTRUMENTED_ENABLED
	#define DX_BEGIN_EVENT(n) if (GroupMarker) { GroupMarker->BeginEvent(n); }
	#define DX_END_EVENT() if (GroupMarker) { GroupMarker->EndEvent(); }
#else
	#define DX_BEGIN_EVENT(n)
	#define DX_END_EVENT()
#endif

#if IRR_DIRECTX_INSTRUMENTED_ENABLED
	void SetDebugObjectName(ID3D11DeviceChild* resource, const char* str, ...);

    #define DX_NAME(resource, format, ...) SetDebugObjectName(resource, format, __VA_ARGS__)
#else
    #define DX_NAME(...)
#endif

namespace irr
{
	namespace video
	{
		class CD3D11FixedFunctionMaterialRenderer;
		class CD3D11MaterialRenderer_ONETEXTURE_BLEND;
		class CD3D11VertexDeclaration;
		class CD3D11HardwareBuffer;
		class D3D11HLSLProgram;
		class CD3D11Driver;
		struct IShader;

		struct MSAA
		{
			enum Enum
			{
				x1,
				x2,
				x4,
				x8,
				x16,

				Count
			};
		};

		struct SDepthSurface11 : public IReferenceCounted
		{
			SDepthSurface11() : Surface(0)
			{
#ifdef _DEBUG
				setDebugName("SDepthSurface");
#endif
			}

			virtual ~SDepthSurface11();

			irr::Ptr<CD3D11Driver> Driver;
			Msw::ComPtr<ID3D11DepthStencilView> Surface;
			core::dimension2du Size;
		};

		class DirectXUtil
		{
		public:
			//! Get D3D color format from Irrlicht color format.
			static DXGI_FORMAT getD3DFormatFromColorFormat(ECOLOR_FORMAT format);

			//! Get Irrlicht color format from D3D color format.
			static ECOLOR_FORMAT getColorFormatFromD3DFormat(DXGI_FORMAT format);

			//! Get Depth color format conversion
			static DXGI_FORMAT getD3DFormatToTypeless(DXGI_FORMAT format, bool checkSupport);
			static DXGI_FORMAT getD3DFormatFromTypeless(DXGI_FORMAT format);

			//! Get primitive topology
			static D3D_PRIMITIVE_TOPOLOGY getTopology(scene::E_PRIMITIVE_TYPE primType);

			//! Get number of bits per pixel
			static u32 getBitsPerPixel(DXGI_FORMAT format);

			//! Get number of components
			static u32 getNumberOfComponents(DXGI_FORMAT format);

			//! Get number of indices
			static u32 getIndexAmount(scene::E_PRIMITIVE_TYPE primType, u32 primitiveCount);

			static D3D11_TEXTURE_ADDRESS_MODE getTextureWrapMode(const u8 clamp);

			//! Get depth function
			static D3D11_COMPARISON_FUNC getDepthFunction(E_COMPARISON_FUNC func);

			//! Get Stenil Operartion
			static D3D11_STENCIL_OP getStencilOp(E_STENCIL_OPERATION func);

			//! get color write enable
			static D3D11_COLOR_WRITE_ENABLE getColorWriteEnable(E_COLOR_PLANE plane);

			//! Get index format from type
			static DXGI_FORMAT getIndexType(E_INDEX_TYPE iType);

			static MSAA::Enum ToMSAA(u32 sampleCount);

			static D3D11_BLEND getD3DBlend(E_BLEND_FACTOR factor);

			static D3D11_BLEND_OP getD3DBlendOp(E_BLEND_OPERATION operation);
		};
	}
}

#endif