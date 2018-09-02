// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OPEN_GL_RENDER_TARGET_H_INCLUDED__
#define __C_OPEN_GL_RENDER_TARGET_H_INCLUDED__

#include "IrrCompileConfig.h"

#include "IRenderTarget.h"

#include "dimension2d.h"
#include "os.h"

#include <d3d11.h>

namespace irr
{
	namespace video
	{

		class CD3D11Driver;

		class CD3D11RenderTarget : public IRenderTarget
		{
		public:
            CD3D11RenderTarget(CD3D11Driver* driver);
			virtual ~CD3D11RenderTarget();

			virtual void setTexture(const core::array<ITexture*>& texture, ITexture* depthStencil) _IRR_OVERRIDE_;

			const core::dimension2d<u32>& getSize() const;

            ID3D11RenderTargetView* getSurface(u32 id) const;

			u32 getSurfaceCount() const;

            ID3D11RenderTargetView* getDepthStencilSurface() const;

			void releaseSurfaces();

			void generateSurfaces();

		protected:
			core::dimension2d<u32> Size;

			core::array<ID3D11RenderTargetView*> Surface;

            ID3D11RenderTargetView* DepthStencilSurface;

            CD3D11Driver* Driver;
		};

	}
}

#endif
