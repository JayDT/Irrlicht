// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COpenGLDriver.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "COpenGLTexture.h"
#include "COpenGLMaterialRenderer.h"
#include "COpenGLShaderMaterialRenderer.h"
#include "COpenGLSLMaterialRenderer.h"
#include "COpenGLCgMaterialRenderer.h"
#include "COpenGLNormalMapRenderer.h"
#include "COpenGLParallaxMapRenderer.h"
#include "COpenGLHardwareBuffer.h"
#include "COpenGLVertexDeclaration.h"
#include "COpenGLCoreRenderTarget.h"
#include "os.h"
#include "IVertexBuffer.h"
#include "buildin_data.h"

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "CIrrDeviceWin32.h"

#pragma comment(lib, "Glu32.lib")
#endif

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
#include "MacOSX/CIrrDeviceMacOSX.h"
#endif

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
#include <SDL2/SDL.h>
#include "CIrrDeviceSDL.h"
#endif

#include "standard/client/datasource_standard.h"

#include <stdio.h>
#include <iostream>

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
#include "ContextManager/CWGLManager.h"
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#include "ContextManager/CGLXManager.h"
#else
#error invalid platform
#include "ContextManager/CNSOGLManager.h"
#endif

#ifdef HAVE_CSYSTEM_EXTENSION
#include "System/Uri.h"
#include "CFramework/System/Resource/ResourceManager.h"

extern bool InitializeModulResourceOpenGL();
#endif

//#include <GL/ex

#define _DEBUG_OGL

using namespace irr;
using namespace irr::video;

extern core::matrix4 OGL_UnitMatrix;
extern core::matrix4 OGL_SphereMapMatrix;

void APIENTRY openglCallbackFunction(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar* message,
                                           const void* userParam)
{
#ifdef _DEBUG_OGL
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return;
    }
    // GL_DEBUG_SOURCE_API_ARB
    std::cout << "---------------------opengl-callback-start------------" << std::endl;
    std::cout << "source: " << source << std::endl;
    std::cout << "message: "<< message << std::endl;
    std::cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER";
        break;
    }
    std::cout << std::endl;

    std::cout << "id: " << id << std::endl;
    std::cout << "severity: ";
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "HIGH";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "NOTIFICATION";
        break;
    }
    std::cout << std::endl;
    std::cout << "---------------------opengl-callback-end--------------" << std::endl;
#endif
}

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_) || defined(_IRR_COMPILE_WITH_X11_DEVICE_) || defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
COpenGLDriver::COpenGLDriver(const SIrrlichtCreationParameters& params, io::IFileSystem* io, IContextManager* contextManager)
: CNullDriver(io, params.WindowSize), COpenGLExtensionHandler(),
    ResetRenderStates(true), Transformation3DChanged(true),
    AntiAlias(params.AntiAlias), RenderTargetTexture(0),
    CurrentRendertargetSize(0,0), ColorFormat(ECF_R8G8B8),
    CurrentTarget(ERT_FRAME_BUFFER), Params(params),
    ContextManager(contextManager),
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
    DeviceType(EIDT_WIN32)
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
    DeviceType(EIDT_X11)
#else
    DeviceType(EIDT_OSX)
#endif
{
    memset(m_defaultShader, 0, sizeof(m_defaultShader));
    memset(DynamicHardwareBuffer, 0, sizeof(DynamicHardwareBuffer));
    memset(CurrentTexture, 0, sizeof(CurrentTexture));

    #ifdef _DEBUG
    setDebugName("COpenGLDriver");
    #endif

    #ifdef _IRR_COMPILE_WITH_CG_
    CgContext = 0;
    #endif

    // create sphere map matrix
    OGL_SphereMapMatrix = core::matrix4(0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

#ifdef HAVE_CSYSTEM_EXTENSION
    InitializeModulResourceOpenGL();
#endif
}
#endif

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
COpenGLDriver::COpenGLDriver(const SIrrlichtCreationParameters& params, io::IFileSystem* io, CIrrDeviceSDL* device)
    : CNullDriver(io, params.WindowSize), COpenGLExtensionHandler(),
    ResetRenderStates(true), Transformation3DChanged(true),
    AntiAlias(params.AntiAlias), RenderTargetTexture(0),
    CurrentRendertargetSize(0, 0), ColorFormat(ECF_R8G8B8),
    CurrentTarget(ERT_FRAME_BUFFER), Params(params),
    ContextManager(nullptr), DeviceType(EIDT_SDL),
    SDLDevice(device)
{
    memset(m_defaultShader, 0, sizeof(m_defaultShader));
    memset(DynamicHardwareBuffer, 0, sizeof(DynamicHardwareBuffer));
    memset(CurrentTexture, 0, sizeof(CurrentTexture));

#ifdef _DEBUG
    setDebugName("COpenGLDriver");
#endif

    genericDriverInit();
}

#endif

//! destructor
COpenGLDriver::~COpenGLDriver()
{
    #ifdef _IRR_COMPILE_WITH_CG_
    if (CgContext)
        cgDestroyContext(CgContext);
    #endif

    SetShader(nullptr);

    for (auto rtt : RenderTargets)
    {
        rtt->drop();
    }

    RenderTargets.clear();

    RequestedLights.clear();

    deleteMaterialRenders();

    // I get a blue screen on my laptop, when I do not delete the
    // textures manually before releasing the dc. Oh how I love this.
    deleteAllTextures();
    removeAllOcclusionQueries();
    removeAllHardwareBuffers();

    for (s32 i = 0; i < (s32)E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE; ++i)
    {
        if (!DynamicHardwareBuffer[i])
            continue;

        DynamicHardwareBuffer[i]->drop();
    }

    if (ContextManager)
    {
        ContextManager->destroyContext();
        ContextManager->destroySurface();
        ContextManager->terminate();
        ContextManager->drop();
    }
}

//! inits the open gl driver
bool COpenGLDriver::initDriver()
{
    CNullDriver::initDriver();

    ContextManager->generateSurface();
    ContextManager->generateContext();
    ExposedData = ContextManager->getContext();
    ContextManager->activateContext(ExposedData);

    genericDriverInit();

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_) || defined(_IRR_COMPILE_WITH_X11_DEVICE_)
    extGlSwapInterval(Params.Vsync ? 1 : 0);
#endif

#ifdef _DEBUG_OGL
    GetStateCache()->setEnabled(GL_DEBUG_OUTPUT_SYNCHRONOUS, true);
    glDebugMessageCallback((GLDEBUGPROC)openglCallbackFunction, nullptr);
    GLuint unusedIds = 0;
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, &unusedIds, true);
#endif

    testGLError();

    extGlSwapInterval(Params.Vsync ? 1 : 0);

    if (DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_STANDARD])
        DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_STANDARD]->drop();
    DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_STANDARD] = nullptr;

    if (DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_2TCOORDS])
        DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_2TCOORDS]->drop();
    DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_2TCOORDS] = nullptr;

    if (DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_SKINNING])
        DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_SKINNING]->drop();
    DynamicHardwareBuffer[E_VERTEX_TYPE::EVT_SKINNING] = nullptr;

    System::IO::StandardDataSource fileMgr;
    {
        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = new IrrDefaultShaderVertexCallBack;
        shaderCI.Callback["PixelConstats"] = new IrrDefaultShaderFragmentCallBack;
        if (m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD])
            delete m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD];
        m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD] = static_cast<GLSLGpuShader*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    {


        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/DefaultSH.vert");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/DefaultSH.vert", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = new IrrDefaultShaderVertexCallBack;
        if (m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW])
            delete m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW];
        m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW] = static_cast<GLSLGpuShader*>(createShader(&shaderCI));

        delete vertShader;
    }

    {


        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/DefaultT2.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/DefaultT2.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = new IrrDefaultShaderVertexCallBack;
        shaderCI.Callback["PixelConstats"] = new IrrDefaultShaderFragmentCallBack;
        if (m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS])
            delete m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS];
        m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS] = static_cast<GLSLGpuShader*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    {


        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/DefaultSK.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/DefaultSK.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = new IrrDefaultShaderVertexCallBack;
        shaderCI.Callback["PixelConstats"] = new IrrDefaultShaderFragmentCallBack;
        if (m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING])
            delete m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING];
        m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING] = static_cast<GLSLGpuShader*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    testGLError();

    return true;
}

// -----------------------------------------------------------------------
// METHODS
// -----------------------------------------------------------------------

bool COpenGLDriver::genericDriverInit()
{
    if (ContextManager)
        ContextManager->grab();

    Name = L"OpenGL ";
    Name.append(glGetString(GL_VERSION));
    s32 pos = Name.findNext(L' ', 7);
    if (pos != -1)
        Name = Name.subString(0, pos);
    printVersion();

    // print renderer information
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    if (renderer && vendor)
    {
        os::Printer::log(reinterpret_cast<const c8*>(renderer), reinterpret_cast<const c8*>(vendor), ELL_INFORMATION);
        VendorName = reinterpret_cast<const c8*>(vendor);
    }

    u32 i;

    // load extensions
    initExtensions(Params.Stencilbuffer);

    // reset cache handler
    m_stateCache.clearCache();
    m_stateCache.initializeCache();

    if (queryFeature(EVDF_ARB_GLSL))
    {
        char buf[32];
        const u32 maj = ShaderLanguageVersion / 100;
        snprintf_irr(buf, 32, "%u.%u", maj, ShaderLanguageVersion - maj * 100);
        os::Printer::log("GLSL version", buf, ELL_INFORMATION);
    }
    else
        os::Printer::log("GLSL not available.", ELL_INFORMATION);
    DriverAttributes->setAttribute("MaxTextures", (s32)MaxTextureUnits);
    DriverAttributes->setAttribute("MaxSupportedTextures", (s32)MaxSupportedTextures);
    DriverAttributes->setAttribute("MaxLights", MaxLights);
    DriverAttributes->setAttribute("MaxAnisotropy", MaxAnisotropy);
    DriverAttributes->setAttribute("MaxUserClipPlanes", MaxUserClipPlanes);
    DriverAttributes->setAttribute("MaxAuxBuffers", MaxAuxBuffers);
    DriverAttributes->setAttribute("MaxMultipleRenderTargets", (s32)MaxMultipleRenderTargets);
    DriverAttributes->setAttribute("MaxIndices", (s32)MaxIndices);
    DriverAttributes->setAttribute("MaxTextureSize", (s32)MaxTextureSize);
    DriverAttributes->setAttribute("MaxGeometryVerticesOut", (s32)MaxGeometryVerticesOut);
    DriverAttributes->setAttribute("MaxTextureLODBias", MaxTextureLODBias);
    DriverAttributes->setAttribute("Version", Version);
    DriverAttributes->setAttribute("ShaderLanguageVersion", ShaderLanguageVersion);
    DriverAttributes->setAttribute("AntiAlias", AntiAlias);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    UserClipPlanes.reallocate(MaxUserClipPlanes);
    for (i = 0; i<MaxUserClipPlanes; ++i)
        UserClipPlanes.push_back(SUserClipPlane());

    for (i = 0; i<ETS_COUNT; ++i)
        setTransform(static_cast<E_TRANSFORMATION_STATE>(i), core::IdentityMatrix);

    //setAmbientLight(SColorf(0.0f, 0.0f, 0.0f, 0.0f));

    Params.HandleSRGB &= ((FeatureAvailable[IRR_ARB_framebuffer_sRGB] || FeatureAvailable[IRR_EXT_framebuffer_sRGB]) &&
        FeatureAvailable[IRR_EXT_texture_sRGB]);
#if defined(GL_ARB_framebuffer_sRGB)
    if (Params.HandleSRGB)
        glEnable(GL_FRAMEBUFFER_SRGB);
#elif defined(GL_EXT_framebuffer_sRGB)
    if (Params.HandleSRGB)
        glEnable(GL_FRAMEBUFFER_SRGB_EXT);
#endif

    // This is a fast replacement for NORMALIZE_NORMALS
    //	if ((Version>101) || FeatureAvailable[IRR_EXT_rescale_normal])
    //		glEnable(GL_RESCALE_NORMAL_EXT);

    glClearDepth(1.0);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
    //glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);

    glFrontFace(GL_CW);
    // adjust flat coloring scheme to DirectX version
#if defined(GL_ARB_provoking_vertex) || defined(GL_EXT_provoking_vertex)
    extGlProvokingVertex(GL_FIRST_VERTEX_CONVENTION_EXT);
#endif

    // Create built-in 2D quad for 2D rendering (both quads and lines).
    Quad2DVertices[0] = S3DVertex(core::vector3df(-1.0f, 1.0f, 0.0f), core::vector3df(0.0f, 0.0f, 0.0f), SColor(255, 255, 255, 255), core::vector2df(0.0f, 1.0f));
    Quad2DVertices[1] = S3DVertex(core::vector3df(1.0f, 1.0f, 0.0f), core::vector3df(0.0f, 0.0f, 0.0f), SColor(255, 255, 255, 255), core::vector2df(1.0f, 1.0f));
    Quad2DVertices[2] = S3DVertex(core::vector3df(1.0f, -1.0f, 0.0f), core::vector3df(0.0f, 0.0f, 0.0f), SColor(255, 255, 255, 255), core::vector2df(1.0f, 0.0f));
    Quad2DVertices[3] = S3DVertex(core::vector3df(-1.0f, -1.0f, 0.0f), core::vector3df(0.0f, 0.0f, 0.0f), SColor(255, 255, 255, 255), core::vector2df(0.0f, 0.0f));

    // create material renderers
    createMaterialRenderers();

    // set the renderstates
    setRenderStates3DMode();

    // set fog mode
    setFog(FogColor, FogType, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

    // create matrix for flipping textures
    TextureFlipMatrix.buildTextureTransform(0.0f, core::vector2df(0, 0), core::vector2df(0, 1.0f), core::vector2df(1.0f, -1.0f));

    // We need to reset once more at the beginning of the first rendering.
    // This fixes problems with intermediate changes to the material during texture load.
    ResetRenderStates = true;

    #ifdef _IRR_COMPILE_WITH_CG_
    CgContext = cgCreateContext();
    #endif

    return true;
}

void COpenGLDriver::createMaterialRenderers()
{
    // create OpenGL material renderers

    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID_2_LAYER(this));

    // add the same renderer for all lightmap types
    COpenGLMaterialRenderer_LIGHTMAP* lmr = new COpenGLMaterialRenderer_LIGHTMAP(this);
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:
    addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:
    lmr->drop();

    // add remaining material renderer
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_DETAIL_MAP(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SPHERE_MAP(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_REFLECTION_2_LAYER(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ADD_COLOR(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_VERTEX_ALPHA(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(this));

    // add normal map renderers
    s32 tmp = 0;
    video::IMaterialRenderer* renderer = 0;
    renderer = nullptr; // new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_SOLID].Renderer);
    //renderer->drop();
    renderer = nullptr; //new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);
    //renderer->drop();
    renderer = nullptr; //new COpenGLNormalMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_VERTEX_ALPHA].Renderer);
    //renderer->drop();

    // add parallax map renderers
    renderer = nullptr; //new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_SOLID].Renderer);
    //renderer->drop();
    renderer = nullptr; //new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);
    //renderer->drop();
    renderer = nullptr; //new COpenGLParallaxMapRenderer(this, tmp, MaterialRenderers[EMT_TRANSPARENT_VERTEX_ALPHA].Renderer);
    //renderer->drop();

    // add basic 1 texture blending
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_ONETEXTURE_BLEND(this));
    addAndDropMaterialRenderer(new COpenGLMaterialRenderer_RAW(this));
}


//! presents the rendered scene on the screen, returns false if failed
bool COpenGLDriver::endScene()
{
    CNullDriver::endScene();

    SetShader(nullptr);

    glFlush();

    bool status = false;

    if (ContextManager)
        status = ContextManager->swapBuffers();

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
    if (DeviceType == EIDT_SDL)
    {
        SDL_GL_SwapWindow(SDLDevice->getInternalWindow());
        status = true;
    }
#endif

    // todo: console device present

    return false;
}


//! clears the zbuffer and color buffer
void COpenGLDriver::clearBuffers(bool backBuffer, bool zBuffer, bool stencilBuffer, SColor color)
{
    GLbitfield mask = 0;
    if (backBuffer)
    {
        m_stateCache.setColourMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        const f32 inv = 1.0f / 255.0f;
        glClearColor(color.getRed() * inv, color.getGreen() * inv,
            color.getBlue() * inv, color.getAlpha() * inv);

        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (zBuffer)
    {
        LastMaterial.ZWriteEnable=true;
        m_stateCache.setDepthMask(GL_TRUE);
        glClearDepth(1.0f);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (stencilBuffer)
    {
        glClearStencil(0);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    if (mask)
        glClear(mask);
}


//! init call for rendering start
bool COpenGLDriver::beginScene(bool backBuffer, bool zBuffer, SColor color,
        const SExposedVideoData& videoData, core::rect<s32>* sourceRect)
{
    testGLError();

    CNullDriver::beginScene(backBuffer, zBuffer, color, videoData, sourceRect);

    if (ContextManager)
        ContextManager->activateContext(videoData);

#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
    if (DeviceType == EIDT_SDL)
        glFrontFace(GL_CW);
#endif

    clearBuffers(backBuffer, zBuffer, true, color);

    glViewport(ViewPort.UpperLeftCorner.X, ViewPort.UpperLeftCorner.Y, ViewPort.getWidth(), ViewPort.getHeight());


    return true;
}


//! Returns the transformation set by setTransform
const core::matrix4& COpenGLDriver::getTransform(E_TRANSFORMATION_STATE state) const
{
    return Matrices[state];
}


//! sets transformation
void COpenGLDriver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
    Matrices[state] = mat;
    Transformation3DChanged = true;

    if (state == ETS_PROJECTION || state == ETS_VIEW)
        Matrices[ETS_PROJECTION_VIEW] = Matrices[ETS_PROJECTION] * Matrices[ETS_VIEW];
}

bool COpenGLDriver::updateVertexHardwareBuffer(COpenGLHardwareBuffer *HWBuffer, E_HARDWARE_BUFFER_TYPE Type)
{
    if (!HWBuffer)
        return false;

    if (!FeatureAvailable[IRR_ARB_vertex_buffer_object])
        return false;

    E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
    const scene::IMeshBuffer* mb = HWBuffer->GetBuffer();
    const E_VERTEX_TYPE vType = mb->getVertexType();

    if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX)
    {
        if ( HWBuffer->GetBuffer()->getHardwareMappingHint_Vertex() == scene::EHM_DYNAMIC)
            MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

        const void* vertices = mb->getVertices();
        const u32 vertexCount = mb->getVertexCount();
        const u32 vertexSize = HWBuffer->GetBuffer()->GetVertexStride() ? HWBuffer->GetBuffer()->GetVertexStride() : getVertexPitchFromType(vType);
        const u32 bufSize = vertexSize * vertexCount;

        const c8* vbuf = static_cast<const c8*>(vertices);
        HWBuffer->UpdateBuffer(Type, MemoryAccess, vbuf, bufSize);
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM)
    {
        irr::scene::IStreamBuffer * streamBuffer = mb->getStreamBuffer();
        const void* vertices = streamBuffer->getData();
        const u32 vertexCount = streamBuffer->size();
        const u32 vertexSize = streamBuffer->stride();
        const u32 vertexBufSize = vertexSize * vertexCount;

        if (HWBuffer->GetBuffer()->getHardwareMappingHint_Instance() == scene::EHM_DYNAMIC)
            MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

        HWBuffer->UpdateBuffer(Type, MemoryAccess, vertices, vertexBufSize);
    }
    return true;
}

bool COpenGLDriver::updateIndexHardwareBuffer(COpenGLHardwareBuffer *HWBuffer)
{
    if (!HWBuffer)
        return false;

    if (!FeatureAvailable[IRR_ARB_vertex_buffer_object])
        return false;

    const scene::IMeshBuffer* mb = HWBuffer->GetBuffer();
    const void* indices = mb->getIndices();
    u32 indexCount = mb->getIndexCount();

    GLenum indexSize;
    switch (mb->getIndexType())
    {
        case EIT_16BIT:
        {
            indexSize = sizeof(u16);
            break;
        }
        case EIT_32BIT:
        {
            indexSize = sizeof(u32);
            break;
        }
        default:
        {
            return false;
        }
    }
    const u32 bufSize = indexSize * indexCount;

    E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
    if (HWBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_DYNAMIC)
        MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

    HWBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, MemoryAccess, indices, bufSize);
    return true;
}

//! updates hardware buffer if needed
bool COpenGLDriver::updateHardwareBuffer(IHardwareBuffer *hwBuffer)
{
    if (!hwBuffer)
        return false;

    if (!hwBuffer->GetBuffer()->GetGPUProgram())
    {
        _IRR_DEBUG_BREAK_IF(!m_defaultShader[hwBuffer->GetBuffer()->getVertexType()]);
        hwBuffer->GetBuffer()->SetGPUProgram(m_defaultShader[hwBuffer->GetBuffer()->getVertexType()]);
    }

    if (hwBuffer->GetBuffer()->getHardwareMappingHint_Vertex() == scene::EHM_NEVER || hwBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_NEVER)
        hwBuffer->GetBuffer()->setHardwareMappingHint(scene::EHM_DYNAMIC);

    {
        if (hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX) != hwBuffer->GetBuffer()->getChangedID_Vertex())
        {
            if (!updateVertexHardwareBuffer((COpenGLHardwareBuffer*)hwBuffer, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX))
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, hwBuffer->GetBuffer()->getChangedID_Vertex());
        }

        if (hwBuffer->GetBuffer()->getStreamBuffer() && hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM) != hwBuffer->GetBuffer()->getStreamBuffer()->getChangedID())
        {
            if (!updateVertexHardwareBuffer((COpenGLHardwareBuffer*)hwBuffer, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM))
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM, hwBuffer->GetBuffer()->getStreamBuffer()->getChangedID());
        }
    }

    {
        if (hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX) != hwBuffer->GetBuffer()->getChangedID_Index())
        {
            if (!updateIndexHardwareBuffer((COpenGLHardwareBuffer*)hwBuffer))
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, hwBuffer->GetBuffer()->getChangedID_Vertex());
        }
    }

    return true;
}

//! Create hardware buffer from meshbuffer
IHardwareBuffer *COpenGLDriver::createHardwareBuffer(const scene::IMeshBuffer* mb)
{
    if (!mb /*|| (mb->getHardwareMappingHint_Index()==scene::EHM_NEVER && mb->getHardwareMappingHint_Vertex()==scene::EHM_NEVER)*/)
        return nullptr;

    _IRR_DEBUG_BREAK_IF(testGLError());

    if (!mb->GetVertexDeclaration())
        mb->SetVertexDeclaration(this->GetVertexDeclaration(mb->getVertexType()));

    COpenGLHardwareBuffer *HWBuffer=new COpenGLHardwareBuffer(this, (scene::IMeshBuffer*)mb,
        E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT,
        mb->getIndexType() == video::EIT_16BIT ? (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_16_BITS : (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_32_BITS,
        static_cast<COpenGLVertexDeclaration*>(mb->GetVertexDeclaration()));

    if (!updateHardwareBuffer(HWBuffer))
    {
        HWBuffer->Unbind();
        deleteHardwareBuffer(HWBuffer);
        return nullptr;
    }

    return HWBuffer;
}

video::VertexDeclaration * COpenGLDriver::createVertexDeclaration()
{
    return new COpenGLVertexDeclaration(this);
}

void COpenGLDriver::deleteHardwareBuffer(IHardwareBuffer *_HWBuffer)
{
    if (!_HWBuffer)
        return;

    CNullDriver::deleteHardwareBuffer(_HWBuffer);
}

void COpenGLDriver::drawMeshBuffer3DBox(const scene::IMeshBuffer* mb)
{
}

void irr::video::COpenGLDriver::drawMeshBuffer(const scene::IMeshBuffer * mb, scene::IMesh * mesh, scene::ISceneNode * node)
{
    if (!mb || !mb->getIndexCount())
        return;

    scene::E_PRIMITIVE_TYPE PrimitiveTopology = mb->getRenderPrimitive();
    IHardwareBuffer *HWBuffer = mb->GetHWBuffer();
    if (!HWBuffer)
    {
        CreateHardwareBuffer(mb);
        HWBuffer = mb->GetHWBuffer();
    }

    if (PrimitiveTopology == scene::E_PRIMITIVE_TYPE::EPT_TRIANGLE_FAN)
        return;

    if (!mb->GetGPUProgram())
    {
        SetShader(m_defaultShader[mb->getVertexType()]);
    }
    else
    {
        SetShader(mb->GetGPUProgram());
    }

    if (mb->Is3DMode())
    {
        if (!setRenderStates3DMode())
            return;
    }
    else
    {
        if (Material.MaterialType == EMT_ONETEXTURE_BLEND)
        {
            E_BLEND_FACTOR srcFact;
            E_BLEND_FACTOR dstFact;
            E_MODULATE_FUNC modulo;
            u32 alphaSource;
            unpack_textureBlendFunc(srcFact, dstFact, modulo, alphaSource, Material.uMaterialTypeParam);
            setRenderStates2DMode(alphaSource & video::EAS_VERTEX_COLOR, (Material.getTexture(0) != 0), (alphaSource&video::EAS_TEXTURE) != 0);
        }
        else
            setRenderStates2DMode(Material.MaterialType == EMT_TRANSPARENT_VERTEX_ALPHA, (Material.getTexture(0) != 0), Material.MaterialType == EMT_TRANSPARENT_ALPHA_CHANNEL);
    }

    if (HWBuffer)
    {
        updateHardwareBuffer(HWBuffer); //check if update is needed

        if (!HWBuffer->IsBinded() || !HWBuffer->IsManualBind())
        {
            HWBuffer->Bind();
        }
    }
    else
    {
        if (!mb->GetVertexDeclaration())
            mb->SetVertexDeclaration(this->GetVertexDeclaration(mb->getVertexType()));

        // copy vertices to dynamic buffers, if needed
        uploadVertexData(mb->getVertices(), mb->getVertexCount(), mb->getIndices(), mb->getIndexCount(), mb->getVertexType(), mb->getIndexType());
        HWBuffer = DynamicHardwareBuffer[mb->getVertexType()];
        if (!HWBuffer->IsBinded() || !HWBuffer->IsManualBind())
        {
            HWBuffer->Bind();
        }
    }

    SyncShaderConstant();
    InitDrawStates(mb);

#ifdef _DEBUG
    if (!checkPrimitiveCount(mb->getVertexCount()))
    {
        if (!HWBuffer->IsManualBind())
            HWBuffer->Unbind();
        return;
    }
#endif

    GLenum renderPrimitive = primitiveTypeToGL(mb->getRenderPrimitive());

    u32 instanceCount = (!mesh || mesh->IsInstanceModeAvailable()) && mb->getStreamBuffer() ? mb->getStreamBuffer()->size() : 1;

    if (instanceCount > 1)
        glDrawElementsInstanced(renderPrimitive, mb->GetIndexRangeCount(), mb->getIndexType() == E_INDEX_TYPE::EIT_32BIT ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (void*)(sizeof(u16) * mb->GetIndexRangeStart()), instanceCount);
    else if (mb->GetSubBufferCount())
        glDrawRangeElements(renderPrimitive, mb->GetVertexRangeStart(), mb->GetVertexRangeStart() + mb->GetVertexRangeEnd(), mb->GetIndexRangeCount(), mb->getIndexType() == E_INDEX_TYPE::EIT_32BIT ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (void*)(sizeof(u16) * mb->GetIndexRangeStart()));
    else
        glDrawElements(renderPrimitive, mb->getIndexCount(), mb->getIndexType() == E_INDEX_TYPE::EIT_32BIT ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (void*)0);

    if (!HWBuffer->IsManualBind())
        HWBuffer->Unbind();
}

void irr::video::COpenGLDriver::InitDrawStates(const scene::IMeshBuffer * mb)
{
    GLsizei maxUpdate = 0;
    bool viewChanged = false;
    for (int i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        if (CurrentTexture[i])
        {
            ++maxUpdate;
            m_stateCache.setEnabled(OGLStateEntries::OGL_TEXTURE_2D, true);
            viewChanged |= m_stateCache.bindGLTexture(i, static_cast<const COpenGLTexture*>(CurrentTexture[i])->getOpenGLTextureName(), true);
            viewChanged |= m_stateCache.bindGLSampler(i, m_stateCache.getSampler(i), false);
        }
        else
        {
            if (i == 0)
                m_stateCache.setEnabled(OGLStateEntries::OGL_TEXTURE_2D, false);
            viewChanged |= m_stateCache.bindGLTexture(i, 0, true);
            //m_stateCache.bindGLSampler(i, 0);
        }
    }

    if (viewChanged && maxUpdate)
    {
        m_stateCache.flushGLTextureBind(maxUpdate);
    }
}

//! Draw hardware buffer
void COpenGLDriver::drawHardwareBuffer(IHardwareBuffer *_HWBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
    if (!_HWBuffer)
        return;
}


//! Create occlusion query.
/** Use node for identification and mesh for occlusion test. */
void COpenGLDriver::addOcclusionQuery(scene::ISceneNode* node,
        const scene::IMesh* mesh)
{
    if (!queryFeature(EVDF_OCCLUSION_QUERY))
        return;

    //CNullDriver::addOcclusionQuery(node, mesh);
    //if (node->getOcculisionQuery() && (node->getOcculisionQuery()->UID == 0))
    //    extGlGenQueries(1, reinterpret_cast<GLuint*>(&node->getOcculisionQuery()->UID));
}


//! Remove occlusion query.
void COpenGLDriver::removeOcclusionQuery(scene::ISceneNode* node)
{
    if (node->getOcculisionQuery())
    {
        //if (node->getOcculisionQuery()->UID != 0)
        //    extGlDeleteQueries(1, reinterpret_cast<GLuint*>(&node->getOcculisionQuery()->UID));
        CNullDriver::removeOcclusionQuery(node);
    }
}


//! Run occlusion query. Draws mesh stored in query.
/** If the mesh shall not be rendered visible, use
overrideMaterial to disable the color and depth buffer. */
void COpenGLDriver::runOcclusionQuery(scene::ISceneNode* node, bool visible)
{
//    if (!node)
//        return;
//
//    if (node->getOcculisionQuery())
//    {
//        if (node->getOcculisionQuery()->UID)
//            extGlBeginQuery(
//#ifdef GL_ARB_occlusion_query
//                GL_SAMPLES_PASSED_ARB,
//#else
//                0,
//#endif
//                node->getOcculisionQuery()->UID);
//        CNullDriver::runOcclusionQuery(node,visible);
//        if (node->getOcculisionQuery()->UID)
//            extGlEndQuery(
//#ifdef GL_ARB_occlusion_query
//                GL_SAMPLES_PASSED_ARB);
//#else
//                0);
//#endif
//        testGLError();
//    }
}


//! Update occlusion query. Retrieves results from GPU.
/** If the query shall not block, set the flag to false.
Update might not occur in this case, though */
void COpenGLDriver::updateOcclusionQuery(scene::ISceneNode* node, bool block)
{
//    if (node->getOcculisionQuery())
//    {
//        // not yet started
//        if (node->getOcculisionQuery()->Run==u32(~0))
//            return;
//        GLint available = block?GL_TRUE:GL_FALSE;
//        if (!block)
//            extGlGetQueryObjectiv(node->getOcculisionQuery()->UID,
//#ifdef GL_ARB_occlusion_query
//                        GL_QUERY_RESULT_AVAILABLE_ARB,
//#elif defined(GL_NV_occlusion_query)
//                        GL_PIXEL_COUNT_AVAILABLE_NV,
//#else
//                        0,
//#endif
//                        &available);
//        testGLError();
//        if (available==GL_TRUE)
//        {
//            extGlGetQueryObjectiv(node->getOcculisionQuery()->UID,
//#ifdef GL_ARB_occlusion_query
//                        GL_QUERY_RESULT_ARB,
//#elif defined(GL_NV_occlusion_query)
//                        GL_PIXEL_COUNT_NV,
//#else
//                        0,
//#endif
//                        &available);
//
//            if (queryFeature(EVDF_OCCLUSION_QUERY))
//                node->getOcculisionQuery()->Result = available;
//        }
//        testGLError();
//    }
}


//! Return query result.
/** Return value is the number of visible pixels/fragments.
The value is a safe approximation, i.e. can be larger than the
actual value of pixels. */
u32 COpenGLDriver::getOcclusionQueryResult(scene::ISceneNode* node) const
{
    //if (node->getOcculisionQuery())
    //    return node->getOcculisionQuery()->Result;
    //else
        return ~0;
}


// small helper function to create vertex buffer object adress offsets
static inline u8* buffer_offset(const long offset)
{
    return ((u8*)0 + offset);
}


//! draws a vertex primitive list
void COpenGLDriver::drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
        const void* indexList, u32 primitiveCount,
        E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    if (!primitiveCount || !vertexCount)
        return;

    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);

    // draw everything
    setRenderStates3DMode();

    draw2D3DVertexPrimitiveList(vertices, vertexCount, vType, indexList, primitiveCount, pType, iType);
}

void COpenGLDriver::draw2D3DVertexPrimitiveList(const void* vertices, u32 vertexCount, E_VERTEX_TYPE vType,
        const void* indexList, u32 primitiveCount,
        scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    if (GetActiveShader() != m_defaultShader[vType])
    {
        SetShader(m_defaultShader[vType]);
        _IRR_DEBUG_BREAK_IF(!GetActiveShader());
    }

    size_t indexCount = getIndexAmount(pType, primitiveCount);

    uploadVertexData(vertices, vertexCount, indexList, indexCount, vType, iType);

#ifdef _DEBUG
    if (!checkPrimitiveCount(vertexCount))
        return;
#endif

    DynamicHardwareBuffer[vType]->Bind();

    GLenum renderPrimitive = primitiveTypeToGL(pType);

    SyncShaderConstant();
    InitDrawStates(nullptr);

    if (!indexCount)
        glDrawArrays(renderPrimitive, 0, vertexCount);
    else
        glDrawElements(renderPrimitive, indexCount, iType == E_INDEX_TYPE::EIT_32BIT ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, nullptr);

    DynamicHardwareBuffer[vType]->Unbind();
}

bool irr::video::COpenGLDriver::uploadVertexData(const void * vertices, u32 vertexCount, const void * indexList, u32 indexCount, E_VERTEX_TYPE vType, E_INDEX_TYPE iType)
{
    //if (!m_defaultShader && !GetActiveShader())
    //{
    //    System::IO::StandardDataSource fileMgr;
    //
    //    irr::core::stringc a;
    //
    //    auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.vert"); //System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.vert", true));
    //    auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/OpenGL/GLSL/Default.frag"); //System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/OpenGL;/GLSL/Default.frag", true));
    //
    //    m_defaultShader = static_cast<GLSLGpuShader*>(createShader(vertShader, fragShader, nullptr, nullptr));
    //    m_defaultShader->Init();
    //    m_defaultShader->addDataBuffer(new ShaderGenericValuesBuffer(video::IShaderDataBuffer::EUT_PER_FRAME_PER_MESH), nullptr);
    //
    //    activeGPUProgram(m_defaultShader);
    //}

    if (!DynamicHardwareBuffer[vType])
    {
        DynamicHardwareBuffer[vType] = new COpenGLHardwareBuffer(this, nullptr, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, 0, static_cast<COpenGLVertexDeclaration*>(GetVertexDeclaration(vType)));

        static_cast<COpenGLVertexDeclaration*>(GetVertexDeclaration(vType))->SetVertexType(vType);

        DynamicHardwareBuffer[vType]->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, vertices, getVertexPitchFromType(vType) * vertexCount);
        DynamicHardwareBuffer[vType]->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, indexList, indexCount * (iType == E_INDEX_TYPE::EIT_32BIT ? sizeof(u32) : sizeof(u16)));

        static_cast<COpenGLVertexDeclaration*>(GetVertexDeclaration(vType))->SetVertexType(E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE);

        testGLError();
    }
    else
    {
        if (vertices)
        {
            DynamicHardwareBuffer[vType]->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, vertices, getVertexPitchFromType(vType) * vertexCount);
        }

        if (indexList)
        {
            DynamicHardwareBuffer[vType]->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, indexList, indexCount * (iType == E_INDEX_TYPE::EIT_32BIT ? sizeof(u32) : sizeof(u16)));
        }
    }

    return true;
}

//! draws a vertex primitive list in 2d
void COpenGLDriver::draw2DVertexPrimitiveList(const void* vertices, u32 vertexCount,
        const void* indexList, u32 primitiveCount,
        E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    if (!primitiveCount || !vertexCount)
        return;

    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::draw2DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);

    draw2D3DVertexPrimitiveList(vertices, vertexCount, vType, indexList, primitiveCount, pType, iType);
}

//! Draws a pixel
void COpenGLDriver::drawPixel(u32 x, u32 y, const SColor &color)
{
}

bool COpenGLDriver::setActiveTexture(u32 stage, const video::ITexture* texture)
{
    if (stage >= MaxSupportedTextures)
        return false;

    if (CurrentTexture[stage]==texture)
        return true;

    // For first stage, set NullTexture if texture is NULL
    CurrentTexture[stage] = (stage == 0 && texture == 0) ? nullptr : texture;
    return true;
}


//! disables all textures beginning with the optional fromStage parameter. Otherwise all texture stages are disabled.
//! Returns whether disabling was successful or not.
bool COpenGLDriver::disableTextures(u32 fromStage)
{
    bool result=true;
    for (u32 i=fromStage; i<MaxSupportedTextures; ++i)
        result &= setActiveTexture(i, 0);
    return result;
}


//! creates a matrix in supplied GLfloat array to pass to OpenGL
inline void COpenGLDriver::getGLMatrix(GLfloat gl_matrix[16], const core::matrix4& m)
{
    memcpy(gl_matrix, m.pointer(), 16 * sizeof(f32));
}


//! creates a opengltexturematrix from a D3D style texture matrix
inline void COpenGLDriver::getGLTextureMatrix(GLfloat *o, const core::matrix4& m)
{
    o[0] = m[0];
    o[1] = m[1];
    o[2] = 0.f;
    o[3] = 0.f;

    o[4] = m[4];
    o[5] = m[5];
    o[6] = 0.f;
    o[7] = 0.f;

    o[8] = 0.f;
    o[9] = 0.f;
    o[10] = 1.f;
    o[11] = 0.f;

    o[12] = m[8];
    o[13] = m[9];
    o[14] = 0.f;
    o[15] = 1.f;
}


//! returns a device dependent texture from a software surface (IImage)
video::ITexture* COpenGLDriver::createDeviceDependentTexture(IImage* surface, const io::path& name)
{
    return new COpenGLTexture(surface, name, nullptr, this);
}


//! Sets a material. All 3d drawing functions draw geometry now using this material.
void COpenGLDriver::setMaterial(const SMaterial& material)
{
    Material = material;
    OverrideMaterial.apply(Material);

    for (s32 i = MaxTextureUnits-1; i>= 0; --i)
    {
        setActiveTexture(i, material.getTexture(i));
        if (material.getTexture(i))
            setTransform ((E_TRANSFORMATION_STATE) (ETS_TEXTURE_0 + i), Material.TextureLayer[i].getTextureMatrixConst());
    }
}


//! prints error if an error happened.
bool COpenGLDriver::testGLError()
{
#ifdef _DEBUG_OGL
    GLenum g = glGetError();
    switch (g)
    {
    case GL_NO_ERROR:
        return false;
    case GL_INVALID_ENUM:
        os::Printer::log("GL_INVALID_ENUM", ELL_ERROR); break;
    case GL_INVALID_VALUE:
        os::Printer::log("GL_INVALID_VALUE", ELL_ERROR); break;
    case GL_INVALID_OPERATION:
        os::Printer::log("GL_INVALID_OPERATION", ELL_ERROR); break;
    case GL_STACK_OVERFLOW:
        os::Printer::log("GL_STACK_OVERFLOW", ELL_ERROR); break;
    case GL_STACK_UNDERFLOW:
        os::Printer::log("GL_STACK_UNDERFLOW", ELL_ERROR); break;
    case GL_OUT_OF_MEMORY:
        os::Printer::log("GL_OUT_OF_MEMORY", ELL_ERROR); break;
    case GL_TABLE_TOO_LARGE:
        os::Printer::log("GL_TABLE_TOO_LARGE", ELL_ERROR); break;
#if defined(GL_EXT_framebuffer_object)
    case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
        os::Printer::log("GL_INVALID_FRAMEBUFFER_OPERATION", ELL_ERROR); break;
#endif
    };
    //const GLubyte * errorStr = gluErrorString(g);
    //os::Printer::log(irr::core::StrFormat("[GL Error]: %s \n %s \n", errorStr, "").c_str());
    assert(false);
    return true;
#else
    return false;
#endif
}


//! sets the needed renderstates
bool COpenGLDriver::setRenderStates3DMode()
{
    if (CurrentRenderMode != ERM_3D)
    {
        ResetRenderStates = true;
    }

    if (ResetRenderStates || LastMaterial != Material)
    {
        // unset old material

        //if (LastMaterial.MaterialType != Material.MaterialType &&
        //        static_cast<u32>(LastMaterial.MaterialType) < MaterialRenderers.size())
        //    MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();
        //
        //// set new material.
        //if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
        //    MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(
        //        Material, LastMaterial, ResetRenderStates, this);

        irr::video::GLSLGpuShader* _vkShader = static_cast<irr::video::GLSLGpuShader*>(GetActiveShader());

        if (_vkShader)
        {
            for (int i = 0; i < _vkShader->mBuffers.size(); ++i)
            {
                irr::video::SConstantBuffer* cbuffer = _vkShader->mBuffers[i];
                if (cbuffer->mCallBack)
                    cbuffer->mCallBack->OnSetMaterial(cbuffer, Material);
            }
        }

        setBasicRenderStates(Material, LastMaterial, ResetRenderStates);

        LastMaterial = Material;
        ResetRenderStates = false;
    }

    //if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
    //    MaterialRenderers[Material.MaterialType].Renderer->OnRender(this, video::EVT_STANDARD);

    CurrentRenderMode = ERM_3D;
    return true;
}


//! Get native wrap mode value
GLint COpenGLDriver::getTextureWrapMode(const u8 clamp)
{
    GLint mode=GL_REPEAT;
    switch (clamp)
    {
        case ETC_REPEAT:
            mode=GL_REPEAT;
            break;
        case ETC_CLAMP:
            mode=GL_CLAMP;
            break;
        case ETC_CLAMP_TO_EDGE:
#ifdef GL_VERSION_1_2
            if (Version>101)
                mode=GL_CLAMP_TO_EDGE;
            else
#endif
#ifdef GL_SGIS_texture_edge_clamp
            if (FeatureAvailable[IRR_SGIS_texture_edge_clamp])
                mode=GL_CLAMP_TO_EDGE_SGIS;
            else
#endif
                // fallback
                mode=GL_CLAMP;
            break;
        case ETC_CLAMP_TO_BORDER:
#ifdef GL_VERSION_1_3
            if (Version>102)
                mode=GL_CLAMP_TO_BORDER;
            else
#endif
#ifdef GL_ARB_texture_border_clamp
            if (FeatureAvailable[IRR_ARB_texture_border_clamp])
                mode=GL_CLAMP_TO_BORDER_ARB;
            else
#endif
#ifdef GL_SGIS_texture_border_clamp
            if (FeatureAvailable[IRR_SGIS_texture_border_clamp])
                mode=GL_CLAMP_TO_BORDER_SGIS;
            else
#endif
                // fallback
                mode=GL_CLAMP;
            break;
        case ETC_MIRROR:
#ifdef GL_VERSION_1_4
            if (Version>103)
                mode=GL_MIRRORED_REPEAT;
            else
#endif
#ifdef GL_ARB_texture_border_clamp
            if (FeatureAvailable[IRR_ARB_texture_mirrored_repeat])
                mode=GL_MIRRORED_REPEAT_ARB;
            else
#endif
#ifdef GL_IBM_texture_mirrored_repeat
            if (FeatureAvailable[IRR_IBM_texture_mirrored_repeat])
                mode=GL_MIRRORED_REPEAT_IBM;
            else
#endif
                mode=GL_REPEAT;
            break;
        case ETC_MIRROR_CLAMP:
#ifdef GL_EXT_texture_mirror_clamp
            if (FeatureAvailable[IRR_EXT_texture_mirror_clamp])
                mode=GL_MIRROR_CLAMP_EXT;
            else
#endif
#if defined(GL_ATI_texture_mirror_once)
            if (FeatureAvailable[IRR_ATI_texture_mirror_once])
                mode=GL_MIRROR_CLAMP_ATI;
            else
#endif
                mode=GL_CLAMP;
            break;
        case ETC_MIRROR_CLAMP_TO_EDGE:
#ifdef GL_EXT_texture_mirror_clamp
            if (FeatureAvailable[IRR_EXT_texture_mirror_clamp])
                mode=GL_MIRROR_CLAMP_TO_EDGE_EXT;
            else
#endif
#if defined(GL_ATI_texture_mirror_once)
            if (FeatureAvailable[IRR_ATI_texture_mirror_once])
                mode=GL_MIRROR_CLAMP_TO_EDGE_ATI;
            else
#endif
                mode=GL_CLAMP;
            break;
        case ETC_MIRROR_CLAMP_TO_BORDER:
#ifdef GL_EXT_texture_mirror_clamp
            if (FeatureAvailable[IRR_EXT_texture_mirror_clamp])
                mode=GL_MIRROR_CLAMP_TO_BORDER_EXT;
            else
#endif
                mode=GL_CLAMP;
            break;
    }
    return mode;
}

GLuint COpenGLDriver::getDepthFunction(E_COMPARISON_FUNC func) const
{
    switch (func)
    {
        case ECFN_LESSEQUAL:
            return GL_LEQUAL;
        case ECFN_EQUAL:
            return GL_EQUAL;
        case ECFN_LESS:
            return GL_LESS;
        case ECFN_NOTEQUAL:
            return GL_NOTEQUAL;
        case ECFN_GREATEREQUAL:
            return GL_GEQUAL;
        case ECFN_GREATER:
            return GL_GREATER;
        case ECFN_ALWAYS:
            return GL_ALWAYS;
        case ECFN_NEVER:
        default:
            return GL_NEVER;
    }
}

GLuint COpenGLDriver::getOGLStencilOp(E_STENCIL_OPERATION func) const
{
    switch (func)
    {
        case E_STENCIL_OPERATION::ESO_DECREMENT:
            return GL_DECR;
        case E_STENCIL_OPERATION::ESO_DECREMENT_WRAP:
            return GL_DECR_WRAP;
        case E_STENCIL_OPERATION::ESO_INCREMENT:
            return GL_INCR;
        case E_STENCIL_OPERATION::ESO_INCREMENT_WRAP:
            return GL_INCR_WRAP;
        case E_STENCIL_OPERATION::ESO_INVERT:
            return GL_INVERT;
        case E_STENCIL_OPERATION::ESO_KEEP:
            return GL_KEEP;
        case E_STENCIL_OPERATION::ESO_REPLACE:
            return GL_REPLACE;
        case E_STENCIL_OPERATION::ESO_ZERO:
        default:
            return GL_ZERO;
    }
}

GLuint COpenGLDriver::getOGLBlendOp(E_BLEND_OPERATION operation) const
{
    switch (operation)
    {
        case EBO_NONE:			return GL_FUNC_ADD;
        case EBO_ADD:			return GL_FUNC_ADD;
        case EBO_SUBTRACT:		return GL_FUNC_SUBTRACT;
        case EBO_REVSUBTRACT:	return GL_FUNC_REVERSE_SUBTRACT;
        case EBO_MIN:			return GL_MIN;
        case EBO_MAX:			return GL_MAX;
        case EBO_MIN_FACTOR:	return GL_MIN;
        case EBO_MAX_FACTOR:	return GL_MAX;
        case EBO_MIN_ALPHA:		return GL_ALPHA_MIN_SGIX;
        case EBO_MAX_ALPHA:		return GL_ALPHA_MAX_SGIX;
    }

    return GL_FUNC_ADD;
}

u32 COpenGLDriver::getIndexAmount(scene::E_PRIMITIVE_TYPE primType, u32 primitiveCount)
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

//! Can be called by an IMaterialRenderer to make its work easier.
void COpenGLDriver::setBasicRenderStates(const SMaterial& material, const SMaterial& lastmaterial, bool resetAllRenderStates)
{
    // Fully obsolete
    m_stateCache.setEnabled(OGLStateEntries::OGL_ALPHA_TEST, false);

    // Sampler
    for (u32 st = 0; st < MaxTextureUnits; ++st)
    {
        if (!CurrentTexture[st])
            continue;

        GLuint sampler = m_stateCache.getSampler(st);
        bool useMipMap = CurrentTexture[st] ? CurrentTexture[st]->hasMipMaps() : material.UseMipMaps;

        if (resetAllRenderStates || lastmaterial.TextureLayer[st].LODBias != material.TextureLayer[st].LODBias)
        {
            const float tmp = material.TextureLayer[st].LODBias * 0.125f;

            glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, 0.0f);
            glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, std::numeric_limits<float>::max());
            glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS_EXT, tmp);
        }

        if (resetAllRenderStates || lastmaterial.TextureLayer[st].TextureWrapU != material.TextureLayer[st].TextureWrapU)
        {
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, getTextureWrapMode(material.TextureLayer[st].TextureWrapU));
        }

        if (resetAllRenderStates || lastmaterial.TextureLayer[st].TextureWrapU != material.TextureLayer[st].TextureWrapU)
        {
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, getTextureWrapMode(material.TextureLayer[st].TextureWrapU));
        }

        if (resetAllRenderStates || lastmaterial.TextureLayer[st].TextureWrapV != material.TextureLayer[st].TextureWrapV)
        {
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, getTextureWrapMode(material.TextureLayer[st].TextureWrapV));
        }

        // Bilinear, trilinear, and anisotropic filter
        if (resetAllRenderStates ||
            lastmaterial.TextureLayer[st].BilinearFilter != material.TextureLayer[st].BilinearFilter ||
            lastmaterial.TextureLayer[st].TrilinearFilter != material.TextureLayer[st].TrilinearFilter ||
            lastmaterial.TextureLayer[st].AnisotropicFilter != material.TextureLayer[st].AnisotropicFilter ||
            bool(GetStateCache()->getUseMipMaps(st)) != useMipMap)
        {
            GetStateCache()->setUseMipMaps(st, useMipMap);

            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, (material.TextureLayer[st].BilinearFilter || material.TextureLayer[st].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);

            if (useMipMap)
            {
                glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER,
                                    material.TextureLayer[st].TrilinearFilter ? GL_LINEAR_MIPMAP_LINEAR :
                                    material.TextureLayer[st].BilinearFilter ? GL_LINEAR_MIPMAP_NEAREST :
                                    GL_NEAREST_MIPMAP_NEAREST);
            }
            else
            {
                glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, (material.TextureLayer[st].BilinearFilter || material.TextureLayer[st].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);
            }

#ifdef GL_EXT_texture_filter_anisotropic
            if (FeatureAvailable[IRR_EXT_texture_filter_anisotropic] &&
                (material.TextureLayer[st].AnisotropicFilter != lastmaterial.TextureLayer[st].AnisotropicFilter))
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                    material.TextureLayer[st].AnisotropicFilter > 1 ? core::min_(MaxAnisotropy, material.TextureLayer[st].AnisotropicFilter) : 1);
            }
#endif

            // OpenGL 4.6
            //glSamplerParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, material.TextureLayer[st].AnisotropicFilter>1 ? core::min_(MaxAnisotropy, material.TextureLayer[st].AnisotropicFilter) : 1);
        }
    }

    // shademode
    if (resetAllRenderStates || lastmaterial.GouraudShading != material.GouraudShading)
    {
        m_stateCache.setShadeModel(material.GouraudShading ? GL_SMOOTH : GL_FLAT);
    }

    // fillmode
    if (resetAllRenderStates || (lastmaterial.Wireframe != material.Wireframe) || (lastmaterial.PointCloud != material.PointCloud))
        m_stateCache.setPolygonMode(material.Wireframe ? GL_LINE : material.PointCloud? GL_POINT : GL_FILL);

    // zbuffer
    if (resetAllRenderStates || lastmaterial.ZBuffer != material.ZBuffer)
    {
        m_stateCache.setEnabled(OGLStateEntries::OGL_DEPTH_TEST, material.ZBuffer != ECFN_NEVER);
        glDepthFunc(getDepthFunction((E_COMPARISON_FUNC)material.ZBuffer));
    }

    // zwrite
    if (resetAllRenderStates || lastmaterial.ZWriteEnable != material.ZWriteEnable)
    {
        if (material.ZWriteEnable && (AllowZWriteOnTransparent || !material.isTransparent()))
        {
            m_stateCache.setDepthMask(GL_TRUE);
        }
        else
        {
            m_stateCache.setDepthMask(GL_FALSE);
        }
    }

    // Stencil
    if (resetAllRenderStates || lastmaterial.StencilTest != material.StencilTest ||
        (*(u32*)&lastmaterial.StencilFront) != (*(u32*)&material.StencilFront) ||
        (*(u32*)&lastmaterial.StencilBack) != (*(u32*)&material.StencilBack))
    {
        GetStateCache()->setEnabled(OGLStateEntries::OGL_STENCIL_TEST, material.StencilTest);

        glStencilOp(getOGLStencilOp((E_STENCIL_OPERATION)material.StencilFront.FailOp), getOGLStencilOp((E_STENCIL_OPERATION)material.StencilFront.DepthFailOp), getOGLStencilOp((E_STENCIL_OPERATION)material.StencilFront.PassOp));
        glStencilFunc(getDepthFunction((E_COMPARISON_FUNC)material.StencilFront.Comparsion), material.StencilFront.Reference, material.StencilFront.Mask);
        GetStateCache()->setStencilMask(material.StencilFront.WriteMask);
    }

    // back face culling
    if (resetAllRenderStates || (lastmaterial.FrontfaceCulling != material.FrontfaceCulling) || (lastmaterial.BackfaceCulling != material.BackfaceCulling))
    {
        if ((material.FrontfaceCulling) && (material.BackfaceCulling))
        {
            glCullFace(GL_FRONT_AND_BACK);
            m_stateCache.setEnabled(OGLStateEntries::OGL_CULL_FACE, true);
        }
        else if (material.BackfaceCulling)
        {
            glCullFace(GL_BACK);
            m_stateCache.setEnabled(OGLStateEntries::OGL_CULL_FACE, true);
        }
        else if (material.FrontfaceCulling)
        {
            glCullFace(GL_FRONT);
            m_stateCache.setEnabled(OGLStateEntries::OGL_CULL_FACE, true);
        }
        else
        {
            m_stateCache.setEnabled(OGLStateEntries::OGL_CULL_FACE, false);
        }
    }

    // Color Mask
    if (resetAllRenderStates || lastmaterial.ColorMask != material.ColorMask)
    {
        m_stateCache.setColourMask(
            (material.ColorMask & ECP_RED)?GL_TRUE:GL_FALSE,
            (material.ColorMask & ECP_GREEN)?GL_TRUE:GL_FALSE,
            (material.ColorMask & ECP_BLUE)?GL_TRUE:GL_FALSE,
            (material.ColorMask & ECP_ALPHA)?GL_TRUE:GL_FALSE);
    }

    if (resetAllRenderStates || lastmaterial.MaterialType != material.MaterialType || lastmaterial.BlendOperation != material.BlendOperation || lastmaterial.uMaterialTypeParam != material.uMaterialTypeParam)
    {
        bool blendEnabled = material.BlendOperation != E_BLEND_OPERATION::EBO_NONE;
        E_BLEND_FACTOR srcFact = E_BLEND_FACTOR::EBF_ONE, dstFact = E_BLEND_FACTOR::EBF_ZERO, srcFactAlpha = E_BLEND_FACTOR::EBF_ONE, dstFactAlpha = E_BLEND_FACTOR::EBF_ZERO;
        E_BLEND_OPERATION blendOp = E_BLEND_OPERATION::EBO_ADD, blendOpAlpha = E_BLEND_OPERATION::EBO_ADD;
        E_MODULATE_FUNC modulate = E_MODULATE_FUNC::EMFN_MODULATE_1X;
        u32 alphaSource = EAS_TEXTURE;

        switch (material.MaterialType)
        {
            case E_MATERIAL_TYPE::EMT_SOLID:
            case E_MATERIAL_TYPE::EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
            {
                blendEnabled = false;
                break;
            }
            //case E_MATERIAL_TYPE::EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
            //    pack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, srcFactAlpha, dstFactAlpha, blendOp, blendOpAlpha);
            //    break;
            case E_MATERIAL_TYPE::EMT_REFLECTION_2_LAYER:
            case E_MATERIAL_TYPE::EMT_TRANSPARENT_ALPHA_CHANNEL:
            {
                blendEnabled = true;
                blendOp = E_BLEND_OPERATION::EBO_ADD;
                srcFact = E_BLEND_FACTOR::EBF_SRC_ALPHA;
                dstFact = E_BLEND_FACTOR::EBF_ONE_MINUS_SRC_ALPHA;
                pack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, srcFactAlpha, dstFactAlpha, blendOp, blendOpAlpha);
                break;
            }
            case E_MATERIAL_TYPE::EMT_TRANSPARENT_ADD_COLOR:
            case E_MATERIAL_TYPE::EMT_TRANSPARENT_VERTEX_ALPHA:
            {
                blendEnabled = true;
                blendOp = E_BLEND_OPERATION::EBO_ADD;
                srcFact = E_BLEND_FACTOR::EBF_ONE;
                dstFact = E_BLEND_FACTOR::EBF_ONE_MINUS_SRC_COLOR;
                srcFactAlpha = E_BLEND_FACTOR::EBF_ZERO;
                dstFactAlpha = E_BLEND_FACTOR::EBF_ZERO;
                pack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, srcFactAlpha, dstFactAlpha, blendOp, blendOpAlpha);
                break;
            }
            default:
            {
                unpack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, material.uMaterialTypeParam, &srcFactAlpha, &dstFactAlpha, &blendOp, &blendOpAlpha);
                blendEnabled = blendOp != E_BLEND_OPERATION::EBO_NONE || blendOpAlpha != E_BLEND_OPERATION::EBO_NONE;
                break;
            }
        }

        if (!blendEnabled && blendOpAlpha == E_BLEND_OPERATION::EBO_NONE)
        {
            GetStateCache()->setEnabled(OGLStateEntries::OGL_BLEND, false);
            //GetStateCache()->setEnabled(OGLStateEntries::OGL_ALPHA_TEST, false);
        }
        else
        {
            GetStateCache()->setEnabled(OGLStateEntries::OGL_BLEND, blendEnabled);
            //GetStateCache()->setEnabled(OGLStateEntries::OGL_ALPHA_TEST, blendOpAlpha != E_BLEND_OPERATION::EBO_NONE);
            GetStateCache()->setBlendEquation(getOGLBlendOp((E_BLEND_OPERATION)blendOp), getOGLBlendOp((E_BLEND_OPERATION)blendOpAlpha));
            GetStateCache()->setBlendFunc(getGLBlend(srcFact), getGLBlend(dstFact), getGLBlend(srcFactAlpha), getGLBlend(dstFactAlpha));
        }
    }

    //if (queryFeature(EVDF_BLEND_OPERATIONS) && (resetAllRenderStates|| lastmaterial.BlendOperation != material.BlendOperation))
    //{
    //    m_stateCache.setEnabled(OGLStateEntries::OGL_BLEND, material.BlendOperation != EBO_NONE);
    //    m_stateCache.setBlendEquation(getOGLBlendOp((E_BLEND_OPERATION)material.BlendOperation));
    //}

    // Polygon Offset
    if (queryFeature(EVDF_POLYGON_OFFSET) && (resetAllRenderStates ||
        lastmaterial.PolygonOffsetDirection != material.PolygonOffsetDirection ||
        lastmaterial.PolygonOffsetFactor != material.PolygonOffsetFactor))
    {
        m_stateCache.setEnabled(lastmaterial.Wireframe ? OGLStateEntries::OGL_POLYGON_OFFSET_LINE : lastmaterial.PointCloud ? OGLStateEntries::OGL_POLYGON_OFFSET_POINT : OGLStateEntries::OGL_POLYGON_OFFSET_FILL, false);
        if (material.PolygonOffsetFactor)
            m_stateCache.setEnabled(material.Wireframe ? OGLStateEntries::OGL_POLYGON_OFFSET_LINE : material.PointCloud ? OGLStateEntries::OGL_POLYGON_OFFSET_POINT : OGLStateEntries::OGL_POLYGON_OFFSET_FILL, true);

        if (material.PolygonOffsetDirection==EPO_BACK)
            glPolygonOffset(1.0f, (GLfloat)material.PolygonOffsetFactor);
        else
            glPolygonOffset(-1.0f, (GLfloat)-material.PolygonOffsetFactor);
    }

    //// thickness
    //if (resetAllRenderStates || lastmaterial.Thickness != material.Thickness)
    //{
    //    if (AntiAlias)
    //    {
    //        // we don't use point smoothing
    //        glPointSize(core::clamp(static_cast<GLfloat>(material.Thickness), DimAliasedPoint[0], DimAliasedPoint[1]));
    //        glLineWidth(core::clamp(static_cast<GLfloat>(material.Thickness), DimSmoothedLine[0], DimSmoothedLine[1]));
    //    }
    //    else
    //    {
    //        glPointSize(core::clamp(static_cast<GLfloat>(material.Thickness), DimAliasedPoint[0], DimAliasedPoint[1]));
    //        glLineWidth(core::clamp(static_cast<GLfloat>(material.Thickness), DimAliasedLine[0], DimAliasedLine[1]));
    //    }
    //}

    // Anti aliasing
    if (resetAllRenderStates || lastmaterial.AntiAliasing != material.AntiAliasing)
    {
        if (FeatureAvailable[IRR_ARB_multisample])
        {
            m_stateCache.setEnabled(OGLStateEntries::OGL_SAMPLE_ALPHA_TO_COVERAGE, material.AntiAliasing & EAAM_ALPHA_TO_COVERAGE);
            m_stateCache.setEnabled(OGLStateEntries::OGL_MULTISAMPLE_ARB, (AntiAlias >= 2) && (material.AntiAliasing & (EAAM_SIMPLE | EAAM_QUALITY)));
        }
        if ((material.AntiAliasing & EAAM_LINE_SMOOTH) != (lastmaterial.AntiAliasing & EAAM_LINE_SMOOTH))
        {
            m_stateCache.setEnabled(OGLStateEntries::OGL_LINE_SMOOTH, material.AntiAliasing & EAAM_LINE_SMOOTH);
        }
        if ((material.AntiAliasing & EAAM_POINT_SMOOTH) != (lastmaterial.AntiAliasing & EAAM_POINT_SMOOTH))
        {
            m_stateCache.setEnabled(OGLStateEntries::OGL_POINT_SMOOTH, material.AntiAliasing & EAAM_POINT_SMOOTH);
        }
    }
}


//! Enable the 2d override material
void COpenGLDriver::enableMaterial2D(bool enable)
{
    if (!enable)
        CurrentRenderMode = E_RENDER_MODE::ERM_2D;
    CNullDriver::enableMaterial2D(enable);
}


//! sets the needed renderstates
void COpenGLDriver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
{
    if (CurrentRenderMode != ERM_2D || Transformation3DChanged)
    {
        // unset last 3d material
        if (CurrentRenderMode == ERM_3D)
        {
            //if (static_cast<u32>(LastMaterial.MaterialType) < MaterialRenderers.size())
            //    MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();
        }
        if (Transformation3DChanged)
        {
            // Set world to identity
            core::matrix4 m;
            setTransform(ETS_WORLD, m);

            // Set view to translate a little forward
            //m.setTranslation(core::vector3df(-0.5f, -0.5f, 0)ü);
            setTransform(ETS_VIEW, getTransform(ETS_VIEW_2D));

            // Adjust projection
            const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
            m.buildProjectionMatrixOrthoLH(f32(renderTargetSize.Width), f32(-(s32)(renderTargetSize.Height)), -1.0, 1.0);
            m.setTranslation(core::vector3df(-1, 1, 0));
            setTransform(ETS_PROJECTION, m);

            Transformation3DChanged = false;
        }
        if (!OverrideMaterial2DEnabled)
        {
            setBasicRenderStates(InitMaterial2D, LastMaterial, true);
            LastMaterial = InitMaterial2D;
        }
    }
    if (OverrideMaterial2DEnabled)
    {
        OverrideMaterial2D.Lighting=false;
        setBasicRenderStates(OverrideMaterial2D, LastMaterial, false);
        LastMaterial = OverrideMaterial2D;
    }

    // no alphaChannel without texture
    alphaChannel &= texture;

    if (alphaChannel || alpha)
    {
        LastMaterial.BlendOperation = E_BLEND_OPERATION::EBO_ADD;
        m_stateCache.setEnabled(OGLStateEntries::OGL_BLEND, true);
        m_stateCache.setBlendEquation(GL_FUNC_ADD_EXT);
        m_stateCache.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        LastMaterial.BlendOperation = E_BLEND_OPERATION::EBO_NONE;
        m_stateCache.setEnabled(OGLStateEntries::OGL_BLEND, false);
    }

    if (texture)
    {
        Material.setTexture(0, const_cast<video::ITexture*>(CurrentTexture[0]));
        setTransform(ETS_TEXTURE_0, core::IdentityMatrix);
        // Due to the transformation change, the previous line would call a reset each frame
        // but we can safely reset the variable as it was false before
        Transformation3DChanged=false;
    }

    CurrentRenderMode = ERM_2D;
}


//! \return Returns the name of the video driver.
const wchar_t* COpenGLDriver::getName() const
{
    return Name.c_str();
}


//! deletes all dynamic lights there are
void COpenGLDriver::deleteAllDynamicLights()
{
    //for (s32 i=0; i<MaxLights; ++i)
    //    glDisable(GL_LIGHT0 + i);
    //
    //RequestedLights.clear();

    CNullDriver::deleteAllDynamicLights();
}


//! adds a dynamic light
s32 COpenGLDriver::addDynamicLight(const SLight& light)
{
    CNullDriver::addDynamicLight(light);

    RequestedLights.push_back(RequestedLight(light));

    u32 newLightIndex = RequestedLights.size() - 1;

    // Try and assign a hardware light just now, but don't worry if I can't
    assignHardwareLight(newLightIndex);

    return (s32)newLightIndex;
}


void COpenGLDriver::assignHardwareLight(u32 lightIndex)
{
    //static core::matrix4 empty;
    //setTransform(ETS_WORLD, empty);
    //
    //s32 lidx = GL_LIGHT0 + lightIndex;
    //RequestedLights[lightIndex].HardwareLightIndex = lidx;
    //
    ////s32 lidx = GL_LIGHT0;
    ////for (lidx = GL_LIGHT0 ; lidx < GL_LIGHT0 + MaxLights; ++lidx)
    ////{
    ////	if(!glIsEnabled(lidx))
    ////	{
    ////		RequestedLights[lightIndex].HardwareLightIndex = lidx;
    ////		break;
    ////	}
    ////}
    //
    //if(lidx == GL_LIGHT0 + MaxLights) // There's no room for it just now
    //    return;
    //
    //GLfloat data[4];
    //const SLight & light = RequestedLights[lightIndex].LightData;
    //
    //switch (light.Type)
    //{
    //case video::ELT_SPOT:
    //    data[0] = light.Direction.X;
    //    data[1] = light.Direction.Y;
    //    data[2] = light.Direction.Z;
    //    data[3] = 0.0f;
    //    glLightfv(lidx, GL_SPOT_DIRECTION, data);
    //
    //    // set position
    //    data[0] = light.Position.X;
    //    data[1] = light.Position.Y;
    //    data[2] = light.Position.Z;
    //    data[3] = 1.0f; // 1.0f for positional light
    //    glLightfv(lidx, GL_POSITION, data);
    //
    //    glLightf(lidx, GL_SPOT_EXPONENT, light.Falloff);
    //    glLightf(lidx, GL_SPOT_CUTOFF, light.OuterCone);
    //break;
    //case video::ELT_POINT:
    //    // set position
    //    data[0] = light.Position.X;
    //    data[1] = light.Position.Y;
    //    data[2] = light.Position.Z;
    //    data[3] = 1.0f; // 1.0f for positional light
    //    glLightfv(lidx, GL_POSITION, data);
    //
    //    glLightf(lidx, GL_SPOT_EXPONENT, 0.0f);
    //    glLightf(lidx, GL_SPOT_CUTOFF, 180.0f);
    //break;
    //case video::ELT_DIRECTIONAL:
    //    // set direction
    //    data[0] = -light.Direction.X;
    //    data[1] = -light.Direction.Y;
    //    data[2] = -light.Direction.Z;
    //    data[3] = 0.0f; // 0.0f for directional light
    //    glLightfv(lidx, GL_POSITION, data);
    //
    //    glLightf(lidx, GL_SPOT_EXPONENT, 0.0f);
    //    glLightf(lidx, GL_SPOT_CUTOFF, 180.0f);
    //break;
    //default:
    //break;
    //}
    //
    //// set diffuse color
    //data[0] = light.DiffuseColor.r;
    //data[1] = light.DiffuseColor.g;
    //data[2] = light.DiffuseColor.b;
    //data[3] = light.DiffuseColor.a;
    //glLightfv(lidx, GL_DIFFUSE, data);
    //
    //// set specular color
    //data[0] = light.SpecularColor.r;
    //data[1] = light.SpecularColor.g;
    //data[2] = light.SpecularColor.b;
    //data[3] = light.SpecularColor.a;
    //glLightfv(lidx, GL_SPECULAR, data);
    //
    //// set ambient color
    //data[0] = light.AmbientColor.r;
    //data[1] = light.AmbientColor.g;
    //data[2] = light.AmbientColor.b;
    //data[3] = light.AmbientColor.a;
    //glLightfv(lidx, GL_AMBIENT, data);
    //
    //// 1.0f / (constant + linear * d + quadratic*(d*d);
    //
    //// set attenuation
    //glLightf(lidx, GL_CONSTANT_ATTENUATION, light.Attenuation.X);
    //glLightf(lidx, GL_LINEAR_ATTENUATION, light.Attenuation.Y);
    //glLightf(lidx, GL_QUADRATIC_ATTENUATION, light.Attenuation.Z);
    //
    //GetStateCache()->setEnabled(OGLStateEntries::Olidx, true);
}


//! Turns a dynamic light on or off
//! \param lightIndex: the index returned by addDynamicLight
//! \param turnOn: true to turn the light on, false to turn it off
void COpenGLDriver::turnLightOn(s32 lightIndex, bool turnOn)
{
    if(lightIndex < 0 || lightIndex >= (s32)RequestedLights.size())
        return;

    RequestedLight & requestedLight = RequestedLights[lightIndex];

    requestedLight.DesireToBeOn = turnOn;

    if(turnOn)
    {
        if(-1 == requestedLight.HardwareLightIndex)
            assignHardwareLight(lightIndex);
    }
    else
    {
        if(-1 != requestedLight.HardwareLightIndex)
        {
            // It's currently assigned, so free up the hardware light
            //glDisable(requestedLight.HardwareLightIndex);
            requestedLight.HardwareLightIndex = -1;

            // Now let the first light that's waiting on a free hardware light grab it
            for(u32 requested = 0; requested < RequestedLights.size(); ++requested)
                if(RequestedLights[requested].DesireToBeOn
                    &&
                    -1 == RequestedLights[requested].HardwareLightIndex)
                {
                    assignHardwareLight(requested);
                    break;
                }
        }
    }
}


//! returns the maximal amount of dynamic lights the device can handle
u32 COpenGLDriver::getMaximalDynamicLightAmount() const
{
    return MaxLights;
}


//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void COpenGLDriver::setAmbientLight(const SColorf& color)
{
    AmbientLight = color;

    //GLfloat data[4] = {color.r, color.g, color.b, color.a};
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, data);
}


// this code was sent in by Oliver Klems, thank you! (I modified the glViewport
// method just a bit.
void COpenGLDriver::setViewPort(const core::rect<s32>& area)
{
    if (area == ViewPort)
        return;
    core::rect<s32> vp = area;
    core::rect<s32> rendert(0,0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);
    vp.clipAgainst(rendert);

    if (vp.getHeight()>0 && vp.getWidth()>0)
    {
        glViewport(vp.UpperLeftCorner.X,
                getCurrentRenderTargetSize().Height - vp.UpperLeftCorner.Y - vp.getHeight(),
                vp.getWidth(), vp.getHeight());

        ViewPort = vp;
    }
}


//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
//! this: First, draw all geometry. Then use this method, to draw the shadow
//! volume. Next use IVideoDriver::drawStencilShadow() to visualize the shadow.
void COpenGLDriver::drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail, u32 debugDataVisible)
{
    if (!Params.Stencilbuffer)
        return;

    if (triangles.empty())
        return;

    SetShader(m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW]);

    setActiveTexture(0, nullptr);
    setActiveTexture(1, nullptr);
    setActiveTexture(2, nullptr);
    setActiveTexture(3, nullptr);
    setActiveTexture(4, nullptr);

    Material.setTexture(0, nullptr);
    Material.setTexture(1, nullptr);
    Material.setTexture(2, nullptr);
    Material.setTexture(3, nullptr);
    Material.setTexture(4, nullptr);

    Material.ColorMask = 0;
    Material.FrontfaceCulling = !zfail;
    Material.BackfaceCulling = zfail;
    Material.ZWriteEnable = false;
    Material.MaterialType = E_MATERIAL_TYPE::EMT_ONETEXTURE_BLEND;
    Material.BlendOperation = E_BLEND_OPERATION::EBO_ADD;
    Material.uMaterialTypeParam = pack_textureBlendFunc(video::EBF_ZERO, video::EBF_ONE, video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR, video::EBF_ZERO, video::EBF_ONE, E_BLEND_OPERATION::EBO_ADD, E_BLEND_OPERATION::EBO_ADD);
    Material.ZBuffer = E_COMPARISON_FUNC::ECFN_LESS;

    Material.StencilTest = true;
    Material.StencilFront.Mask = 0xFF;
    Material.StencilFront.WriteMask = 0xFF;
    Material.StencilFront.Reference = 0;

    Material.StencilFront.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilFront.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT;
    Material.StencilFront.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;

    Material.StencilBack.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT;
    Material.StencilBack.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;

    //RasterizerDesc.AntialiasedLineEnable = false;
    //RasterizerDesc.MultisampleEnable = true;
    //RasterizerDesc.ScissorEnable = false;

    // draw everything
    setRenderStates3DMode();

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), E_VERTEX_TYPE::EVT_SHADOW, nullptr, 0,
        scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT);

    Material.FrontfaceCulling = zfail;
    Material.BackfaceCulling = !zfail;

    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT;

    // draw everything
    setRenderStates3DMode();

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), E_VERTEX_TYPE::EVT_SHADOW, nullptr, 0,
        scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT);

    CurrentRenderMode = zfail ? ERM_SHADOW_VOLUME_ZFAIL : ERM_SHADOW_VOLUME_ZPASS;
}

//! Fills the stencil shadow with color. After the shadow volume has been drawn
//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
//! to draw the color of the shadow.
void COpenGLDriver::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge,
    video::SColor rightUpEdge, video::SColor leftDownEdge, video::SColor rightDownEdge)
{
    if (!Params.Stencilbuffer)
        return;

    Material.ColorMask = 0xF;
    SetShader(m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD]);

    S3DVertex vtx[4];
    vtx[0] = S3DVertex(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, leftUpEdge, 0.0f, 0.0f);
    vtx[1] = S3DVertex(1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, rightUpEdge, 0.0f, 1.0f);
    vtx[2] = S3DVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, leftDownEdge, 1.0f, 0.0f);
    vtx[3] = S3DVertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, rightDownEdge, 1.0f, 1.0f);

    s16 indices[6] = { 0,1,2,1,3,2 };

    setActiveTexture(0, 0);

    auto proj = getTransform(E_TRANSFORMATION_STATE::ETS_PROJECTION);
    auto view = getTransform(E_TRANSFORMATION_STATE::ETS_VIEW);

    setTransform(E_TRANSFORMATION_STATE::ETS_PROJECTION, core::IdentityMatrix);
    setTransform(E_TRANSFORMATION_STATE::ETS_VIEW, core::IdentityMatrix);
    setTransform(E_TRANSFORMATION_STATE::ETS_WORLD, core::IdentityMatrix);

    Material.FrontfaceCulling = false;
    Material.BackfaceCulling = false;

    Material.MaterialType = E_MATERIAL_TYPE::EMT_ONETEXTURE_BLEND;
    Material.BlendOperation = E_BLEND_OPERATION::EBO_ADD;
    Material.uMaterialTypeParam = pack_textureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA, video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR, video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA, E_BLEND_OPERATION::EBO_ADD, E_BLEND_OPERATION::EBO_ADD);
    Material.ZBuffer = E_COMPARISON_FUNC::ECFN_NEVER;

    Material.StencilTest = true;
    Material.StencilFront.Mask = 0xFF;
    Material.StencilFront.WriteMask = 0xFF;
    Material.StencilFront.Reference = 1;

    Material.StencilFront.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilFront.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilFront.Comparsion = E_COMPARISON_FUNC::ECFN_LESSEQUAL;

    Material.StencilBack.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.Comparsion = E_COMPARISON_FUNC::ECFN_LESSEQUAL;

    // draw everything
    setRenderStates3DMode();

    draw2D3DVertexPrimitiveList(vtx, 4, E_VERTEX_TYPE::EVT_STANDARD, indices, 6,
        scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT);

    setTransform(E_TRANSFORMATION_STATE::ETS_PROJECTION, proj);
    setTransform(E_TRANSFORMATION_STATE::ETS_VIEW, view);

    Material.StencilTest = false;
    Material.StencilFront.Mask = 0x0;
    Material.StencilFront.WriteMask = 0x0;
    Material.StencilFront.Reference = 0;

    if (clearStencilBuffer)
    {
        clearBuffers(false, false, true, SColor());
    }

    CurrentRenderMode = ERM_STENCIL_FILL;
}


//! Sets the fog mode.
void COpenGLDriver::setFog(SColor c, E_FOG_TYPE fogType, f32 start,
            f32 end, f32 density, bool pixelFog, bool rangeFog)
{
    CNullDriver::setFog(c, fogType, start, end, density, pixelFog, rangeFog);
}


//! Removes a texture from the texture cache and deletes it, freeing lot of memory.
void COpenGLDriver::removeTexture(ITexture* texture)
{
    if (!texture)
        return;

    CNullDriver::removeTexture(texture);
}


//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void COpenGLDriver::OnResize(const core::dimension2d<u32>& size)
{
    CNullDriver::OnResize(size);
    glViewport(0, 0, size.Width, size.Height);
    Transformation3DChanged = true;
}


//! Returns type of video driver
E_DRIVER_TYPE COpenGLDriver::getDriverType() const
{
    return EDT_OPENGL;
}


//! returns color format
ECOLOR_FORMAT COpenGLDriver::getColorFormat() const
{
    return ColorFormat;
}


//! Sets a vertex shader constant.
void COpenGLDriver::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
#ifdef GL_ARB_vertex_program
    for (s32 i=0; i<constantAmount; ++i)
        extGlProgramLocalParameter4fv(GL_VERTEX_PROGRAM_ARB, startRegister+i, &data[i*4]);
#endif
}

//! Sets a pixel shader constant.
void COpenGLDriver::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
#ifdef GL_ARB_fragment_program
    for (s32 i=0; i<constantAmount; ++i)
        extGlProgramLocalParameter4fv(GL_FRAGMENT_PROGRAM_ARB, startRegister+i, &data[i*4]);
#endif
}

//! Sets a constant for the vertex shader based on a name.
bool COpenGLDriver::setVertexShaderConstant(const c8* name, const f32* floats, int count)
{
    //pass this along, as in GLSL the same routine is used for both vertex and fragment shaders
    return setPixelShaderConstant(name, floats, count);
}

//! Bool interface for the above.
bool COpenGLDriver::setVertexShaderConstant(const c8* name, const bool* bools, int count)
{
    return setPixelShaderConstant(name, bools, count);
}

//! Int interface for the above.
bool COpenGLDriver::setVertexShaderConstant(const c8* name, const s32* ints, int count)
{
    return setPixelShaderConstant(name, ints, count);
}

//! Sets a constant for the pixel shader based on a name.
bool COpenGLDriver::setPixelShaderConstant(const c8* name, const f32* floats, int count)
{
    os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
    return false;
}

//! Bool interface for the above.
bool COpenGLDriver::setPixelShaderConstant(const c8* name, const bool* bools, int count)
{
    os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
    return false;
}

//! Int interface for the above.
bool COpenGLDriver::setPixelShaderConstant(const c8* name, const s32* ints, int count)
{
    os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
    return false;
}


//! Adds a new material renderer to the VideoDriver, using pixel and/or
//! vertex shaders to render geometry.
s32 COpenGLDriver::addShaderMaterial(const c8* vertexShaderProgram,
    const c8* pixelShaderProgram,
    IShaderConstantSetCallBack* callback,
    E_MATERIAL_TYPE baseMaterial, s32 userData)
{
    s32 nr = -1;
    COpenGLShaderMaterialRenderer* r = new COpenGLShaderMaterialRenderer(
        this, nr, vertexShaderProgram, pixelShaderProgram,
        callback, getMaterialRenderer(baseMaterial), userData);

    r->drop();
    return nr;
}


//! Adds a new material renderer to the VideoDriver, using GLSL to render geometry.
s32 COpenGLDriver::addHighLevelShaderMaterial(
    const c8* vertexShaderProgram,
    const c8* vertexShaderEntryPointName,
    E_VERTEX_SHADER_TYPE vsCompileTarget,
    const c8* pixelShaderProgram,
    const c8* pixelShaderEntryPointName,
    E_PIXEL_SHADER_TYPE psCompileTarget,
    const c8* geometryShaderProgram,
    const c8* geometryShaderEntryPointName,
    E_GEOMETRY_SHADER_TYPE gsCompileTarget,
    scene::E_PRIMITIVE_TYPE inType,
    scene::E_PRIMITIVE_TYPE outType,
    u32 verticesOut,
    IShaderConstantSetCallBack* callback,
    E_MATERIAL_TYPE baseMaterial,
    s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
{
    s32 nr = -1;

    #ifdef _IRR_COMPILE_WITH_CG_
    if (shadingLang == EGSL_CG)
    {
        COpenGLCgMaterialRenderer* r = new COpenGLCgMaterialRenderer(
            this, nr,
            vertexShaderProgram, vertexShaderEntryPointName, vsCompileTarget,
            pixelShaderProgram, pixelShaderEntryPointName, psCompileTarget,
            geometryShaderProgram, geometryShaderEntryPointName, gsCompileTarget,
            inType, outType, verticesOut,
            callback,getMaterialRenderer(baseMaterial), userData);

        r->drop();
    }
    else
    #endif
    {
        COpenGLSLMaterialRenderer* r = new COpenGLSLMaterialRenderer(
            this, nr,
            vertexShaderProgram, vertexShaderEntryPointName, vsCompileTarget,
            pixelShaderProgram, pixelShaderEntryPointName, psCompileTarget,
            geometryShaderProgram, geometryShaderEntryPointName, gsCompileTarget,
            inType, outType, verticesOut,
            callback,getMaterialRenderer(baseMaterial), userData);

        r->drop();
    }

    return nr;
}


//! Returns a pointer to the IVideoDriver interface. (Implementation for
//! IMaterialRendererServices)
IVideoDriver* COpenGLDriver::getVideoDriver()
{
    return this;
}


ITexture* COpenGLDriver::addRenderTargetTexture(const core::dimension2d<u32>& size,
                    const io::path& name,
                    const ECOLOR_FORMAT format)
{
    //disable mip-mapping
    bool generateMipLevels = getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
    setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);

    bool supportForFBO = (ColorAttachment > 0);

    core::dimension2du destSize(size);

    if (!supportForFBO)
    {
        destSize = core::dimension2d<u32>(core::min_(size.Width, ScreenSize.Width), core::min_(size.Height, ScreenSize.Height));
        destSize = destSize.getOptimalSize((size == size.getOptimalSize()), false, false);
    }

    COpenGLTexture* renderTargetTexture = new COpenGLTexture(name, destSize, format, this);
    addTexture(renderTargetTexture);
    renderTargetTexture->drop();

    //restore mip-mapping
    setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, generateMipLevels);

    return renderTargetTexture;
}


//! Returns the maximum amount of primitives (mostly vertices) which
//! the device is able to render with one drawIndexedTriangleList
//! call.
u32 COpenGLDriver::getMaximalPrimitiveCount() const
{
    return 0x7fffffff;
}

ITexture * COpenGLDriver::addTextureCubemap(const io::path & name, IImage * imagePosX, IImage * imageNegX, IImage * imagePosY, IImage * imageNegY, IImage * imagePosZ, IImage * imageNegZ)
{
    return nullptr;
}

ITexture * COpenGLDriver::addTextureArray(const io::path & name, irr::core::array<IImage*> images)
{
    return nullptr;
}

IRenderTarget* COpenGLDriver::addRenderTarget()
{
    COpenGLRenderTarget* renderTarget = new COpenGLRenderTarget(this);
    RenderTargets.insert(renderTarget);

    return renderTarget;
}

bool COpenGLDriver::setRenderTargetEx(IRenderTarget* target, u16 clearFlag, SColor clearColor /*= SColor(255, 0, 0, 0)*/,
    f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    // if simply disabling the MRT via array call
    //if (!target || target->getTexture().size() == 0)
    //    return setRenderTarget(target ? target->getTexture()[0] : 0, clearFlag, clearColor, clearDepth, clearStencil);

    if (target && target->getDriverType() != EDT_OPENGL)
    {
        os::Printer::log("Fatal Error: Tried to set a render target not owned by this driver.", ELL_ERROR);
        return false;
    }

    bool supportForFBO = (ColorAttachment > 0);

    core::dimension2d<u32> destRenderTargetSize(0, 0);

    if (target)
    {
        COpenGLRenderTarget* renderTarget = static_cast<COpenGLRenderTarget*>(target);

        if (supportForFBO)
        {
            GetStateCache()->setFBO(renderTarget->getBufferID());
            renderTarget->update();
        }

        destRenderTargetSize = renderTarget->getSize();

        GetStateCache()->setViewport(0, 0, destRenderTargetSize.Width, destRenderTargetSize.Height);
    }
    else
    {
        if (supportForFBO)
            GetStateCache()->setFBO(0);
        else
        {
            COpenGLRenderTarget* prevRenderTarget = static_cast<COpenGLRenderTarget*>(CurrentRenderTarget);
            COpenGLTexture* renderTargetTexture = static_cast<COpenGLTexture*>(prevRenderTarget->getTexture());

            if (renderTargetTexture)
            {
                const COpenGLTexture* prevTexture = static_cast<const COpenGLTexture*>(CurrentTexture[0]);

                m_stateCache.bindGLTexture(0, static_cast<const COpenGLTexture*>(renderTargetTexture)->getOpenGLTextureName(), false);

                const core::dimension2d<u32> size = renderTargetTexture->getSize();
                glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, size.Width, size.Height);

                m_stateCache.bindGLTexture(0, static_cast<const COpenGLTexture*>(prevTexture)->getOpenGLTextureName(), false);
            }
        }

        destRenderTargetSize = core::dimension2d<u32>(0, 0);

        GetStateCache()->setViewport(0, 0, ScreenSize.Width, ScreenSize.Height);
    }

    testGLError();

    if (CurrentRenderTargetSize != destRenderTargetSize)
    {
        CurrentRenderTargetSize = destRenderTargetSize;

        Transformation3DChanged = true;
    }

    RenderTargetTexture = nullptr;
    CurrentRenderTarget = target;
    CurrentTarget = ERT_MULTI_RENDER_TEXTURES;

    if (!supportForFBO)
    {
        clearFlag |= ECBF_COLOR;
        clearFlag |= ECBF_DEPTH;
        clearFlag |= ECBF_STENCIL;
    }

    clearBuffers(clearFlag & ECBF_COLOR, clearFlag & ECBF_DEPTH, clearFlag & ECBF_STENCIL, clearColor);

    testGLError();

    return true;
}

bool COpenGLDriver::setRenderTarget(ITexture* texture, u16 clearFlag /*= ECBF_COLOR | ECBF_DEPTH*/, SColor clearColor /*= SColor(255, 0, 0, 0)*/,
    f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    // check for right driver type

    if (texture && texture->getDriverType() != EDT_OPENGL)
    {
        os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
        return false;
    }

    #if defined(GL_EXT_framebuffer_object)
        if (CurrentTarget==ERT_MULTI_RENDER_TEXTURES)
        {
            for (u32 i=0; i<ColorAttachment; ++i)
            {
                extGlFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, 0, 0);
            }
        }
    #endif

        testGLError();

        // check if we should set the previous RT back
    if ((RenderTargetTexture != texture) ||
        (CurrentTarget == ERT_MULTI_RENDER_TEXTURES))
    {
        setActiveTexture(0, 0);
        ResetRenderStates = true;
        if (RenderTargetTexture != 0)
        {
            RenderTargetTexture->unbindRTT();
        }

        if (texture)
        {
            // we want to set a new target. so do this.
            glViewport(0, 0, texture->getSize().Width, texture->getSize().Height);
            RenderTargetTexture = static_cast<COpenGLTexture*>(texture);
            // calls glDrawBuffer as well
            RenderTargetTexture->bindRTT();
            CurrentRendertargetSize = texture->getSize();
            CurrentTarget = ERT_RENDER_TEXTURE;
        }
        else
        {
            glViewport(0, 0, ScreenSize.Width, ScreenSize.Height);
            RenderTargetTexture = 0;
            CurrentRendertargetSize = core::dimension2d<u32>(0, 0);
            CurrentTarget = ERT_FRAME_BUFFER;
            glDrawBuffer(Params.Doublebuffer ? GL_BACK_LEFT : GL_FRONT_LEFT);
        }

        testGLError();

        CurrentRenderTarget = nullptr;

        // we need to update the matrices due to the rendersize change.
        Transformation3DChanged = true;
    }

    testGLError();


    if (clearFlag & ECBF_DEPTH | ECBF_STENCIL)
        clearBuffers(false, clearFlag & ECBF_DEPTH, clearFlag & ECBF_STENCIL, clearColor);

    return true;
}


// returns the current size of the screen or rendertarget
const core::dimension2d<u32>& COpenGLDriver::getCurrentRenderTargetSize() const
{
    if (CurrentRendertargetSize.Width == 0)
        return ScreenSize;
    else
        return CurrentRendertargetSize;
}


//! Clears the ZBuffer.
void COpenGLDriver::clearZBuffer(f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    clearBuffers(false, true, false, 0x0);
}


//! Returns an image created from the last rendered frame.
IImage* COpenGLDriver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)
{
    if (target==video::ERT_MULTI_RENDER_TEXTURES || target==video::ERT_RENDER_TEXTURE || target==video::ERT_STEREO_BOTH_BUFFERS)
        return 0;

    // allows to read pixels in top-to-bottom order
#ifdef GL_MESA_pack_invert
    if (FeatureAvailable[IRR_MESA_pack_invert])
        glPixelStorei(GL_PACK_INVERT_MESA, GL_TRUE);
#endif

    if (format==video::ECF_UNKNOWN)
        format=getColorFormat();
    GLenum fmt;
    GLenum type;
    switch (format)
    {
    case ECF_A1R5G5B5:
        fmt = GL_BGRA;
        type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
        break;
    case ECF_R5G6B5:
        fmt = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case ECF_R8G8B8:
        fmt = GL_RGB;
        type = GL_UNSIGNED_BYTE;
        break;
    case ECF_A8R8G8B8:
        fmt = GL_BGRA;
        if (Version > 101)
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
        else
            type = GL_UNSIGNED_BYTE;
        break;
    case ECF_R16F:
        if (FeatureAvailable[IRR_ARB_texture_rg])
            fmt = GL_RED;
        else
            fmt = GL_LUMINANCE;
#ifdef GL_ARB_half_float_pixel
        if (FeatureAvailable[IRR_ARB_half_float_pixel])
            type = GL_HALF_FLOAT_ARB;
        else
#endif
        {
            type = GL_FLOAT;
            format = ECF_R32F;
        }
        break;
    case ECF_G16R16F:
#ifdef GL_ARB_texture_rg
        if (FeatureAvailable[IRR_ARB_texture_rg])
            fmt = GL_RG;
        else
#endif
            fmt = GL_LUMINANCE_ALPHA;
#ifdef GL_ARB_half_float_pixel
        if (FeatureAvailable[IRR_ARB_half_float_pixel])
            type = GL_HALF_FLOAT_ARB;
        else
#endif
        {
            type = GL_FLOAT;
            format = ECF_G32R32F;
        }
        break;
    case ECF_A16B16G16R16F:
        fmt = GL_BGRA;
#ifdef GL_ARB_half_float_pixel
        if (FeatureAvailable[IRR_ARB_half_float_pixel])
            type = GL_HALF_FLOAT_ARB;
        else
#endif
        {
            type = GL_FLOAT;
            format = ECF_A32B32G32R32F;
        }
        break;
    case ECF_R32F:
        if (FeatureAvailable[IRR_ARB_texture_rg])
            fmt = GL_RED;
        else
            fmt = GL_LUMINANCE;
        type = GL_FLOAT;
        break;
    case ECF_G32R32F:
#ifdef GL_ARB_texture_rg
        if (FeatureAvailable[IRR_ARB_texture_rg])
            fmt = GL_RG;
        else
#endif
            fmt = GL_LUMINANCE_ALPHA;
        type = GL_FLOAT;
        break;
    case ECF_A32B32G32R32F:
        fmt = GL_BGRA;
        type = GL_FLOAT;
        break;
    default:
        fmt = GL_BGRA;
        type = GL_UNSIGNED_BYTE;
        break;
    }
    IImage* newImage = createImage(format, ScreenSize);

    u8* pixels = 0;
    if (newImage)
        pixels = static_cast<u8*>(newImage->lock());
    if (pixels)
    {
        GLenum tgt=GL_FRONT;
        switch (target)
        {
        case video::ERT_FRAME_BUFFER:
            break;
        case video::ERT_STEREO_LEFT_BUFFER:
            tgt=GL_FRONT_LEFT;
            break;
        case video::ERT_STEREO_RIGHT_BUFFER:
            tgt=GL_FRONT_RIGHT;
            break;
        default:
            tgt=GL_AUX0+(target-video::ERT_AUX_BUFFER0);
            break;
        }
        glReadBuffer(tgt);
        glReadPixels(0, 0, ScreenSize.Width, ScreenSize.Height, fmt, type, pixels);
        testGLError();
        glReadBuffer(GL_BACK);
    }

#ifdef GL_MESA_pack_invert
    if (FeatureAvailable[IRR_MESA_pack_invert])
        glPixelStorei(GL_PACK_INVERT_MESA, GL_FALSE);
    else
#endif
    if (pixels)
    {
        // opengl images are horizontally flipped, so we have to fix that here.
        const s32 pitch=newImage->getPitch();
        u8* p2 = pixels + (ScreenSize.Height - 1) * pitch;
        u8* tmpBuffer = new u8[pitch];
        for (u32 i=0; i < ScreenSize.Height; i += 2)
        {
            memcpy(tmpBuffer, pixels, pitch);
//			for (u32 j=0; j<pitch; ++j)
//			{
//				pixels[j]=(u8)(p2[j]*255.f);
//			}
            memcpy(pixels, p2, pitch);
//			for (u32 j=0; j<pitch; ++j)
//			{
//				p2[j]=(u8)(tmpBuffer[j]*255.f);
//			}
            memcpy(p2, tmpBuffer, pitch);
            pixels += pitch;
            p2 -= pitch;
        }
        delete [] tmpBuffer;
    }

    if (newImage)
    {
        newImage->unlock();
        if (testGLError() || !pixels)
        {
            newImage->drop();
            return 0;
        }
    }
    return newImage;
}


//! get depth texture for the given render target texture
ITexture* COpenGLDriver::createDepthTexture(ITexture* texture, bool shared)
{
    if ((texture->getDriverType() != EDT_OPENGL) || (!texture->isRenderTarget()))
        return 0;
    COpenGLTexture* tex = static_cast<COpenGLTexture*>(texture);

    if (!tex->isFrameBufferObject())
        return 0;

    if (shared)
    {
        for (u32 i=0; i<DepthTextures.size(); ++i)
        {
            if (DepthTextures[i]->getSize()==texture->getSize())
            {
                DepthTextures[i]->grab();
                return DepthTextures[i];
            }
        }
        DepthTextures.push_back(new COpenGLFBODepthTexture(texture->getSize(), "depth1", this));
        return DepthTextures.getLast();
    }
    return (new COpenGLFBODepthTexture(texture->getSize(), "depth1", this));
}


void COpenGLDriver::removeDepthTexture(ITexture* texture)
{
    for (u32 i=0; i<DepthTextures.size(); ++i)
    {
        if (texture==DepthTextures[i])
        {
            DepthTextures.erase(i);
            return;
        }
    }
}


//! Set/unset a clipping plane.
bool COpenGLDriver::setClipPlane(u32 index, const core::plane3df& plane, bool enable)
{
    if (index >= MaxUserClipPlanes)
        return false;

    UserClipPlanes[index].Plane=plane;
    enableClipPlane(index, enable);
    return true;
}


void COpenGLDriver::uploadClipPlane(u32 index)
{
    // opengl needs an array of doubles for the plane equation
    GLdouble clip_plane[4];
    clip_plane[0] = UserClipPlanes[index].Plane.Normal.X;
    clip_plane[1] = UserClipPlanes[index].Plane.Normal.Y;
    clip_plane[2] = UserClipPlanes[index].Plane.Normal.Z;
    clip_plane[3] = UserClipPlanes[index].Plane.D;
    glClipPlane(GL_CLIP_PLANE0 + index, clip_plane);
}


//! Enable/disable a clipping plane.
void COpenGLDriver::enableClipPlane(u32 index, bool enable)
{
    if (index >= MaxUserClipPlanes)
        return;
    if (enable)
    {
        if (!UserClipPlanes[index].Enabled)
        {
            uploadClipPlane(index);
            GetStateCache()->setEnabled(OGLStateEntries(u8(OGLStateEntries::OGL_CLIP_PLANE0) + index), true);
        }
    }
    else
        GetStateCache()->setEnabled(OGLStateEntries(u8(OGLStateEntries::OGL_CLIP_PLANE0) + index), false);

    UserClipPlanes[index].Enabled=enable;
}


core::dimension2du COpenGLDriver::getMaxTextureSize() const
{
    return core::dimension2du(MaxTextureSize, MaxTextureSize);
}


//! Convert E_PRIMITIVE_TYPE to OpenGL equivalent
GLenum COpenGLDriver::primitiveTypeToGL(scene::E_PRIMITIVE_TYPE type) const
{
    switch (type)
    {
        case scene::EPT_POINTS:
            return GL_POINTS;
        case scene::EPT_LINE_STRIP:
            return GL_LINE_STRIP;
        case scene::EPT_LINE_LOOP:
            return GL_LINE_LOOP;
        case scene::EPT_LINES:
            return GL_LINES;
        case scene::EPT_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case scene::EPT_TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case scene::EPT_TRIANGLES:
            return GL_TRIANGLES;
        case scene::EPT_QUAD_STRIP:
            return GL_QUAD_STRIP;
        case scene::EPT_QUADS:
            return GL_QUADS;
        case scene::EPT_POLYGON:
            return GL_POLYGON;
        case scene::EPT_POINT_SPRITES:
#ifdef GL_ARB_point_sprite
            return GL_POINT_SPRITE_ARB;
#else
            return GL_POINTS;
#endif
    }
    return GL_TRIANGLES;
}


GLenum COpenGLDriver::getGLBlend(E_BLEND_FACTOR factor) const
{
    GLenum r = 0;
    switch (factor)
    {
        case EBF_ZERO:			r = GL_ZERO; break;
        case EBF_ONE:			r = GL_ONE; break;
        case EBF_DST_COLOR:		r = GL_DST_COLOR; break;
        case EBF_ONE_MINUS_DST_COLOR:	r = GL_ONE_MINUS_DST_COLOR; break;
        case EBF_SRC_COLOR:		r = GL_SRC_COLOR; break;
        case EBF_ONE_MINUS_SRC_COLOR:	r = GL_ONE_MINUS_SRC_COLOR; break;
        case EBF_SRC_ALPHA:		r = GL_SRC_ALPHA; break;
        case EBF_ONE_MINUS_SRC_ALPHA:	r = GL_ONE_MINUS_SRC_ALPHA; break;
        case EBF_DST_ALPHA:		r = GL_DST_ALPHA; break;
        case EBF_ONE_MINUS_DST_ALPHA:	r = GL_ONE_MINUS_DST_ALPHA; break;
        case EBF_SRC_ALPHA_SATURATE:	r = GL_SRC_ALPHA_SATURATE; break;
    }
    return r;
}

GLenum COpenGLDriver::getZBufferBits() const
{
    GLenum bits = 0;
    switch (Params.ZBufferBits)
    {
    case 16:
        bits = GL_DEPTH_COMPONENT16;
        break;
    case 24:
        bits = GL_DEPTH_COMPONENT24;
        break;
    case 32:
        bits = GL_DEPTH_COMPONENT32;
        break;
    default:
        bits = GL_DEPTH_COMPONENT;
        break;
    }
    return bits;
}

#ifdef _IRR_COMPILE_WITH_CG_
const CGcontext& COpenGLDriver::getCgContext()
{
    return CgContext;
}
#endif
#endif // _IRR_COMPILE_WITH_OPENGL_

namespace irr
{
    namespace video
    {
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_) || defined(_IRR_COMPILE_WITH_X11_DEVICE_) || defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params, io::IFileSystem* io, IContextManager* contextManager)
	{
#ifdef _IRR_COMPILE_WITH_OPENGL_
		COpenGLDriver* ogl = new COpenGLDriver(params, io, contextManager);

		if (!ogl->initDriver())
		{
			ogl->drop();
			ogl = 0;
		}

		return ogl;
#else
		return 0;
#endif
	}
#endif        

        // -----------------------------------
        // SDL VERSION
        // -----------------------------------
#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
        IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
            io::IFileSystem* io, CIrrDeviceSDL* device)
        {
#ifdef _IRR_COMPILE_WITH_OPENGL_
            return new COpenGLDriver(params, io, device);
#else
            return 0;
#endif //  _IRR_COMPILE_WITH_OPENGL_
        }
#endif // _IRR_COMPILE_WITH_SDL_DEVICE_
    }
}
