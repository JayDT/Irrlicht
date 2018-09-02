// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CVulkanRenderTarget.h"

#include "IImage.h"
#include "irrMath.h"
#include "irrString.h"

#include "CVulkanDriver.h"
#include "CVulkanTexture.h"
#include "CVulkanFramebuffer.h"
#include "CVulkanCommandBuffer.h"

using namespace irr;
using namespace video;


CVulkanRenderTarget::CVulkanRenderTarget(CVulkanDriver* driver)
    : FrameBuffer(0)
    , Driver(driver)
{
#ifdef _DEBUG
    setDebugName("CVulkanRenderTarget");
#endif

    this->DriverType = EDT_VULKAN;
    Size = Driver->getScreenSize();
    Sync = new VulkanSemaphore(driver);
}

CVulkanRenderTarget::~CVulkanRenderTarget()
{
    if (FrameBuffer)
        FrameBuffer->drop();

    if (Sync)
        Sync->drop();
}

void CVulkanRenderTarget::setTexture(const core::array<ITexture*>& texture, ITexture* depthStencil)
{
    bool textureUpdate = (Texture != texture) ? true : false;
    bool depthStencilUpdate = (DepthStencil != depthStencil) ? true : false;

    if (textureUpdate || depthStencilUpdate)
    {
        // Set color attachments.
        if (FrameBuffer)
            FrameBuffer->drop();
        FrameBuffer = nullptr;

        Surface = {};

        if (textureUpdate)
        {
            if (texture.size() > _MAX_MULTIPLE_RENDER_TARGETS)
            {
                core::stringc message = "This GPU supports up to ";
                message += _MAX_MULTIPLE_RENDER_TARGETS;
                message += " textures per render target.";
    
                os::Printer::log(message.c_str(), ELL_WARNING);
            }
    
            const u32 size = core::min_(texture.size(), u32(_MAX_MULTIPLE_RENDER_TARGETS));
    
            for (u32 i = 0; i < Texture.size(); ++i)
            {
                if (Texture[i])
                    Texture[i]->drop();
            }
    
            Texture.set_used(size);
    
            for (u32 i = 0; i < size; ++i)
            {
                CVulkanTexture* currentTexture = (texture[i] && texture[i]->getDriverType() == DriverType) ? static_cast<CVulkanTexture*>(texture[i]) : 0;
    
                if (currentTexture && currentTexture->getType() != E_TEXTURE_TYPE::e2D)
                    os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);
    
                if (currentTexture)
                {
                    Texture[i] = texture[i];
                    Texture[i]->grab();
    
                    Surface.color[i].baseLayer = 0;
                    Surface.color[i].format = VulkanUtility::getPixelFormat(currentTexture->getColorFormat());
                    Surface.color[i].image = currentTexture->GetVkImages(0);
                }
                else
                {
                    Surface.color[i].image = 0;
                    Texture[i] = 0;
                }
            }
        }
    
        // Set depth and stencil attachments.
    
        if (depthStencilUpdate)
        {
            if (DepthStencil)
            {
                DepthStencil->drop();
                DepthStencil = 0;
        
            }
        
            CVulkanTexture* currentTexture = (depthStencil && depthStencil->getDriverType() == DriverType) ? static_cast<CVulkanTexture*>(depthStencil) : 0;
        
            if (currentTexture && currentTexture->getType() != E_TEXTURE_TYPE::e2D)
                os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);
        
            if (currentTexture)
            {
                const ECOLOR_FORMAT textureFormat = (depthStencil) ? depthStencil->getColorFormat() : ECF_UNKNOWN;
        
                if (IImage::isDepthFormat(textureFormat))
                {
                    DepthStencil = depthStencil;
                    DepthStencil->grab();
        
                    Surface.depth.baseLayer = 0;
                    Surface.depth.format = VulkanUtility::getPixelFormat(currentTexture->getColorFormat());
                    Surface.depth.image = currentTexture->GetVkImages(0);
                    Surface.depth.image->grab();
                }
            }
        }
        
        // Set size required for a viewport.
        
        bool sizeDetected = false;
        
        for (u32 i = 0; i < Texture.size(); ++i)
        {
            if (Texture[i])
            {
                Size = Texture[i]->getSize();
                sizeDetected = true;
        
                break;
            }
        }
        
        if (!sizeDetected)
        {
            if (DepthStencil)
                Size = DepthStencil->getSize();
            else
                Size = Driver->getScreenSize();
        }


        Surface.width = Size.Width;
        Surface.height = Size.Height;
        Surface.layers = 1;
        Surface.numSamples = 1;
        Surface.offscreen = true;

        FrameBuffer = new VulkanFramebuffer(Driver, Surface);
    }
}

const core::dimension2d<u32>& CVulkanRenderTarget::getSize() const
{
    return Size;
}

void CVulkanRenderTarget::releaseSurfaces()
{

}

void CVulkanRenderTarget::generateSurfaces()
{
}
