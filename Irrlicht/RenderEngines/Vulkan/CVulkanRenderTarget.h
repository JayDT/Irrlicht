// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OPEN_GL_RENDER_TARGET_H_INCLUDED__
#define __C_OPEN_GL_RENDER_TARGET_H_INCLUDED__

#include "IrrCompileConfig.h"

#include "IRenderTarget.h"

#include "CVulkanUtility.h"
#include "CVulkanFramebuffer.h"

#include "dimension2d.h"
#include "os.h"

namespace irr
{
	namespace video
	{

		class CVulkanDriver;
        class VulkanFramebuffer;
        class VulkanSemaphore;

		class CVulkanRenderTarget : public IRenderTarget
		{
		public:
            CVulkanRenderTarget(CVulkanDriver* driver);
			virtual ~CVulkanRenderTarget();

			virtual void setTexture(const core::array<ITexture*>& texture, ITexture* depthStencil) _IRR_OVERRIDE_;

			const core::dimension2d<u32>& getSize() const;

            VulkanFramebuffer* getFrameBuffer() const { return FrameBuffer; }

			void releaseSurfaces();

			void generateSurfaces();

		protected:
			core::dimension2d<u32> Size;

            VULKAN_FRAMEBUFFER_DESC Surface;
            VulkanFramebuffer* FrameBuffer;
            VulkanSemaphore* Sync;

            CVulkanDriver* Driver;
		};

	}
}

#endif
