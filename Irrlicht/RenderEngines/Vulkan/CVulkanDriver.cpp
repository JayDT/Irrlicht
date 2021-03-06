#include "CVulkanDriver.h"
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
#   include "ContextManager/CWinPlatform.h"
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#   include "ContextManager/CLinuxPlatform.h"
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
#   include "ContextManager/CMacPlatform.h"
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
#   include "ContextManager/CAndroidPlatform.h"
#endif
#include "CVulkanVertexDeclaration.h"
#include "CVulkanHardwareBuffer.h"
#include "CVulkanTexture.h"
#include "CVulkanShader.h"
#include "CVulkanDevice.h"
#include "CVulkanQueue.h"
#include "CVulkanGpuPipelineState.h"
#include "CVulkanGpuParams.h"
#include "CVulkanSwapChain.h"
#include "CVulkanRenderTarget.h"
#include "ContextManager/CVulkanPlatform.h"

#include "standard/client/DataSource_Standard.h"
#include "buildin_data.h"

#include "os.h"
#include "glslang/Public/ShaderLang.h"

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <gli/gli.hpp>

#ifdef HAVE_CSYSTEM_EXTENSION
#include "System/Uri.h"
#include "System/Resource/ResourceManager.h"
#endif

using namespace irr;
using namespace irr::video;

namespace glslang
{
    bool InitProcess();
    bool DetachProcess();
}

extern core::matrix4 VK_UnitMatrix;
extern core::matrix4 VK_SphereMapMatrix;
extern "C" void VKLoadShaderCache(System::IO::IFileReader* file);

VKAPI_ATTR VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

std::string GetVendorStringByID(u16 vendorId)
{
    switch (vendorId)
    {
        case 0x1002: return "ATI Technologies Inc."; break;
        case 0x10DE: return "NVIDIA Corporation"; break;
        case 0x102B: return "Matrox Electronic Systems Ltd."; break;
        case 0x121A: return "3dfx Interactive Inc"; break;
        case 0x5333: return "S3 Graphics Co., Ltd."; break;
        case 0x8086: return "Intel Corporation"; break;
        default: return "Unknown VendorId: " + std::to_string(vendorId); break;
    }
}

irr::video::CVulkanDriver::CVulkanDriver(const SIrrlichtCreationParameters & params, io::IFileSystem * io)
    : CNullDriver(io, params.WindowSize)
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
    , DeviceType(EIDT_WIN32)
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
    , DeviceType(EIDT_X11)
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
    , DeviceType(EIDT_OSX)
#endif
    , mCreateParams(params)
    , mFrameID(0)
    , VendorID(0)
    , ColorFormat(ECF_A8R8G8B8)
    , MaxActiveLights(8)
    , MaxTextureUnits(MATERIAL_MAX_TEXTURES)
{
    memset(m_defaultShader, 0, sizeof(m_defaultShader));

    // create sphere map matrix
    VK_SphereMapMatrix = core::matrix4(0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);
}

irr::video::CVulkanDriver::~CVulkanDriver()
{
    SetShader(nullptr);

    RequestedLights.clear();

    deleteMaterialRenders();

    for (auto rtt : RenderTargets)
    {
        rtt->drop();
    }

    RenderTargets.clear();

    // I get a blue screen on my laptop, when I do not delete the
    // textures manually before releasing the dc. Oh how I love this.
    deleteAllTextures();
    removeAllOcclusionQueries();
    removeAllHardwareBuffers();

    mMainCommandBuffer->drop();
    mMainCommandBuffer = nullptr;

    if (mPlatform)
        delete mPlatform;

    while (!VertexDeclarations.empty())
    {
        static_cast<CVulkanVertexDeclaration*>(VertexDeclarations.begin()->second)->OnDeviceDestroy(this);
        VertexDeclarations.erase(VertexDeclarations.begin());
    }

    size_t lastSize = ResourceList.size();
    for (auto i = ResourceList.begin(); i != ResourceList.end();)
    {
        (*i)->OnDeviceDestroy(this);
        if (lastSize == ResourceList.size())
        {
            ++i;
            continue;
        }

        i = ResourceList.begin();
        lastSize = ResourceList.size();
    }

    while (!ShaderModuls.empty())
    {
        static_cast<CVulkanGLSLProgram*>(ShaderModuls.getLast())->drop();
        ShaderModuls.erase(ShaderModuls.size() - 1);
    }

    ShaderModuls.clear();

    vkDestroySampler(this->mPrimaryDevices->getLogical(), mDummySampler, VulkanDevice::gVulkanAllocator);

    if (blankTexture)
        blankTexture->drop();
    blankTexture = nullptr;

    for (auto stages : mPipelines)
    {
        for (auto pipeline : stages.second)
            pipeline.second->drop();
    }

    mPipelines.clear();

    for (s32 i = 0; i < _MAX_DEVICES; ++i)
    {
        if (!mDevices[i])
            continue;

        delete mDevices[i];
    }

#if VULKAN_DEBUG_MODE
    if (VulkanDispatcherExt.vkDestroyDebugReportCallbackEXT)
        VulkanDispatcherExt.vkDestroyDebugReportCallbackEXT(mInstance, mDebugCallback, VulkanDevice::gVulkanAllocator);
#endif


    if (mInstance)
        vkDestroyInstance(mInstance, NULL);
}

void irr::video::CVulkanDriver::ReleaseDriver()
{
}

void irr::video::CVulkanDriver::getSyncSemaphores(u32 deviceIdx, u32 syncMask, VulkanSemaphore ** semaphores, u32 & count)
{
    bool semaphoreRequestFailed = false;
    VulkanDevice* device = _getDevice(deviceIdx);

    u32 semaphoreIdx = 0;
    for (u32 i = 0; i < GQT_COUNT; i++)
    {
        GpuQueueType queueType = (GpuQueueType)i;

        u32 numQueues = device->getNumQueues(queueType);
        for (u32 j = 0; j < numQueues; j++)
        {
            VulkanQueue* queue = device->getQueue(queueType, j);
            VulkanCmdBuffer* lastCB = queue->getLastCommandBuffer();

            // Check if a buffer is currently executing on the queue
            if (lastCB == nullptr || !lastCB->isSubmitted())
                continue;

            // Check if we care about this specific queue
            u32 queueMask = device->getQueueMask(queueType, j);
            if ((syncMask & queueMask) == 0)
                continue;

            VulkanSemaphore* semaphore = lastCB->requestInterQueueSemaphore();
            if (semaphore == nullptr)
            {
                semaphoreRequestFailed = true;
                continue;
            }

            semaphores[semaphoreIdx++] = semaphore;
        }
    }

    count = semaphoreIdx;

    if (semaphoreRequestFailed)
    {
        os::Printer::log(("Failed to allocate semaphores for a command buffer sync. This means some of the dependency requests "
            "will not be fulfilled. This happened because a command buffer has too many dependant command "
            "buffers. The maximum allowed number is " + std::to_string(_MAX_VULKAN_CB_DEPENDENCIES) + " but can be "
            "increased by incrementing the value of _MAX_VULKAN_CB_DEPENDENCIES.").c_str());
    }
}

VulkanSwapChain * irr::video::CVulkanDriver::_getSwapChain()
{
    return mPlatform->GetSwapChain();
}

void irr::video::CVulkanDriver::initialize(void* param)
{
    //uint32_t count;
    //vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); //get number of extensions
    //std::vector<VkExtensionProperties> _extensions(count);
    //vkEnumerateInstanceExtensionProperties(nullptr, &count, _extensions.data()); //populate buffer
    //std::set<std::string> results;
    //for (auto& extension : _extensions) {
    //    results.insert(extension.extensionName);
    //}

    // Create instance
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Irrlicht App";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "IrrVulkan";
    appInfo.engineVersion = (0 << 24) | (4 << 16) | 0;
    appInfo.apiVersion = VK_API_VERSION_1_1;

#if VULKAN_DEBUG_MODE
    const char* layers[] =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };

    const char* extensions[] =
    {
        nullptr, /** Surface extension */
        nullptr, /** OS specific surface extension */
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        nullptr, /** Debug Marker **/
        //VK_KHR_DISPLAY_EXTENSION_NAME
    };

#if VULKAN_DEBUG_MARKER
    extensions[3] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

    uint32_t numLayers = sizeof(layers) / sizeof(layers[0]);
#else
    const char** layers = nullptr;
    const char* extensions[] =
    {
        nullptr, /** Surface extension */
        nullptr, /** OS specific surface extension */
        nullptr, /** Debug Marker **/
        //VK_KHR_DISPLAY_EXTENSION_NAME
    };

#if VULKAN_DEBUG_MARKER
    extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

    uint32_t numLayers = 0;
#endif

    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
    extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
    extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
    extensions[1] = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#else
    extensions[1] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#endif

    uint32_t numExtensions = sizeof(extensions) / sizeof(extensions[0]);

#if !VULKAN_DEBUG_MARKER
    --numExtensions;
#endif

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = numLayers;
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledExtensionCount = numExtensions;
    instanceInfo.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateInstance(&instanceInfo, VulkanDevice::gVulkanAllocator, &mInstance);
    if (result != VK_SUCCESS)
    {
        --instanceInfo.enabledExtensionCount;
        result = vkCreateInstance(&instanceInfo, VulkanDevice::gVulkanAllocator, &mInstance);
    }
    assert(result == VK_SUCCESS);

    vk::Instance _instance(mInstance);

#if VK_HEADER_VERSION <= 82
    VulkanDispatcherExt.init(_instance);
#else
    auto m_hVulkanDLL = LoadLibrary("vulkan-1.dll");
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(GetProcAddress(m_hVulkanDLL, "vkGetInstanceProcAddr"));

    VulkanDispatcherExt.init(_instance, vkGetInstanceProcAddr);
#endif

    // Set up debugging
#if VULKAN_DEBUG_MODE
    VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT debugInfo;
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugInfo.pNext = nullptr;
    debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMsgCallback;
    debugInfo.flags = debugFlags;
    debugInfo.pUserData = this;

    if (VulkanDispatcherExt.vkCreateDebugReportCallbackEXT)
    {
        result = VulkanDispatcherExt.vkCreateDebugReportCallbackEXT(mInstance, &debugInfo, VulkanDevice::gVulkanAllocator, &mDebugCallback);
        assert(result == VK_SUCCESS);
    }
#endif

    // Enumerate all devices
    result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, nullptr);
    assert(result == VK_SUCCESS);

    std::vector<VkPhysicalDevice> physicalDevices(mNumDevices);
    result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, physicalDevices.data());
    assert(result == VK_SUCCESS);

    mPrimaryDevices = nullptr;

    for (uint32_t i = 0; i < mNumDevices; i++)
        if (i < _MAX_DEVICES)
            mDevices[i] = new VulkanDevice(this, physicalDevices[i], i);

    // Find primary device
    // Note: MULTIGPU - Detect multiple similar devices here if supporting multi-GPU
    for (uint32_t i = 0; i < mNumDevices; i++)
    {
        bool isPrimary = mDevices[i]->getDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if (isPrimary)
        {
            mDevices[i]->setIsPrimary();
            mPrimaryDevices = mDevices[i];
            break;
        }
    }

    if (!mPrimaryDevices)
        mPrimaryDevices = mDevices[0];

    // Create main command buffer
    mMainCommandBuffer = new VulkanCommandBuffer(this, GQT_GRAPHICS, 0, 0, false);

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
    mPlatform = new CWinVulkanPlatform(this);
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
    //mPlatform = new CAndroidVulkanPlatform(this);
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
    //mPlatform = new CMacVulkanPlatform(this);
#else
    mPlatform = new CLinuxVulkanPlatform(this, (Display*)param);
#endif

    mPlatform->initialize();

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    //GetVendorStringByID(_getPrimaryDevice()->getDeviceProperties().vendorID);
    DriverAndFeatureName = L"Vulkan Adapter: ";
    DriverAndFeatureName += converter.from_bytes(_getPrimaryDevice()->getDeviceProperties().deviceName);

    //{
    //    uint32_t planeIndex = 0;
    //    uint32_t planeCount;
    //    VkResult result = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(mPrimaryDevices->getPhysical(), &planeCount, nullptr);
    //
    //    uint32_t displayCount;
    //    result = vkGetDisplayPlaneSupportedDisplaysKHR(mPrimaryDevices->getPhysical(), planeIndex, &displayCount, nullptr);
    //    assert(result == VK_SUCCESS);
    //    VkDisplayKHR* displayList = new VkDisplayKHR[displayCount];
    //    result = vkGetDisplayPlaneSupportedDisplaysKHR(mPrimaryDevices->getPhysical(), planeIndex, &displayCount, displayList);
    //    assert(result == VK_SUCCESS);
    //
    //    for (uint32_t d = 0; d < displayCount; ++d)
    //    {
    //        uint32_t numFormats = 100;
    //        result = vkGetDisplayModePropertiesKHR(mPrimaryDevices->getPhysical(), displayList[d], &numFormats, nullptr);
    //        assert(result == VK_SUCCESS);
    //        VkDisplayModePropertiesKHR* surfaceFormats = new VkDisplayModePropertiesKHR[numFormats];
    //        result = vkGetDisplayModePropertiesKHR(mPrimaryDevices->getPhysical(), displayList[d], &numFormats, surfaceFormats);
    //        assert(result == VK_SUCCESS);
    //
    //        m_displayModeList.set_used(numFormats);
    //        for (uint8 i = 0; i < numFormats; ++i)
    //        {
    //            VkDisplayPlaneCapabilitiesKHR planeCapability;
    //            result = vkGetDisplayPlaneCapabilitiesKHR(mPrimaryDevices->getPhysical(), surfaceFormats[i].displayMode, planeIndex, &planeCapability);
    //            assert(result == VK_SUCCESS);
    //
    //            m_displayModeList[i].Resolution.Height = surfaceFormats[i].parameters.visibleRegion.height;
    //            m_displayModeList[i].Resolution.Width = surfaceFormats[i].parameters.visibleRegion.width;
    //            m_displayModeList[i].Format = irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8; //VulkanUtility::getPixelFormat(surfaceFormats[i].);
    //            m_displayModeList[i].RefreshRate = surfaceFormats[i].parameters.refreshRate;
    //            m_displayModeList[i].param0 = 0;
    //            m_displayModeList[i].param1 = 0;
    //        }
    //
    //        delete[] surfaceFormats;
    //    }
    //
    //    delete[] displayList;
    //}
}

bool irr::video::CVulkanDriver::initDriver(void* param)
{
    CNullDriver::initDriver();

    initialize(param);
    glslang::InitProcess();
    ShInitialize();
    ShInitialize();  // also test reference counting of users
    ShFinalize();    // also test reference counting of users

    OnDebugMode = true;

    //ExposedData = ContextManager->getContext();

    // Only enable multisampling if needed
    disableFeature(EVDF_TEXTURE_MULTISAMPLING, true);

    // Set render targets
    setRenderTarget(0, true, true);

    // set fog mode
    //setFog(FogColor, FogType, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);
    setFog(FogColor, (E_FOG_TYPE)0, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

    ResetRenderStates = true;

    for (auto const& vertDecl : VertexDeclarations)
        static_cast<irr::video::CVulkanVertexDeclaration*>(vertDecl.second)->getVertexDeclaration();

    // create materials
    createMaterialRenderers();

    // clear textures
    for (u16 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
        setActiveTexture(i, 0);

    u8 dummydata[16];

    // Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
    mDummyStorageBuffer = static_cast<CVulkanHardwareBuffer*>(createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE, 16));
    mDummyStorageBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE, dummydata, 16);
    mDummyStorageBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_SHADER_RESOURCE, E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE, dummydata, 16);
    //mDummyUniformBuffer = static_cast<CVulkanHardwareBuffer*>(createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_SHADER_RESOURCE, E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE, 16));

    blankImage = createImage(ECF_A1R5G5B5, getCurrentRenderTargetSize());
    // Size.Height * Pitch
    memset(blankImage->lock(), 0, (blankImage->getPitch() * blankImage->getDimension().Height));
    blankImage->unlock();

    // ToDo: per device
    VkSamplerCreateInfo samplerCI = {};
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    VkResult result = vkCreateSampler(_getPrimaryDevice()->getLogical(), &samplerCI, VulkanDevice::gVulkanAllocator, &mDummySampler);
    assert(result == VK_SUCCESS);

    if (blankTexture)
        blankTexture->drop();
    blankTexture = static_cast<CVulkanTexture*>(createDeviceDependentTexture(blankImage, "internal_null_texture"));
    blankImage->drop();
    blankImage = nullptr;

    System::IO::StandardDataSource fileMgr;

    {
        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefault.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefault.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefault.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefault.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD] = static_cast<CVulkanGLSLProgram*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    /// ToDo: hlsl compile
    //{
    //    video::IShaderDataBuffer* bufferHandler = new irr::video::VulkanShaderGenericValuesBuffer(video::IShaderDataBuffer::EUT_PER_FRAME_PER_MESH);
    //
    //    ShaderInitializerEntry shaderCI;
    //
    //    auto resource = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Direct3D11/HLSL/Default/d3d11.hlsl");
    //    shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main", "vs_4_0")->Buffers.push_back(bufferHandler);
    //    shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "ps_main", "ps_4_0");
    //    m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD] = static_cast<CVulkanGLSLProgram*>(createShader(&shaderCI));
    //
    //    delete resource;
    //}

    {
        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefaultSH.vert");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefaultSH.vert", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW] = static_cast<CVulkanGLSLProgram*>(createShader(&shaderCI));

        delete vertShader;
        //delete fragShader;
    }

    {
        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefaultT2.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefault.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefaultT2.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefault.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS] = static_cast<CVulkanGLSLProgram*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    {
        ShaderInitializerEntry shaderCI;

#ifndef HAVE_CSYSTEM_EXTENSION
        auto vertShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefaultSK.vert");
        auto fragShader = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Vulkan/GLSL/VkDefault.frag");
#else
        auto vertShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefaultSK.vert", true))->ToMemoryStreamReader();
        auto fragShader = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/CVulkan;/GLSL/VkDefault.frag", true))->ToMemoryStreamReader();
#endif

        shaderCI.AddShaderStage(vertShader, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", nullptr);
        shaderCI.AddShaderStage(fragShader, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", nullptr);
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING] = static_cast<CVulkanGLSLProgram*>(createShader(&shaderCI));

        delete vertShader;
        delete fragShader;
    }

    SetShader(m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD]);

    return true;
}

bool irr::video::CVulkanDriver::_getErrorsFromQueue() const
{
    return false;
}

bool irr::video::CVulkanDriver::beginScene(bool backBuffer, bool zBuffer, SColor color, const SExposedVideoData & videoData, core::rect<s32>* sourceRect)
{
    ++mFrameID;

    CNullDriver::beginScene(backBuffer, zBuffer, color, videoData, sourceRect);

    mPlatform->acquireBackBuffer();

    mMainCommandBuffer->getInternal()->setSwapChain(_getSwapChain());
    mMainCommandBuffer->getInternal()->setRenderTarget(nullptr, 0, RenderSurfaceMaskBits::RT_ALL, true);
    mMainCommandBuffer->getInternal()->clearViewport(FrameBufferType::FBT_COLOR | FrameBufferType::FBT_DEPTH | FrameBufferType::FBT_STENCIL, color, 1.0f, 0, 0xFF);

    mMainCommandBuffer->getInternal()->beginRenderPass();
    return true;
}

bool irr::video::CVulkanDriver::endScene()
{
    CNullDriver::endScene();

    if (!mMainCommandBuffer->getInternal()->isInRenderPass())
        mMainCommandBuffer->getInternal()->beginRenderPass();
    mMainCommandBuffer->getInternal()->endRenderPass();

    _getPrimaryDevice()->flushTransferBuffers();
    mMainCommandBuffer->submit(0xFFFFFFFF);
    mPlatform->swapBuffers();

    return true;
}

void irr::video::CVulkanDriver::beginInstrumentEvent(const wchar_t* wlabel, const char* label, SColor color)
{
    VulkanUtility::beginRegion(*mMainCommandBuffer->getInternal(), label, color);
}

void irr::video::CVulkanDriver::endInstrumentEvent()
{
    VulkanUtility::endRegion(*mMainCommandBuffer->getInternal());
}

bool irr::video::CVulkanDriver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
{
    return true;
}

void convertProjectionMatrix(const core::matrix4& matrix, core::matrix4& dest)
{
    dest = matrix;

    // Flip Y axis
    dest(1,1) = -dest(1, 1);

    // Convert depth range from [-1,1] to [0,1]
    dest(2, 0) = dest(2, 0) * 0.5f;
    dest(2, 1) = dest(2, 1) * 0.5f;
    dest(2, 2) = dest(2, 2) * 0.5f;
    dest(3, 2) = dest(3, 2) * 0.5f;


    //dest(2, 0) =  (dest(2, 0) + dest(3, 0)) / 2;
    //dest(2, 1) =  (dest(2, 1) + dest(3, 1)) / 2;
    //dest(2, 2) =  (dest(2, 2) + dest(3, 2)) / 2;
    //dest(2, 3) =  (dest(2, 3) + dest(3, 3)) / 2;
}

void irr::video::CVulkanDriver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 & mat)
{
    Transformation3DChanged = state == ETS_PROJECTION || state == ETS_VIEW || state == ETS_WORLD;

    if (state == ETS_PROJECTION)
        convertProjectionMatrix(mat, Matrices[state]);
    else
        Matrices[state] = mat;

    if (state == ETS_PROJECTION || state == ETS_VIEW)
        Matrices[ETS_PROJECTION_VIEW] = Matrices[ETS_PROJECTION] * Matrices[ETS_VIEW];
}

void irr::video::CVulkanDriver::setMaterial(const SMaterial & material)
{
    Material = material;
    //OverrideMaterial.apply(Material);

    // set textures
    for (u16 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
    {
        setActiveTexture(i, Material.getTexture(i));
        setTransform((E_TRANSFORMATION_STATE)(ETS_TEXTURE_0 + i), material.TextureLayer[i].getTextureMatrixConst());
    }
}

ITexture * irr::video::CVulkanDriver::addTextureCubemap(const io::path & name, IImage * imagePosX, IImage * imageNegX, IImage * imagePosY, IImage * imageNegY, IImage * imagePosZ, IImage * imageNegZ)
{
    return nullptr;
}

ITexture * irr::video::CVulkanDriver::addTextureArray(const io::path & name, irr::core::array<IImage*> images)
{
    return nullptr;
}

IRenderTarget* irr::video::CVulkanDriver::addRenderTarget()
{
    CVulkanRenderTarget* renderTarget = new CVulkanRenderTarget(this);
    RenderTargets.insert(renderTarget);

    return renderTarget;
}

bool irr::video::CVulkanDriver::setRenderTargetEx(IRenderTarget* target, u16 clearFlag, SColor clearColor /*= SColor(255, 0, 0, 0)*/,
                                    f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/, core::array<core::recti>* scissors)
{
    u32 loadMask = 0;
    if (~clearFlag & ECBF_COLOR)
        loadMask |= RenderSurfaceMaskBits::RT_ALL;
    if (~clearFlag & ECBF_DEPTH)
        loadMask |= RenderSurfaceMaskBits::RT_DEPTH;
    if (~clearFlag & ECBF_STENCIL)
        loadMask |= RenderSurfaceMaskBits::RT_STENCIL;

    mMainCommandBuffer->getInternal()->setRenderTarget(target, 0, RenderSurfaceMaskBits(loadMask), true);

    u16 _vkClearFlags = 0;
    if (clearFlag & ECBF_COLOR)
        _vkClearFlags |= FrameBufferType::FBT_COLOR;
    if (clearFlag & ECBF_DEPTH)
        _vkClearFlags |= FrameBufferType::FBT_DEPTH;
    if (clearFlag & ECBF_STENCIL)
        _vkClearFlags |= FrameBufferType::FBT_STENCIL;

    mMainCommandBuffer->getInternal()->clearViewport(_vkClearFlags, clearColor, clearDepth, clearStencil, 0xFF);
    return true;
}

bool irr::video::CVulkanDriver::setRenderTarget(ITexture* texture, u16 clearFlag /*= ECBF_COLOR | ECBF_DEPTH*/, SColor clearColor /*= SColor(255, 0, 0, 0)*/,
                                    f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    return false;
}

void irr::video::CVulkanDriver::setViewPort(const core::rect<s32>& area)
{
    mMainCommandBuffer->getInternal()->setViewport(area);
}

void irr::video::CVulkanDriver::setScissorRect(const core::rect<s32>& rect)
{
    mMainCommandBuffer->getInternal()->setScissorRect(rect);
}

const core::rect<s32>& irr::video::CVulkanDriver::getViewPort() const
{
    return ViewPort;
}

bool irr::video::CVulkanDriver::updateVertexHardwareBuffer(CVulkanHardwareBuffer * HWBuffer, E_HARDWARE_BUFFER_TYPE Type)
{
    if (!HWBuffer)
        return false;

    E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;

    scene::IMeshBuffer* mb = HWBuffer->GetBuffer();

    u32 offset = 0;
    u32 bufSize = 0;
    u32 typeMask = 0;
    // Note: Revert because bugged after update buffers
    //if (mb->getHardwareMappingHint_Vertex() == scene::EHM_STATIC)
    //{
    //    const E_VERTEX_TYPE vType = mb->getVertexType();
    //    const void* vertices = mb->getVertices();
    //    const u32 vertexCount = mb->getVertexCount();
    //    const u32 vertexSize = mb->GetVertexStride() ? mb->GetVertexStride() : getVertexPitchFromType(vType);
    //    bufSize += vertexSize * vertexCount;
    //    typeMask |= 1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX);
    //}
    //if (mb->getHardwareMappingHint_Index() == scene::EHM_STATIC)
    //{
    //    const u16* indices = mb->getIndices();
    //    const u32 indexCount = mb->getIndexCount();
    //    const u32 indexStride = video::getIndexSize(mb->getIndexType());
    //
    //    if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_INDEX)
    //        offset = bufSize;
    //    bufSize += indexStride * indexCount;
    //    typeMask |= 1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX);
    //}
    //if (mb->getHardwareMappingHint_Instance() == scene::EHM_STATIC)
    //{
    //    IShaderDataBufferElement* variable = HWBuffer->GetInstanceBuffer()->m_bufferDataArray[0];
    //
    //    if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM)
    //        offset = bufSize;
    //    bufSize += variable->getValueSizeOf() * std::max(1u, variable->getShaderValueCount());
    //    typeMask |= 1 << u32(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM);
    //}

    if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX)
    {
        const E_VERTEX_TYPE vType = mb->getVertexType();
        const void* vertices = mb->getVertices();
        const u32 vertexCount = mb->getVertexCount();
        const u32 vertexSize = mb->GetVertexStride() ? mb->GetVertexStride() : getVertexPitchFromType(vType);
        const u32 vertexBufSize = vertexSize * vertexCount;

        //if (mb->getHardwareMappingHint_Vertex() == scene::EHM_STATIC)
        //{
        //    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE;
        //
        //    HWBuffer->UpdateBuffer(Type, MemoryAccess, vertices, bufSize, offset, vertexBufSize, typeMask);
        //}
        //else
        {
            if (mb->getHardwareMappingHint_Vertex() == scene::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (mb->getHardwareMappingHint_Instance() == scene::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM;

            HWBuffer->UpdateBuffer(Type, MemoryAccess, vertices, vertexBufSize, 0, vertexBufSize);
        }

        HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, mb->getChangedID_Vertex());
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM)
    {
        irr::scene::IStreamBuffer * streamBuffer = mb->getStreamBuffer();
        const void* vertices = streamBuffer->getData();
        const u32 vertexCount = streamBuffer->size();
        const u32 vertexSize = streamBuffer->stride();
        const u32 vertexBufSize = vertexSize * vertexCount;

        //if (mb->getHardwareMappingHint_Instance() == scene::EHM_STATIC)
        //{
        //    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE;
        //
        //    HWBuffer->UpdateBuffer(Type, MemoryAccess, vertices, bufSize, offset, vertexBufSize, typeMask);
        //}
        //else
        {
            if (mb->getHardwareMappingHint_Instance() == scene::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (mb->getHardwareMappingHint_Instance() == scene::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM;

            HWBuffer->UpdateBuffer(Type, MemoryAccess, vertices, vertexBufSize, 0, vertexBufSize);
        }

        HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM, streamBuffer->getChangedID());
    }
    else if (Type == E_HARDWARE_BUFFER_TYPE::EHBT_INDEX)
    {
        const u16* indices = mb->getIndices();
        const u32 indexCount = mb->getIndexCount();
        const u32 indexStride = video::getIndexSize(mb->getIndexType());
        const u32 indexBufSize = indexStride * indexCount;

        //if (mb->getHardwareMappingHint_Vertex() == scene::EHM_STATIC)
        //{
        //    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE;
        //
        //    HWBuffer->UpdateBuffer(Type, MemoryAccess, indices, bufSize, offset, indexBufSize, typeMask);
        //}
        //else
        {
            if (mb->getHardwareMappingHint_Instance() == scene::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (mb->getHardwareMappingHint_Instance() == scene::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM;

            HWBuffer->UpdateBuffer(Type, MemoryAccess, indices, indexBufSize, 0, indexBufSize);
        }

        HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, mb->getChangedID_Index());
    }
    return true;
}

bool irr::video::CVulkanDriver::updateIndexHardwareBuffer(CVulkanHardwareBuffer * HWBuffer)
{
    //if (!HWBuffer)
        return false;

    //const scene::IMeshBuffer* mb = HWBuffer->GetBuffer();
    //const u16* indices = mb->getIndices();
    //const u32 indexCount = mb->getIndexCount();
    //const u32 indexStride = video::getIndexSize(mb->getIndexType());
    //const u32 bufSize = indexStride * indexCount;
    //
    //E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
    //if (HWBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_DYNAMIC)
    //    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
    //
    //HWBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, MemoryAccess, indices, bufSize);
    //return true;
}

bool irr::video::CVulkanDriver::updateHardwareBuffer(IHardwareBuffer * HWBuffer)
{
    if (!HWBuffer)
        return false;

    if (HWBuffer->GetBuffer()->getHardwareMappingHint_Vertex() == scene::EHM_NEVER || HWBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_NEVER)
        HWBuffer->GetBuffer()->setHardwareMappingHint(scene::EHM_DYNAMIC);

    {
        if (HWBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX) != HWBuffer->GetBuffer()->getChangedID_Vertex())
        {
            if (!updateVertexHardwareBuffer(static_cast<CVulkanHardwareBuffer*>(HWBuffer), E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX))
                return false;

            //HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, HWBuffer->GetBuffer()->getChangedID_Vertex());
        }

        if (HWBuffer->GetBuffer()->getStreamBuffer() && HWBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM) != HWBuffer->GetBuffer()->getStreamBuffer()->getChangedID())
        {
            if (!updateVertexHardwareBuffer(static_cast<CVulkanHardwareBuffer*>(HWBuffer), E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM))
                return false;

            //HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM, HWBuffer->GetInstanceBuffer()->getChangedID());
        }
    }

    {
        if (HWBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX) != HWBuffer->GetBuffer()->getChangedID_Index())
        {
            if (!updateVertexHardwareBuffer(static_cast<CVulkanHardwareBuffer*>(HWBuffer), E_HARDWARE_BUFFER_TYPE::EHBT_INDEX))
                return false;

            //HWBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, HWBuffer->GetBuffer()->getChangedID_Index());
        }
    }

    return true;
}

IHardwareBuffer * irr::video::CVulkanDriver::createHardwareBuffer(const scene::IMeshBuffer * mb)
{
    if (!mb)
        return 0;

    CVulkanHardwareBuffer *hwBuffer = new CVulkanHardwareBuffer(this, const_cast<scene::IMeshBuffer*>(mb),
        mb->getIndexType() == video::EIT_16BIT ? (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_16_BITS : (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_32_BITS, mb->getVertexType());

    CVulkanGLSLProgram* vkShader = mb->GetGPUProgram() ? static_cast<CVulkanGLSLProgram*>(mb->GetGPUProgram()) : m_defaultShader[mb->getVertexType()];

    hwBuffer->SetCommandBuffer(mMainCommandBuffer);

    if (!mb->GetVertexDeclaration())
        mb->SetVertexDeclaration(this->GetVertexDeclaration(E_VERTEX_TYPE::EVT_STANDARD));
    assert(mb->GetVertexDeclaration());

    //if (mb->GetShaderConstantBuffers())
    //{
    //    for (u32 ib = 0; ib != mb->GetShaderConstantBuffers()->size(); ++ib)
    //    {
    //        const auto& cbuffer = (*mb->GetShaderConstantBuffers())[ib];
    //        if (!cbuffer)
    //            continue;
    //
    //        CShaderBuffer* shaderBuffer = static_cast<CShaderBuffer*>(cbuffer);
    //
    //        for (u32 i = 0; i < shaderBuffer->ConstantBuffers.size(); ++i)
    //        {
    //            shaderBuffer->ConstantBuffers[i] = createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC, shaderBuffer->mBufferSize);
    //        }
    //    }
    //}

    VulkanGraphicsPipelineState*& pipeline = mPipelines[vkShader][static_cast<CVulkanVertexDeclaration*>(mb->GetVertexDeclaration())];
    if (!pipeline)
    {
        pipeline = new VulkanGraphicsPipelineState(mb->getMaterial(), vkShader, static_cast<CVulkanVertexDeclaration*>(mb->GetVertexDeclaration()), GpuDeviceFlags::GDF_PRIMARY);
        pipeline->initialize();
    }

    if (mb->GetSubBufferCount())
    {
        for (u32 is = 0; is != mb->GetSubBufferCount(); ++is)
        {
            const_cast<scene::IMeshBuffer *>(mb)->SetActiveSubBuffer(is);

            hwBuffer->SetPipeline(is, pipeline);

            hwBuffer->SetGpuParams(is, vkShader->GetDefaultGpuParams());

            if (!updateHardwareBuffer(hwBuffer))
            {
                deleteHardwareBuffer(hwBuffer);
                return 0;
            }
        }

        const_cast<scene::IMeshBuffer *>(mb)->SetActiveSubBuffer(0);
    }
    else
    {
        hwBuffer->SetPipeline(0, pipeline);

        hwBuffer->SetGpuParams(0, vkShader->GetDefaultGpuParams());

        if (!updateHardwareBuffer(hwBuffer))
        {
            deleteHardwareBuffer(hwBuffer);
            return 0;
        }
    }

    hwBuffer->Initialize();
    return hwBuffer;
}

void irr::video::CVulkanDriver::deleteHardwareBuffer(IHardwareBuffer * HWBuffer)
{
    if (!HWBuffer)
        return;

    CNullDriver::deleteHardwareBuffer(HWBuffer);
}

void irr::video::CVulkanDriver::InitDrawStates(CVulkanHardwareBuffer* mb, scene::E_PRIMITIVE_TYPE pType)
{
    irr::video::CVulkanGLSLProgram* _vkShader = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());

    VulkanGraphicsPipelineState* _bindGraphicsPipeline = nullptr;
    if (mb)
    {
        scene::IMeshBuffer* buffer = mb->GetBuffer();
        if (buffer)
        {
            _bindGraphicsPipeline = mb->GetPipeline(buffer->GetActiveSubBuffer());
            mMainCommandBuffer->getInternal()->setPipelineState(_bindGraphicsPipeline);
            mMainCommandBuffer->getInternal()->setGpuParams(mb->GetGpuParams(buffer->GetActiveSubBuffer()));
        }
        else
        {
            _bindGraphicsPipeline = mb->GetPipeline(0);
            mMainCommandBuffer->getInternal()->setPipelineState(_bindGraphicsPipeline);
            mMainCommandBuffer->getInternal()->setGpuParams(mb->GetGpuParams(0));
        }
        mMainCommandBuffer->getInternal()->setDrawOp(pType);
    }


    auto GpuParams = mMainCommandBuffer->getInternal()->getGpuParams();
    u8 textureSize = GpuParams->GetTextureCount();
    for (int i = 0; i < textureSize; ++i)
    {
        if (CurrentTexture[i])
        {
            GpuParams->setTexture(0, i, CurrentTexture[i], static_cast<const CVulkanTexture*>(CurrentTexture[i])->GetSurface());
            GpuParams->setSamplerState(0, i, _bindGraphicsPipeline->getSampler(0, i));
        }
        //else
        //{
        //    static TextureSurface emptySurface;
        //    GpuParams->setTexture(0, i, nullptr, emptySurface);
        //}
    }
}

void irr::video::CVulkanDriver::drawMeshBuffer(const scene::IMeshBuffer * mb, scene::IMesh * mesh, scene::ISceneNode * node)
{
    if (!mb || !mb->getIndexCount())
        return;

    scene::E_PRIMITIVE_TYPE pType = mb->getRenderPrimitive();
    CVulkanHardwareBuffer* HWBuffer = static_cast<CVulkanHardwareBuffer*>(mb->GetHWBuffer());
    if (!HWBuffer)
    {
        CreateHardwareBuffer(mb);
        HWBuffer = static_cast<CVulkanHardwareBuffer*>(mb->GetHWBuffer());
    }

    bool hasConstantBuffer = mb->GetShaderConstantBuffers() != nullptr && !mb->GetShaderConstantBuffers()->empty();

    if (!mb->GetGPUProgram())
    {
        SetShader(m_defaultShader[mb->getVertexType()]);
    }
    else
    {
        SetShader(mb->GetGPUProgram());
    }

    _IRR_DEBUG_BREAK_IF(!GetActiveShader());

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
            mb->SetVertexDeclaration(this->GetVertexDeclaration(E_VERTEX_TYPE::EVT_STANDARD));

        // copy vertices to dynamic buffers, if needed
        uploadVertexData(mb->getVertices(), mb->getVertexCount(), mb->getIndices(), mb->getIndexCount(), mb->getVertexType(), mb->getIndexType());
        HWBuffer = static_cast<CVulkanGLSLProgram*>(GetActiveShader())->DynamicHardwareBuffer;
        if (!HWBuffer->IsBinded() || !HWBuffer->IsManualBind())
        {
            HWBuffer->Bind();
        }
    }

#ifdef _DEBUG
    if (!checkPrimitiveCount(mb->getVertexCount()))
        return;
#endif

    HWBuffer->GetPipeline(mb->GetActiveSubBuffer())->update(0, GetMaterial(), pType);

    InitDrawStates(HWBuffer, pType);
    SyncShaderConstant(HWBuffer, mesh, node);

    u32 instanceCount = (!mesh || mesh->IsInstanceModeAvailable()) && mb->getStreamBuffer() ? mb->getStreamBuffer()->size() : 1;

    HWBuffer->GetCommandBuffer()->getInternal()->drawIndexed(mb->GetIndexRangeStart(), mb->GetIndexRangeCount(), mb->GetBaseVertexLocation(), instanceCount);

    if (HWBuffer && !HWBuffer->IsManualBind())
        HWBuffer->Unbind();
}

void irr::video::CVulkanDriver::drawHardwareBuffer(IHardwareBuffer * HWBuffer, scene::IMesh * mesh, scene::ISceneNode * node)
{
}

void irr::video::CVulkanDriver::drawHardwareBuffer(IHardwareBuffer * vertices, IHardwareBuffer * indices, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType, u32 numInstances)
{
}

void irr::video::CVulkanDriver::drawAuto(IHardwareBuffer * vertices, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
}

void irr::video::CVulkanDriver::drawVertexPrimitiveList(const void * vertices, u32 vertexCount, const void * indexList, u32 primitiveCount, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);

    if (vertexCount || primitiveCount)
        draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount,
            vType, pType, iType, true);
}

void irr::video::CVulkanDriver::draw2DVertexPrimitiveList(const void * vertices, u32 vertexCount, const void * indexList, u32 primitiveCount, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::draw2DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);

    if (vertexCount || primitiveCount)
        draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount,
            vType, pType, iType, false);
}

const wchar_t * irr::video::CVulkanDriver::getName() const
{
    return DriverAndFeatureName.c_str();
}

void irr::video::CVulkanDriver::deleteAllDynamicLights()
{
    RequestedLights.clear();

    CNullDriver::deleteAllDynamicLights();
}

s32 irr::video::CVulkanDriver::addDynamicLight(const SLight & light)
{
    CNullDriver::addDynamicLight(light);

    RequestedLights.push_back(RequestedLight(light));

    u32 newLightIndex = RequestedLights.size() - 1;

    // Try and assign a hardware light just now, but don't worry if I can't
    //assignHardwareLight(newLightIndex);

    return (s32)newLightIndex;
}

void irr::video::CVulkanDriver::turnLightOn(s32 lightIndex, bool turnOn)
{
    if (lightIndex < 0 || lightIndex >= (s32)RequestedLights.size())
        return;

    RequestedLight & requestedLight = RequestedLights[lightIndex];

    requestedLight.DesireToBeOn = turnOn;
}

u32 irr::video::CVulkanDriver::getMaximalDynamicLightAmount() const
{
    return MaxActiveLights;
}

void irr::video::CVulkanDriver::setAmbientLight(const SColorf & color)
{
    AmbientLight = color;
}

void irr::video::CVulkanDriver::drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail, u32 debugDataVisible)
{
    if (!mCreateParams.Stencilbuffer)
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
    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT_WRAP;
    Material.StencilFront.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;

    Material.StencilBack.Mask = 0xFF;
    Material.StencilBack.WriteMask = 0xFF;
    Material.StencilBack.Reference = 0;

    Material.StencilBack.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT_WRAP;
    Material.StencilBack.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;

    //RasterizerDesc.AntialiasedLineEnable = false;
    //RasterizerDesc.MultisampleEnable = true;
    //RasterizerDesc.ScissorEnable = false;

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), nullptr, 0,
        E_VERTEX_TYPE::EVT_SHADOW, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    Material.FrontfaceCulling = zfail;
    Material.BackfaceCulling = !zfail;

    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT_WRAP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT_WRAP;

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), nullptr, 0,
        E_VERTEX_TYPE::EVT_SHADOW, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    CurrentRenderMode = zfail ? ERM_SHADOW_VOLUME_ZFAIL : ERM_SHADOW_VOLUME_ZPASS;
}

void irr::video::CVulkanDriver::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge, video::SColor rightUpEdge, video::SColor leftDownEdge, video::SColor rightDownEdge)
{
    if (!mCreateParams.Stencilbuffer)
        return;

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

    Matrices[ETS_PROJECTION] = core::IdentityMatrix;
    setTransform(E_TRANSFORMATION_STATE::ETS_VIEW, core::IdentityMatrix);
    setTransform(E_TRANSFORMATION_STATE::ETS_WORLD, core::IdentityMatrix);

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

    Material.StencilBack.Mask = 0xFF;
    Material.StencilBack.WriteMask = 0xFF;
    Material.StencilBack.Reference = 1;

    Material.StencilBack.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.Comparsion = E_COMPARISON_FUNC::ECFN_LESSEQUAL;

    draw2D3DVertexPrimitiveList(vtx, 4, indices, 6,
        E_VERTEX_TYPE::EVT_STANDARD, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    Matrices[ETS_PROJECTION] = proj;
    setTransform(E_TRANSFORMATION_STATE::ETS_VIEW, view);

    Material.StencilTest = false;
    Material.StencilFront.Mask = 0x0;
    Material.StencilFront.WriteMask = 0x0;
    Material.StencilFront.Reference = 0;

    if (clearStencilBuffer)
    {
        mMainCommandBuffer->getInternal()->clearViewport(FrameBufferType::FBT_STENCIL, SColor(), 1.0f, 0, 0xFF);
    }

    CurrentRenderMode = ERM_3D; // ERM_STENCIL_FILL;
}

u32 irr::video::CVulkanDriver::getMaximalPrimitiveCount() const
{
    return 0x7fffffff;
}

void irr::video::CVulkanDriver::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)
{
    if (flag == video::ETCF_CREATE_MIP_MAPS && !queryFeature(EVDF_MIP_MAP))
        enabled = false;

    CNullDriver::setTextureCreationFlag(flag, enabled);
}

void irr::video::CVulkanDriver::setFog(SColor color, E_FOG_TYPE fogType, f32 start, f32 end, f32 density, bool pixelFog, bool rangeFog)
{
    // just forward method, configurations will be applyed in the material renderer
    CNullDriver::setFog(color, fogType, start, end, density, pixelFog, rangeFog);
}

void irr::video::CVulkanDriver::OnResize(const core::dimension2d<u32>& size)
{
    CNullDriver::OnResize(size);

    if (mPlatform->GetSwapChain()->getWidth() != size.Width || mPlatform->GetSwapChain()->getHeight() != size.Height)
        mPlatform->resizeSwapBuffers();
}

void irr::video::CVulkanDriver::setBasicRenderStates(const SMaterial & material, const SMaterial & lastMaterial, bool resetAllRenderstates)
{
}

E_DRIVER_TYPE irr::video::CVulkanDriver::getDriverType() const
{
    return E_DRIVER_TYPE::EDT_VULKAN;
}

const core::matrix4 & irr::video::CVulkanDriver::getTransform(E_TRANSFORMATION_STATE state) const
{
    return Matrices[state];
}

void irr::video::CVulkanDriver::setVertexShaderConstant(const f32 * data, s32 startRegister, s32 constantAmount)
{
}

void irr::video::CVulkanDriver::setPixelShaderConstant(const f32 * data, s32 startRegister, s32 constantAmount)
{
}

bool irr::video::CVulkanDriver::setVertexShaderConstant(const c8 * name, const f32 * floats, int count)
{
    return false;
}

bool irr::video::CVulkanDriver::setPixelShaderConstant(const c8 * name, const f32 * floats, int count)
{
    return false;
}

bool irr::video::CVulkanDriver::setStreamOutputBuffer(IHardwareBuffer * buffer)
{
    return false;
}

IVideoDriver * irr::video::CVulkanDriver::getVideoDriver()
{
    return this;
}

ITexture * irr::video::CVulkanDriver::addRenderTargetTexture(const core::dimension2d<u32>& size, const io::path & name, const ECOLOR_FORMAT format, u8 sampleCount)
{
    VkSampleCountFlagBits sampleDesc = _getSwapChain()->getBackBuffer().framebuffer->getSampleFlags();
    if (sampleCount)
        sampleDesc = VkSampleCountFlagBits(sampleCount);

    ECOLOR_FORMAT _format = format;
    // ToDo:: multiple render pass when use different color format
    if (IImage::isDepthFormat(format))
    {
        _format = VulkanUtility::getPixelFormat(_getSwapChain()->getBackBuffer().framebufferDesc.depth.format);
    }
    else
    {
        _format = VulkanUtility::getPixelFormat(_getSwapChain()->getBackBuffer().framebufferDesc.color[0].format);
    }

    ITexture* tex = new CVulkanTexture(this, size, name, _format, 1, sampleDesc, 0);
    if (tex)
    {
        if (IImage::isRenderTargetOnlyFormat(format))
            checkDepthBuffer(tex);
        addTexture(tex);
        tex->drop();
    }
    return tex;
}

void irr::video::CVulkanDriver::clearZBuffer(f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    mMainCommandBuffer->getInternal()->clearViewport(FrameBufferType::FBT_DEPTH | FrameBufferType::FBT_STENCIL, irr::video::SColor(255, 0, 0, 0), clearDepth, clearStencil, 0xFF);
}

IImage * irr::video::CVulkanDriver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)
{
    u32 width = getCurrentRenderTargetSize().Width;
    u32 height = getCurrentRenderTargetSize().Height;

    auto& backBuffer = _getSwapChain()->getBackBuffer();
    ECOLOR_FORMAT colorFormat = VulkanUtility::getPixelFormat(backBuffer.image->GetImageFormat());

    //colorFormat = ECF_A8R8G8B8;
    //ECF_B8G8R8A8 ECF_A8R8G8B8;

    CVulkanTexture* srcTexture = new CVulkanTexture(this, backBuffer.image, getCurrentRenderTargetSize(), colorFormat);

    IImage* img = createImage(ECF_A8R8G8B8, getCurrentRenderTargetSize());

    MappedImageData backImageDesc = {};
    backImageDesc.format = colorFormat;
    void* imagePtr = img->lock();
    void* srcPtr = srcTexture->lock(E_TEXTURE_LOCK_MODE::ETLM_READ_ONLY);

    size_t srcImgSize = IImage::getMemorySize(width, height, 1, colorFormat);

    assert(img->getImageDataSizeInBytes() >= backImageDesc.size);

    // Copy data to image
    memcpy(imagePtr, srcPtr, srcImgSize);

    srcTexture->unlock();
    img->unlock();

    srcTexture->drop();

    // Return image
    return img;
}

bool irr::video::CVulkanDriver::setClipPlane(u32 index, const core::plane3df & plane, bool enable)
{
    if (index > 3)
        return false;

    ClipPlanes[index] = plane;

    enableClipPlane(index, enable);

    return true;
}

void irr::video::CVulkanDriver::enableClipPlane(u32 index, bool enable)
{
    ClipPlaneEnabled[index] = enable;
}

void irr::video::CVulkanDriver::getClipPlane(u32 index, core::plane3df & plane, bool & enable)
{
    plane = ClipPlanes[index];
    enable = ClipPlaneEnabled[index];
}

void irr::video::CVulkanDriver::enableMaterial2D(bool enable)
{
    CNullDriver::enableMaterial2D(enable);
}

ECOLOR_FORMAT irr::video::CVulkanDriver::getColorFormat() const
{
    return ColorFormat;
}

core::dimension2du irr::video::CVulkanDriver::getMaxTextureSize() const
{
    return core::dimension2du(16384, 16384);
}

IHardwareBuffer * irr::video::CVulkanDriver::createHardwareBuffer(E_HARDWARE_BUFFER_TYPE type, E_HARDWARE_BUFFER_ACCESS accessType, u32 size, u32 flags, const void * initialData)
{
    return new CVulkanHardwareBuffer(this, type, accessType, size, flags, initialData);
}

video::VertexDeclaration * irr::video::CVulkanDriver::createVertexDeclaration()
{
    return new CVulkanVertexDeclaration(this);
}

E_VERTEX_TYPE irr::video::CVulkanDriver::registerVertexType(core::array<SVertexElement>& elements)
{
    return E_VERTEX_TYPE();
}

u32 irr::video::CVulkanDriver::queryMultisampleLevels(ECOLOR_FORMAT format, u32 numSamples) const
{
    return u32();
}

//bool irr::video::CVulkanDriver::setShaderConstant(ShaderVariableDescriptor const * desc, const void * values, int count, IHardwareBuffer * buffer)
//{
//    irr::video::CVulkanGLSLProgram* hlsl = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());
//
//    if (desc->m_type == ESVT_INPUT_STREAM)
//        return false;
//
//    irr::video::CVulkanGLSLProgram::CbufferDesc& cbuffer = *hlsl->GetBlockBufferInfos()[desc->m_shaderIndex];
//    irr::video::CVulkanGLSLProgram::ShaderConstantDesc const& constantDecl = cbuffer.Variables[desc->m_location & 0xFF];
//
//    UINT elementSize = constantDecl.elementSize * count;
//
//    _IRR_DEBUG_BREAK_IF(elementSize > constantDecl.BufferDesc->DataBuffer.size() || (cbuffer.DataBuffer.size() < (constantDecl.offset + elementSize)));
//
//    if (memcmp(&cbuffer.DataBuffer[constantDecl.offset], values, elementSize))
//    {
//        if (cbuffer.ChangeStartOffset > constantDecl.offset)
//            cbuffer.ChangeStartOffset = constantDecl.offset;
//
//        if (cbuffer.ChangeEndOffset < (constantDecl.offset + elementSize))
//            cbuffer.ChangeEndOffset = (constantDecl.offset + elementSize);
//
//        memcpy(&cbuffer.DataBuffer[constantDecl.offset], values, elementSize);
//        ++cbuffer.ChangeId;
//    }
//    return true;
//}

bool irr::video::CVulkanDriver::SyncShaderConstant(CVulkanHardwareBuffer* HWBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
    if (!HWBuffer)
        return false;

    VulkanCommandBuffer* cb = HWBuffer->GetCommandBuffer();

    irr::video::CVulkanGLSLProgram* _vkShader = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());
    _IRR_DEBUG_BREAK_IF(!_vkShader);

    VulkanGpuParams* gpuParams = cb->getInternal()->getGpuParams();

    for (int i = 0; i < _vkShader->mBuffers.size(); ++i)
    {
        irr::video::SConstantBuffer* cbuffer = _vkShader->mBuffers[i];

        if (cbuffer->mCallBack)
            cbuffer->mCallBack->OnSetConstants(cbuffer, HWBuffer ? HWBuffer->GetBuffer() : nullptr, mesh, node);

        E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
        s8* dataBuffer;
        u32 dataBufferSize;
        u32 changeId = cbuffer->getChangedID();

        CVulkanHardwareBuffer * constantBuffer;
        if (HWBuffer->GetBuffer() && HWBuffer->GetBuffer()->GetShaderConstantBuffers() && !HWBuffer->GetBuffer()->GetShaderConstantBuffers()->empty())
        {
            SConstantBuffer* bufferCache = static_cast<SConstantBuffer*>((*HWBuffer->GetBuffer()->GetShaderConstantBuffers())[cbuffer->getBindingIndex()]);
            constantBuffer = static_cast<CVulkanHardwareBuffer*>(bufferCache->mHwObject);
            changeId = bufferCache->getChangedID();
            dataBuffer = (s8*)bufferCache->mHostMemory.data();
            dataBufferSize = bufferCache->mHostMemory.size();

            if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STATIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE;
            else if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_SYSTEM_MEMORY;

        }
        else
        {
            constantBuffer = static_cast<CVulkanHardwareBuffer*>(cbuffer->mHwObject);

            changeId = cbuffer->getChangedID();
            dataBuffer = (s8*)cbuffer->mHostMemory.data();
            dataBufferSize = cbuffer->mHostMemory.size();

            if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STATIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_IMMUTABLE;
            else if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_SYSTEM_MEMORY;

        }

        if (cbuffer->getHardwareMappingHint() == scene::EHM_NEVER)
            cbuffer->setHardwareMappingHint(scene::EHM_STATIC);

        {
            if (constantBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS) != changeId)
            {
                s32 preferBuffer = gpuParams->getBufferId(0, i);
                if (constantBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, MemoryAccess, dataBuffer,
                    dataBufferSize, 0, dataBufferSize, 0, preferBuffer))
                {
                    //cbuffer->ChangeStartOffset = cbuffer->DataBuffer.size();
                    //cbuffer->ChangeEndOffset = 0;
                    constantBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, changeId);
                }
            }
        }

        gpuParams->setBuffer(0, i, constantBuffer);
        constantBuffer->GetBufferDesc(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS)->Buffer->NotifySoftBound(VulkanUseFlag::eRead);
        //cb->getInternal()->registerResource(constantBuffer->GetBufferDesc(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS)->Buffer, VulkanUseFlag::Read);
    }

    return true;
}

IShader * irr::video::CVulkanDriver::createShader(ShaderInitializerEntry * shaderCreateInfo)
{
    irr::video::CVulkanGLSLProgram* gpuProgram = shaderCreateInfo->mShader ? static_cast<irr::video::CVulkanGLSLProgram*>(shaderCreateInfo->mShader.GetPtr()) : new irr::video::CVulkanGLSLProgram(this);

    for (auto stage : shaderCreateInfo->mStages)
    {
        if (!stage->DataStream)
            continue;

        const char* ShaderModel = stage->ShaderModel.c_str();
        const char* ShaderEntryPoint = stage->EntryPoint.c_str();

        stage->DataStream->Seek(stage->DataStreamOffset, false);
        gpuProgram->CreateShaderModul(stage->ShaderStageType, this, stage->DataStream, ShaderEntryPoint, ShaderModel);

        switch (stage->ShaderStageType)
        {
        case E_SHADER_TYPES::EST_VERTEX_SHADER:
            VulkanUtility::setObjectNamef(*_getPrimaryDevice(), (uint64_t)* gpuProgram->GetShaderModule(stage->ShaderStageType), VkObjectType::VK_OBJECT_TYPE_SHADER_MODULE, "%s_VS", stage->DataStream->FileName.c_str());
            break;
        case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
            VulkanUtility::setObjectNamef(*_getPrimaryDevice(), (uint64_t)* gpuProgram->GetShaderModule(stage->ShaderStageType), VkObjectType::VK_OBJECT_TYPE_SHADER_MODULE, "%s_GS", stage->DataStream->FileName.c_str());
            break;
        case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
            VulkanUtility::setObjectNamef(*_getPrimaryDevice(), (uint64_t)* gpuProgram->GetShaderModule(stage->ShaderStageType), VkObjectType::VK_OBJECT_TYPE_SHADER_MODULE, "%s_PS", stage->DataStream->FileName.c_str());
            break;
        }
    }

    for (auto cbEntry : shaderCreateInfo->Callback)
    {
        auto buffer = gpuProgram->getConstantBufferByName(cbEntry.first.c_str());
        if (buffer)
            buffer->setShaderDataCallback(cbEntry.second);
        cbEntry.second->OnPrepare(buffer);
    }

    gpuProgram->Init();
    shaderCreateInfo->mShaderId = gpuProgram->mId = AddShaderModul(gpuProgram, shaderCreateInfo->mShaderId);
    return gpuProgram;
}

void irr::video::CVulkanDriver::useShader(IShader * gpuProgram)
{
}

void irr::video::CVulkanDriver::HandleDeviceLost()
{
}

void irr::video::CVulkanDriver::createMaterialRenderers()
{
}

void irr::video::CVulkanDriver::draw2D3DVertexPrimitiveList(const void * vertices, u32 vertexCount, const void * indexList, u32 primitiveCount, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType, bool is3D)
{
    if (vType < E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE && GetActiveShader() != m_defaultShader[vType])
        SetShader(m_defaultShader[vType]);
    _IRR_DEBUG_BREAK_IF(!GetActiveShader());

    size_t indexCount = VulkanUtility::getIndexAmount(pType, primitiveCount);

    uploadVertexData(vertices, vertexCount, indexList, indexCount, vType, iType);

#ifdef _DEBUG
    if (!checkPrimitiveCount(vertexCount))
        return;
#endif

    auto HWBuffer = static_cast<CVulkanGLSLProgram*>(GetActiveShader())->DynamicHardwareBuffer;
    HWBuffer->GetPipeline(0)->update(0, GetMaterial(), pType);
    HWBuffer->Bind();

    InitDrawStates(HWBuffer, pType);
    SyncShaderConstant(HWBuffer);

    if (!indexCount)
        HWBuffer->GetCommandBuffer()->getInternal()->draw(0, vertexCount, 1);
    else
        HWBuffer->GetCommandBuffer()->getInternal()->drawIndexed(0, indexCount, 0, 1);

    HWBuffer->Unbind();
}

bool irr::video::CVulkanDriver::setRenderStates3DMode()
{
    if (ResetRenderStates || LastMaterial != Material)
    {
        irr::video::CVulkanGLSLProgram* _vkShader = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());
        if (!_vkShader)
            _vkShader = m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD];

        if (_vkShader)
        {
            for (int i = 0; i < _vkShader->mBuffers.size(); ++i)
            {
                irr::video::SConstantBuffer* cbuffer = _vkShader->mBuffers[i];
                if (cbuffer->mCallBack)
                    cbuffer->mCallBack->OnSetMaterial(cbuffer, Material);
            }
        }

        LastMaterial = Material;
        ResetRenderStates = false;
    }

    CurrentRenderMode = ERM_3D;

    return true;
}

void irr::video::CVulkanDriver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
{
    if (CurrentRenderMode != ERM_2D || Transformation3DChanged)
    {
        // Set world to identity
        core::matrix4 m;
        setTransform(ETS_WORLD, m);
        
        // Adjust projection
        const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
        
        auto _tmp_mat = glm::ortho(0.0f, (f32)renderTargetSize.Width, 0.0f, (f32)renderTargetSize.Height, 0.0f, 1.0f);
        m = (core::matrix4&)_tmp_mat;
        Matrices[ETS_PROJECTION] = m;
        
        Transformation3DChanged = false;
    }

    // Set view to translate a little forward
    //m.setTranslation(core::vector3df(-0.5f, -0.5f, 0)ü);
    setTransform(ETS_VIEW, getTransform(ETS_VIEW_2D));
    Transformation3DChanged = false;

    // no alphaChannel without texture
    alphaChannel &= texture;

    if (texture)
    {
        setTransform(ETS_TEXTURE_0, core::IdentityMatrix);
        // Due to the transformation change, the previous line would call a reset each frame
        // but we can safely reset the variable as it was false before
        Transformation3DChanged = false;
    }

    Material.AntiAliasing = video::EAAM_OFF;
    Material.Lighting = false;
    Material.ZBuffer = ECFN_NEVER;
    Material.ZWriteEnable = false;

    // handle alpha
    if (alpha || alphaChannel)
    {
        Material.MaterialType = E_MATERIAL_TYPE::EMT_TRANSPARENT_ALPHA_CHANNEL;
        Material.BlendOperation = E_BLEND_OPERATION::EBO_ADD;
        Material.ColorMask = ECP_ALL;

        Material.uMaterialTypeParam = pack_textureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA, video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR, video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA, E_BLEND_OPERATION::EBO_ADD, E_BLEND_OPERATION::EBO_ADD);
    }
    else
    {
        Material.BlendOperation = E_BLEND_OPERATION::EBO_NONE;
    }

    if (ResetRenderStates || LastMaterial != Material)
    {
        irr::video::CVulkanGLSLProgram* shader = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());
        if (!shader)
            shader = m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD];

        if (shader)
        {
            for (int i = 0; i < shader->mBuffers.size(); ++i)
            {
                irr::video::SConstantBuffer* cbuffer = shader->mBuffers[i];
                if (cbuffer->mCallBack)
                    cbuffer->mCallBack->OnSetMaterial(cbuffer, Material);
            }
        }

        setBasicRenderStates(Material, LastMaterial, ResetRenderStates);

        LastMaterial = Material;
        ResetRenderStates = false;
    }

    CurrentRenderMode = ERM_2D;
}

bool irr::video::CVulkanDriver::setActiveTexture(u32 stage, const video::ITexture * texture)
{
    if (texture && texture->getDriverType() != EDT_VULKAN)
    {
        os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
        return false;
    }

    // For first stage, set NullTexture if texture is NULL
    CurrentTexture[stage] = (stage == 0 && texture == 0) ? nullptr : texture;

    return true;

}

video::ITexture * irr::video::CVulkanDriver::createDeviceDependentTexture(IImage * surface, const io::path & name)
{
    return new CVulkanTexture(surface, this, TextureCreationFlags, name, 1, nullptr, 1, 0);
}

void irr::video::CVulkanDriver::checkDepthBuffer(ITexture * tex)
{
}

const core::dimension2d<u32>& irr::video::CVulkanDriver::getCurrentRenderTargetSize() const
{
    if (CurrentRendertargetSize.Width == 0)
        return ScreenSize;
    else
        return CurrentRendertargetSize;
}

bool irr::video::CVulkanDriver::reallocateDynamicBuffers(u32 vertexBufferSize, u32 indexBufferSize)
{
    return false;
}

bool irr::video::CVulkanDriver::uploadVertexData(const void * vertices, u32 vertexCount, const void * indexList, u32 indexCount, E_VERTEX_TYPE vType, E_INDEX_TYPE iType)
{
    u32 vertexStride;
    irr::video::CVulkanGLSLProgram* _vkShader = static_cast<irr::video::CVulkanGLSLProgram*>(GetActiveShader());
    if (!_vkShader)
        return false;

    CVulkanVertexDeclaration* vertexDecl = static_cast<CVulkanVertexDeclaration*>(GetVertexDeclaration(vType));
    {
        if (vType >= E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE)
        {
            vertexStride = vertexDecl->GetVertexPitch(0);
        }
        else
        {
            vertexStride = getVertexPitchFromType(vType);
        }
    }

    if (!_vkShader->DynamicHardwareBuffer)
    {
        _vkShader->DynamicHardwareBuffer = irr::MakePtr<CVulkanHardwareBuffer>(this, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, 0);

        if (vertices)
            _vkShader->DynamicHardwareBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, vertices, vertexStride * vertexCount);

        if (indexList)
            _vkShader->DynamicHardwareBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, indexList, indexCount * (iType == E_INDEX_TYPE::EIT_32BIT ? sizeof(u32) : sizeof(u16)));

        VulkanGraphicsPipelineState*& pipeline = mPipelines[_vkShader][vertexDecl];
        if (!pipeline)
        {
            pipeline = new VulkanGraphicsPipelineState(GetMaterial(), _vkShader, static_cast<CVulkanVertexDeclaration*>(GetVertexDeclaration(vType)), GpuDeviceFlags::GDF_PRIMARY);
            pipeline->initialize();
        }

        _vkShader->DynamicHardwareBuffer->SetCommandBuffer(mMainCommandBuffer);
        _vkShader->DynamicHardwareBuffer->SetPipeline(0, pipeline); // new VulkanGraphicsPipelineState(GetMaterial(), _vkShader, static_cast<CVulkanVertexDeclaration*>(GetVertexDeclaration(vType)), GpuDeviceFlags::GDF_PRIMARY));

        _vkShader->DynamicHardwareBuffer->SetGpuParams(0, _vkShader->GetDefaultGpuParams());
    }
    else
    {
        if (vertices)
            _vkShader->DynamicHardwareBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, vertices, vertexStride * vertexCount);

        if (indexList)
            _vkShader->DynamicHardwareBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, E_HARDWARE_BUFFER_ACCESS::EHBA_STREAM, indexList, indexCount * (iType == E_INDEX_TYPE::EIT_32BIT ? sizeof(u32) : sizeof(u16)));
    }

    return true;
}

void irr::video::CVulkanDriver::reset()
{
}

namespace irr
{
    namespace video
    {
        //! creates a video driver
        IVideoDriver* createVulkanDriver(const SIrrlichtCreationParameters& params,
            io::IFileSystem* io, void* param)
        {
            CVulkanDriver* driver = new CVulkanDriver(params, io);
            if (!driver->initDriver(param))
            {
                driver->drop();
                driver = 0;
            }

            return driver;
        }

        void loadVulkanShaderCache(System::IO::IFileReader* file)
        {
            VKLoadShaderCache(file);
        }
    }
}
