// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OGLCORE_RENDER_TARGET_H_INCLUDED__
#define __C_OGLCORE_RENDER_TARGET_H_INCLUDED__

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_OPENGL_) || defined(_IRR_COMPILE_WITH_OGLES1_) || defined(_IRR_COMPILE_WITH_OGLES2_)

#include "IRenderTarget.h"
#include "COpenGLExtensionHandler.h"

namespace irr
{
namespace video
{

template <class TOpenGLDriver, class TOpenGLTexture>
class COpenGLCoreRenderTarget : public IRenderTarget
{
public:
	COpenGLCoreRenderTarget(TOpenGLDriver* driver) : AssignedDepth(false), AssignedStencil(false), RequestTextureUpdate(false), RequestDepthStencilUpdate(false),
		BufferID(0), ColorAttachment(0), MultipleRenderTarget(0), Driver(driver)
	{
#ifdef _DEBUG
		setDebugName("COpenGLCoreRenderTarget");
#endif

		DriverType = Driver->getDriverType();

		Size = Driver->getScreenSize();

		ColorAttachment = Driver->ColorAttachment;
		MultipleRenderTarget = Driver->MaxMultipleRenderTargets;

		if (ColorAttachment > 0)
            glGenFramebuffers(1, &BufferID);

		AssignedTexture.set_used(static_cast<u32>(ColorAttachment));

		for (u32 i = 0; i < AssignedTexture.size(); ++i)
			AssignedTexture[i] = GL_NONE;
	}

	virtual ~COpenGLCoreRenderTarget()
	{
		if (ColorAttachment > 0 && BufferID != 0)
            glDeleteFramebuffers(1, &BufferID);

		for (u32 i = 0; i < Texture.size(); ++i)
		{
			if (Texture[i])
				Texture[i]->drop();
		}

		if (DepthStencil)
			DepthStencil->drop();
	}

	virtual void setTexture(const core::array<ITexture*>& texture, ITexture* depthStencil) _IRR_OVERRIDE_
	{
		bool textureUpdate = (Texture != texture) ? true : false;
		bool depthStencilUpdate = (DepthStencil != depthStencil) ? true : false;

		if (textureUpdate || depthStencilUpdate)
		{
			// Set color attachments.

			if (textureUpdate)
			{
				for (u32 i = 0; i < Texture.size(); ++i)
				{
					if (Texture[i])
						Texture[i]->drop();
				}

				if (texture.size() > static_cast<u32>(ColorAttachment))
				{
					core::stringc message = "This GPU supports up to ";
					message += static_cast<u32>(ColorAttachment);
					message += " textures per render target.";

					os::Printer::log(message.c_str(), ELL_WARNING);
				}

				Texture.set_used(core::min_(texture.size(), static_cast<u32>(ColorAttachment)));

				for (u32 i = 0; i < Texture.size(); ++i)
				{
					TOpenGLTexture* currentTexture = (texture[i] && texture[i]->getDriverType() == DriverType) ? static_cast<TOpenGLTexture*>(texture[i]) : 0;

					GLuint textureID = 0;

					if (currentTexture)
					{
						if (currentTexture->getType() == E_TEXTURE_TYPE::e2D)
							textureID = currentTexture->getOpenGLTextureName();
						else
							os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);
					}

					if (textureID != 0)
					{
						Texture[i] = texture[i];
						Texture[i]->grab();
					}
					else
					{
						Texture[i] = 0;
					}
				}

				RequestTextureUpdate = true;
			}

			// Set depth and stencil attachments.

			if (depthStencilUpdate)
			{
				TOpenGLTexture* currentTexture = (depthStencil && depthStencil->getDriverType() == DriverType) ? static_cast<TOpenGLTexture*>(depthStencil) : 0;

				GLuint textureID = 0;

				if (currentTexture)
				{
					if (currentTexture->getType() == E_TEXTURE_TYPE::e2D)
						textureID = currentTexture->getOpenGLTextureName();
					else
						os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);
				}

				const ECOLOR_FORMAT textureFormat = (textureID != 0) ? depthStencil->getColorFormat() : ECF_UNKNOWN;

				if (IImage::isDepthFormat(textureFormat))
				{
					DepthStencil = depthStencil;
					DepthStencil->grab();
				}
				else
				{
					if (DepthStencil)
						DepthStencil->drop();

					DepthStencil = 0;
				}

				RequestDepthStencilUpdate = true;
			}

			// Set size required for a viewport.

			ITexture* firstTexture = getTexture();

			if (firstTexture)
				Size = firstTexture->getSize();
			else
			{
				if (DepthStencil)
					Size = DepthStencil->getSize();
				else
					Size = Driver->getScreenSize();
			}
		}
	}

	void update()
	{
		if (RequestTextureUpdate || RequestDepthStencilUpdate)
		{
			// Set color attachments.

			if (RequestTextureUpdate)
			{
				// Set new color textures.

				const u32 textureSize = core::min_(Texture.size(), AssignedTexture.size());

				for (u32 i = 0; i < textureSize; ++i)
				{
					GLuint textureID = (Texture[i]) ? static_cast<TOpenGLTexture*>(Texture[i])->getOpenGLTextureName() : 0;

					if (textureID != 0)
					{
						AssignedTexture[i] = GL_COLOR_ATTACHMENT0 + i;
						glFramebufferTexture2D(GL_FRAMEBUFFER, AssignedTexture[i], GL_TEXTURE_2D, textureID, 0);
					}
					else if (AssignedTexture[i] != GL_NONE)
					{
						AssignedTexture[i] = GL_NONE;
						glFramebufferTexture2D(GL_FRAMEBUFFER, AssignedTexture[i], GL_TEXTURE_2D, 0, 0);

						os::Printer::log("Error: Could not set render target.", ELL_ERROR);
					}
				}

				// Reset other render target channels.

				for (u32 i = textureSize; i < AssignedTexture.size(); ++i)
				{
					if (AssignedTexture[i] != GL_NONE)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, AssignedTexture[i], GL_TEXTURE_2D, 0, 0);
						AssignedTexture[i] = GL_NONE;
					}
				}

				RequestTextureUpdate = false;
			}

			// Set depth and stencil attachments.

			if (RequestDepthStencilUpdate)
			{
				const ECOLOR_FORMAT textureFormat = (DepthStencil) ? DepthStencil->getColorFormat() : ECF_UNKNOWN;

				if (IImage::isDepthFormat(textureFormat))
				{
					GLuint textureID = static_cast<TOpenGLTexture*>(DepthStencil)->getOpenGLTextureName();

					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

					if (textureFormat == ECF_D24S8)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

						AssignedStencil = true;
					}
					else
					{
						if (AssignedStencil)
							glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
                    
						AssignedStencil = false;
					}

					AssignedDepth = true;
				}
				else
				{
					if (AssignedDepth)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

					if (AssignedStencil)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

					AssignedDepth = false;
					AssignedStencil = false;
				}

				RequestDepthStencilUpdate = false;
			}

			// Configure drawing operation.

			if (ColorAttachment > 0 && BufferID != 0)
			{
				const u32 textureSize = Texture.size();

				if (textureSize == 0)
                    glDrawBuffer(GL_NONE);
				else if (textureSize == 1 || MultipleRenderTarget == 0)
                    glDrawBuffer(GL_COLOR_ATTACHMENT0);
				else
				{
					const u32 bufferCount = core::min_(MultipleRenderTarget, core::min_(textureSize, AssignedTexture.size()));

                    glDrawBuffers(bufferCount, AssignedTexture.pointer());
				}
			}

//#ifdef _DEBUG
			checkFBO(Driver);
//#endif
            Driver->testGLError();
		}
	}

	GLuint getBufferID() const
	{
		return BufferID;
	}

	const core::dimension2d<u32>& getSize() const
	{
		return Size;
	}

	ITexture* getTexture() const
	{
		for (u32 i = 0; i < Texture.size(); ++i)
		{
			if (Texture[i])
				return Texture[i];
		}

		return 0;
	}

protected:
	bool checkFBO(TOpenGLDriver* driver)
	{
		if (ColorAttachment == 0)
			return true;
		
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		switch (status)
		{
			case GL_FRAMEBUFFER_COMPLETE:
				return true;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				os::Printer::log("FBO has invalid read buffer", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				os::Printer::log("FBO has invalid draw buffer", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				os::Printer::log("FBO has one or several incomplete image attachments", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
				os::Printer::log("FBO has one or several image attachments with different internal formats", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
				os::Printer::log("FBO has one or several image attachments with different dimensions", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				os::Printer::log("FBO missing an image attachment", ELL_ERROR);
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				os::Printer::log("FBO format unsupported", ELL_ERROR);
				break;
			default:
				os::Printer::log("FBO error", ELL_ERROR);
				break;
		}

		return false;
	}

	core::array<GLenum> AssignedTexture;
	bool AssignedDepth;
	bool AssignedStencil;

	bool RequestTextureUpdate;
	bool RequestDepthStencilUpdate;

	GLuint BufferID;

	core::dimension2d<u32> Size;

	u32 ColorAttachment;
	u32 MultipleRenderTarget;

	TOpenGLDriver* Driver;
};

typedef COpenGLCoreRenderTarget<COpenGLDriver, COpenGLTexture> COpenGLRenderTarget;

}
}

#endif
#endif
