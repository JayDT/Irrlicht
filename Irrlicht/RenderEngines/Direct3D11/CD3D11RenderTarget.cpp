// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CD3D11RenderTarget.h"

#include "IImage.h"
#include "irrMath.h"
#include "irrString.h"

#include "CD3D11Driver.h"
#include "CD3D11Texture.h"

using namespace irr;
using namespace video;


CD3D11RenderTarget::CD3D11RenderTarget(CD3D11Driver* driver)
    : DepthStencilSurface(0)
    , Driver(driver)
{
#ifdef _DEBUG
    setDebugName("CD3D11RenderTarget");
#endif

    this->DriverType = EDT_DIRECT3D11;
    Size = Driver->getScreenSize();
}

CD3D11RenderTarget::~CD3D11RenderTarget()
{
    for (u32 i = 0; i < Surface.size(); ++i)
    {
        if (Surface[i])
            Surface[i]->Release();
    }

    if (DepthStencilSurface)
        DepthStencilSurface->Release();

    for (u32 i = 0; i < Texture.size(); ++i)
    {
        if (Texture[i])
            Texture[i]->drop();
    }

    if (DepthStencil)
        DepthStencil->drop();
}

void CD3D11RenderTarget::setTexture(const core::array<ITexture*>& texture, ITexture* depthStencil)
{
    bool textureUpdate = (Texture != texture) ? true : false;
    bool depthStencilUpdate = (DepthStencil != depthStencil) ? true : false;

    if (textureUpdate || depthStencilUpdate)
    {
        // Set color attachments.
    
        if (textureUpdate)
        {
            if (texture.size() > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
            {
                core::stringc message = "This GPU supports up to ";
                message += D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
                message += " textures per render target.";
    
                os::Printer::log(message.c_str(), ELL_WARNING);
            }
    
            const u32 size = core::min_(texture.size(), u32(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT));
    
            for (u32 i = 0; i < Surface.size(); ++i)
            {
                if (Surface[i])
                    Surface[i]->Release();
            }
    
            Surface.set_used(size);
    
            for (u32 i = 0; i < Texture.size(); ++i)
            {
                if (Texture[i])
                    Texture[i]->drop();
            }
    
            Texture.set_used(size);
    
            for (u32 i = 0; i < size; ++i)
            {
                CD3D11Texture* currentTexture = (texture[i] && texture[i]->getDriverType() == DriverType) ? static_cast<CD3D11Texture*>(texture[i]) : 0;
    
                if (currentTexture && currentTexture->getType() != E_TEXTURE_TYPE::e2D)
                    os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);
    
                if (currentTexture)
                {
                    Texture[i] = texture[i];
                    Texture[i]->grab();
    
                    Surface[i] = currentTexture->getRenderTargetView();
                }
                else
                {
                    Surface[i] = 0;
                    Texture[i] = 0;
                }
            }
        }
    
        // Set depth and stencil attachments.
    
        if (depthStencilUpdate)
        {
            if (DepthStencilSurface)
            {
                DepthStencilSurface->Release();
                DepthStencilSurface = 0;
            }
    
            if (DepthStencil)
            {
                DepthStencil->drop();
                DepthStencil = 0;
    
                DepthStencilSurface = 0;
            }
    
            CD3D11Texture* currentTexture = (depthStencil && depthStencil->getDriverType() == DriverType) ? static_cast<CD3D11Texture*>(depthStencil) : 0;

            if (currentTexture && currentTexture->getType() != E_TEXTURE_TYPE::e2D)
                os::Printer::log("This driver doesn't support render to cubemaps.", ELL_WARNING);

            if (currentTexture)
            {
                const ECOLOR_FORMAT textureFormat = (depthStencil) ? depthStencil->getColorFormat() : ECF_UNKNOWN;
    
                if (IImage::isDepthFormat(textureFormat))
                {
                    DepthStencil = depthStencil;
                    DepthStencil->grab();
    
                    DepthStencilSurface = currentTexture->getRenderTargetView();
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
    }
}

const core::dimension2d<u32>& CD3D11RenderTarget::getSize() const
{
    return Size;
}

ID3D11RenderTargetView* CD3D11RenderTarget::getSurface(u32 id) const
{
    return (id < Surface.size()) ? Surface[id] : 0;
}

u32 CD3D11RenderTarget::getSurfaceCount() const
{
    return Surface.size();
}

ID3D11RenderTargetView* CD3D11RenderTarget::getDepthStencilSurface() const
{
    return DepthStencilSurface;
}

void CD3D11RenderTarget::releaseSurfaces()
{
    for (u32 i = 0; i < Surface.size(); ++i)
    {
        if (Surface[i])
        {
            Surface[i]->Release();
            Surface[i] = 0;
        }
    }

    if (DepthStencilSurface)
    {
        DepthStencilSurface->Release();
        DepthStencilSurface = 0;
    }
}

void CD3D11RenderTarget::generateSurfaces()
{
    for (u32 i = 0; i < Surface.size(); ++i)
    {
        if (!Surface[i] && Texture[i])
        {
            Surface[i] = static_cast<CD3D11Texture*>(Texture[i])->getRenderTargetView();
        }
    }

    if (!DepthStencilSurface && DepthStencil)
    {
        DepthStencilSurface = static_cast<CD3D11Texture*>(DepthStencil)->getRenderTargetView();
    }
}
