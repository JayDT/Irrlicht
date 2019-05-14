#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Driver.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include <stdio.h>
#include <fstream>

#define _WIN32_WINNT 0x600
#include <comdef.h>  // you will need this

#include <DirectXMath.h>
#include "os.h"
#include "S3DVertex.h"
#include "CD3D11Texture.h"
#include "CD3D11MaterialRenderer.h"
#include "CD3D11FixedFunctionMaterialRenderer.h"
#include "CD3D11CustomMaterialRenderer.h"
#include "CD3D11NormalMapRenderer.h"
#include "CD3D11VertexDeclaration.h"
#include "CD3D11HardwareBuffer.h"
#include "CD3D11Shader.h"
#include "CD3D11Texture.h"
#include "CD3D11RenderTarget.h"
#include "IVertexBuffer.h"
#include "IShaderConstantSetCallBack.h"
#include "standard/client/DataSource_Standard.h"
#include "buildin_data.h"

#include "DirectXDefines.h"

//#ifdef HAVE_CSYSTEM_EXTENSION
#include "System/Uri.h"
#include "System/Resource/ResourceManager.h"
//#endif

#undef min
#undef max

static DWORD STATHREADiD = 0;
extern DirectX::XMMATRIX UnitMatrixD3D11;
extern DirectX::XMMATRIX SphereMapMatrixD3D11;

/////////////
// LINKING //
/////////////
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

#define SAFE_RELEASE(x)     \
    if(x)                   \
        x->Release();       \
    x = 0;

#define RELEASE_ALL(x)            \
    ULONG rest = x->Release();    \
    while( rest > 0 )             \
        rest = x->Release();      \
    x = 0;

namespace irr
{

namespace video
{

    namespace
    {
        inline DWORD F2DW(FLOAT f)
        {
            return *((DWORD*)&f);
        }
    }

//! constructor
CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters& params, io::IFileSystem* io, const bool pureSoftware /*= false*/)
    : CNullDriver(io, params.WindowSize),
    //CurrentRenderMode(ERM_NONE),
    ResetRenderStates(true), ResetBlending(false), Transformation3DChanged(false), Block2DRenderStateChange(false),
    DriverType(D3D_DRIVER_TYPE_HARDWARE),
    D3DLibrary(0), Device(0), ImmediateContext(0), SwapChain(0), Adapter(0), DXGIFactory(0), DXGIDevice(0),
    Output(0), CurrentInputLayout(0), DefaultBackBuffer(0), DefaultDepthBuffer(0),
    DynVertexBuffer(0), DynIndexBuffer(0), DynVertexBufferSize(0), DynIndexBufferSize(0),
    SceneSourceRect(0), LastVertexType((video::E_VERTEX_TYPE)-1), VendorID(0), ColorFormat(ECF_A8R8G8B8),
    MaxActiveLights(8), AlphaToCoverageSupport(true), DepthStencilFormat(DXGI_FORMAT_UNKNOWN),
    D3DColorFormat(DXGI_FORMAT_R8G8B8A8_UNORM), Params(params), CurrentDepthBuffer(nullptr), CurrentBackBuffer(nullptr),
    // DirectX 11 can handle much more than this value, but keep compatibility
    MaxTextureUnits(MATERIAL_MAX_TEXTURES) 

{
    #ifdef _DEBUG
    setDebugName("CD3D11Driver");
    #endif

    memset(m_defaultShader, 0, sizeof(m_defaultShader));

    enableMaterial2D(true);

    blockgpuprogramchange = false;

    blankImage = nullptr;
    blankTexture = nullptr;

    BlendState = nullptr;
    RasterizerState = nullptr;
    DepthStencilState = nullptr;
    memset(SamplerState, 0, sizeof(SamplerState));

    printVersion();

    for(u32 i=0; i<MATERIAL_MAX_TEXTURES; ++i)
    {
        CurrentTexture[i] = 0;
    }

    // init clip planes
    ClipPlanes.push_back( core::plane3df() );
    ClipPlanes.push_back( core::plane3df() );
    ClipPlanes.push_back( core::plane3df() );
    ClipPlaneEnabled[0] = ClipPlaneEnabled[1] = ClipPlaneEnabled[2] = false;

    // create sphere map matrix
    SphereMapMatrixD3D11 = DirectX::XMMatrixSet(0.5f, 0.0f, 0.0f, 0.0f,
                                        0.0f, -0.5f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f,
                                        0.5f, 0.5f, 0.0f, 1.0f);

    core::matrix4 mat;
    UnitMatrixD3D11 = *(DirectX::XMMATRIX*)((void*)mat.pointer());

    // TODO: 1x1 texture
    blankImage = irr::Ptr<IImage>(*createImage(ECF_A1R5G5B5, getCurrentRenderTargetSize()));
    // Size.Height * Pitch
    memset(blankImage->lock(), 0, (blankImage->getPitch() * blankImage->getDimension().Height));
    blankImage->unlock();
}

CD3D11Driver::~CD3D11Driver()
{
	for (auto& s : m_rtResolveShader)
		s = nullptr;

    // ToDo: free resources
    for (s32 i = 0; i < E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE; ++i)
    {
        if (!m_defaultShader[i])
            continue;

        delete m_defaultShader[i];
    }

    ReleaseDriver();

    FreeLibrary(D3DLibrary);
}

void CD3D11Driver::ReleaseDriver()
{
    // Unbound all shader resources
    ID3D11ShaderResourceView* views[1] = { NULL };
    ImmediateContext->VSSetShaderResources(0, 1, views);
    ImmediateContext->HSSetShaderResources(0, 1, views);
    ImmediateContext->DSSetShaderResources(0, 1, views);
    ImmediateContext->GSSetShaderResources(0, 1, views);
    ImmediateContext->PSSetShaderResources(0, 1, views);

    // Set windowed mode before release swap chain
    SwapChain->SetFullscreenState( FALSE, NULL );

    // Delete renderers and textures
    deleteMaterialRenders();
    deleteAllTextures();

    // clear state
    if (ImmediateContext)
    {
        ImmediateContext->ClearState();
        ImmediateContext->Flush();
    }

    if ( BlendState )
        BlendState->Release();
    BlendState = nullptr;

    if ( RasterizerState )
        RasterizerState->Release();
    RasterizerState = nullptr;

    if ( DepthStencilState )
        DepthStencilState->Release();
    DepthStencilState = nullptr;

    for ( u32 i = 0; i != MATERIAL_MAX_TEXTURES; ++i )
        if (SamplerState[i])
        {
            SamplerState[i]->Release();
            SamplerState[i] = nullptr;
        }

    //RELEASE_ALL(Device);
}

void CD3D11Driver::createMaterialRenderers()
{
    // create D3D11 material renderers
    CD3D11MaterialRenderer_SOLID* renderer = new CD3D11MaterialRenderer_SOLID(this, FileSystem);
    addMaterialRenderer( renderer );
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_SOLID_2_LAYER(this, FileSystem) ); // video::EMT_SOLID_2_LAYER;

    CD3D11MaterialRenderer_LIGHTMAP* lmr = new CD3D11MaterialRenderer_LIGHTMAP(this, FileSystem);
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_ADD
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_M2
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_M4
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_LIGHTING
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_LIGHTING_M2
    addMaterialRenderer( lmr ); // video::EMT_LIGHTMAP_LIGHTING_M4
    lmr->drop();

    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_DETAIL_MAP(this, FileSystem) ); // video::EMT_DETAIL_MAP
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_SPHERE_MAP(this, FileSystem) ); // video::EMT_SPHERE_MAP
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_REFLECTION_2_LAYER(this, FileSystem) ); // video::EMT_REFLECTION_2_LAYER
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(this, FileSystem) ); // video::EMT_TRANSPARENT_ADD_COLOR
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(this, FileSystem) ); // video::EMT_TRANSPARENT_ALPHA_CHANNEL
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(this, FileSystem) ); // video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(this, FileSystem) ); // video::EMT_TRANSPARENT_VERTEX_ALPHA
    addAndDropMaterialRenderer( new CD3D11MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(this, FileSystem) ); // video::EMT_TRANSPARENT_REFLECTION_2_LAYER

    addMaterialRenderer(renderer);
    addMaterialRenderer(renderer);
    addMaterialRenderer(renderer);

    // The following shall be changed to parallax in future
    addMaterialRenderer( renderer );
    addMaterialRenderer( renderer );
    addMaterialRenderer( renderer );
    renderer->drop();

    addMaterialRenderer(new CD3D11MaterialRenderer_ONETEXTURE_BLEND(this, FileSystem));
    addAndDropMaterialRenderer(new CD3D11MaterialRenderer_RAW(this, FileSystem));
}

void CD3D11Driver::InitAdapters(void)
{
    UINT pSelectedAdapter = 0;
	std::vector <Msw::ComPtr<IDXGIAdapter>> vAdapters;
    std::vector <DXGI_ADAPTER_DESC> vAdaptersDesc;

    if (!DXGIFactory)
    {
        // Create a DXGIFactory object.
        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)DXGIFactory.GetAddressOf())))
        {
            throw std::runtime_error("Critical Error");
        }
    }

    for (UINT i = 0; DXGIFactory->EnumAdapters(i, Adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        vAdapters.push_back(Adapter);
        DXGI_ADAPTER_DESC adapDesc;
        ::ZeroMemory(&adapDesc, sizeof(DXGI_ADAPTER_DESC));
        // Get the adapter (video card) description.
        if (FAILED(Adapter->GetDesc(&adapDesc)))
        {
            throw std::runtime_error("Critical Error");
        }

        vAdaptersDesc.push_back(adapDesc);

        // Select best performance adapter
        if (vAdaptersDesc[i].DedicatedVideoMemory > vAdaptersDesc[pSelectedAdapter].DedicatedVideoMemory)
            pSelectedAdapter = i;
    }

    Adapter = vAdapters[pSelectedAdapter];
}

bool CD3D11Driver::initOutput(HWND hwnd, bool pureSoftware)
{
    HRESULT hr;

    // Get adapter used by this device and query informations

    if (SUCCEEDED(DXGIDevice->GetAdapter(&Adapter)))
    {
        DXGI_ADAPTER_DESC adapDesc;
        ::ZeroMemory(&adapDesc, sizeof(DXGI_ADAPTER_DESC));

        Adapter->GetDesc(&adapDesc);

        s32 Product = HIWORD(adapDesc.AdapterLuid.HighPart);
        s32 Version = LOWORD(adapDesc.AdapterLuid.HighPart);
        s32 SubVersion = HIWORD(adapDesc.AdapterLuid.LowPart);
        s32 Build = LOWORD(adapDesc.AdapterLuid.LowPart);

        char description[120];
        wcstombs(description, adapDesc.Description, wcslen(adapDesc.Description) + 1);    // convert from wchar_t* to char*
        char tmp[512];
        snprintf(tmp, 512, "%s, Revision %d", description, adapDesc.Revision);
        os::Printer::log(tmp, ELL_INFORMATION);
        os::Printer::log("Currently available Video Memory (kB)", core::stringc((int)(adapDesc.DedicatedVideoMemory / 1024)).c_str());

        // Assign vendor name based on vendor id.
        VendorID = static_cast<u16>(adapDesc.VendorId);
        switch (VendorID)
        {
        case 0x1002: VendorName = "ATI Technologies Inc."; break;
        case 0x10DE: VendorName = "NVIDIA Corporation"; break;
        case 0x102B: VendorName = "Matrox Electronic Systems Ltd."; break;
        case 0x121A: VendorName = "3dfx Interactive Inc"; break;
        case 0x5333: VendorName = "S3 Graphics Co., Ltd."; break;
        case 0x8086: VendorName = "Intel Corporation"; break;
        default: VendorName = "Unknown VendorId: "; VendorName += (u32)adapDesc.VendorId; break;
        }

        const WCHAR* wc = adapDesc.Description;
        _bstr_t b(wc);

        ZeroMemory(tmp, 512);
        snprintf(tmp, 512, "Dx %s GPU: %s",
#ifdef _WIN32_WINNT_WIN10
            (FeatureLevel == D3D_FEATURE_LEVEL_12_1) ? "12.1" :
            (FeatureLevel == D3D_FEATURE_LEVEL_12_0) ? "12.0" :
#endif
#ifdef _WIN32_WINNT_WINBLUE
            (FeatureLevel == D3D_FEATURE_LEVEL_11_1) ? "11.1" :
#endif
            (FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? "11.0" :
            (FeatureLevel == D3D_FEATURE_LEVEL_10_1) ? "10.1" : "10.0", b.operator const char*());

        size_t length = 512;
        std::wstring out(length, L'#');
        mbstowcs(&out[0], tmp, length);

        DriverAndFeatureName = out;
    }

    if ( FAILED(Adapter->EnumOutputs(0, Output.GetAddressOf())) )
    {
        //UINT i = 0;
        //IDXGIOutput * pOutput;
        //std::vector<IDXGIOutput*> vOutputs;
        //while (Adapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
        //{
        //    vOutputs.push_back(pOutput);
        //    ++i;
        //}

        os::Printer::log("Error, could not get monitor", ELL_INFORMATION);
        //throw std::runtime_error("Critical Error");
    }

    INT selectedMode = -1;
    DXGI_MODE_DESC* displayModes = nullptr;

    if (Output)
    {
        UINT numModes = 0;

        // Get the number of elements
        hr = Output->GetDisplayModeList(D3DColorFormat, 0, &numModes, NULL);
        if (FAILED(hr))
        {
            throw std::runtime_error("Critical Error");
        }

        displayModes = new DXGI_MODE_DESC[numModes];

        // Get the list
        hr = Output->GetDisplayModeList(D3DColorFormat, 0, &numModes, displayModes);
        if (FAILED(hr))
        {
            throw std::runtime_error("Critical Error");
        }

        m_displayModeList.set_used(numModes);
        for (u32 i = 0; i != numModes; ++i)
        {
            m_displayModeList[i].Resolution.Height = displayModes[i].Height;
            m_displayModeList[i].Resolution.Width = displayModes[i].Width;
            m_displayModeList[i].Format = DirectXUtil::getColorFormatFromD3DFormat(displayModes[i].Format);
            m_displayModeList[i].RefreshRate = displayModes[i].RefreshRate.Numerator;
            m_displayModeList[i].param0 = displayModes[i].Scaling;
            m_displayModeList[i].param1 = displayModes[i].ScanlineOrdering;

            if (displayModes[i].Height == Params.WindowSize.Height &&
                displayModes[i].Width == Params.WindowSize.Width &&
                displayModes[i].Format == D3DColorFormat)
                selectedMode = i;
        }

        if (selectedMode == -1)
        {
            selectedMode = numModes - 1;
            //throw std::runtime_error("Critical Error");
        }
    }

    // Initialize the swap chain description.
    ZeroMemory(&present, sizeof(present));

    // Set to a single back buffer.
    present.BufferCount = 2;

    // Set the usage of the back buffer.
    present.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    if (displayModes)
    {
        present.Width = displayModes[selectedMode].Width;
        present.Height = displayModes[selectedMode].Height;
        present.Format = displayModes[selectedMode].Format;           // This is the most common swap chain format.
    }
    else  // Fail Safe
    {
        present.Width  = 0;                                     // Use automatic sizing.
        present.Height = 0;
        present.Format = D3DColorFormat;                        // This is the default suggest swap chain format.
    }

    // Turn multisampling off.
    present.SampleDesc.Count = 1;
    present.SampleDesc.Quality = 0;

	UINT quality = 0;
	UINT counts[MSAA::Count] = { 1, 2, 4, 8, 16 };

	for (uint32_t i = 0, last = 0; i < MSAA::Count; i++)
	{
		UINT count = counts[i];

		HRESULT hr = Device->CheckMultisampleQualityLevels(present.Format, count, &quality);

		if (SUCCEEDED(hr) && quality > 0)
		{
			mSampleDescs[i].Count = count;
			mSampleDescs[i].Quality = quality - 1;
			last = i;
		}
		else
		{
			mSampleDescs[i] = mSampleDescs[last];
		}
	}

    // enable anti alias if possible and desired
    if (Params.AntiAlias > 0)
    {
        if (Params.AntiAlias > 32)
            Params.AntiAlias = 32;
	
        //if (Params.AntiAlias == 0)
        //{
        //    os::Printer::log("Anti aliasing disabled because hardware/driver lacks necessary caps.", ELL_WARNING);
        //}
    }

	present.SampleDesc = mSampleDescs[(u32)DirectXUtil::ToMSAA(Params.AntiAlias)];

    present.Stereo = false;
    present.Scaling = DXGI_SCALING_STRETCH;
    present.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // All Windows Store apps must use this SwapEffect.
    //present.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    // Don't set the advanced flags.
    present.Flags = Params.Fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0; // allow full-screen switchin if need

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenPresent;
    if (Params.Vsync && displayModes)
    {
        fullscreenPresent.RefreshRate = displayModes[selectedMode].RefreshRate;
    }
    else
    {
        fullscreenPresent.RefreshRate.Numerator = 0;
        fullscreenPresent.RefreshRate.Denominator = 1;
    }

    fullscreenPresent.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    fullscreenPresent.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fullscreenPresent.Windowed = !Params.Fullscreen;

    delete[] displayModes;


    hr = DXGIFactory->CreateSwapChainForHwnd(Device.Get(), hwnd, &present, &fullscreenPresent, Output.Get(), SwapChain.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Critical Error");
    }

    if (FAILED(DXGIFactory->MakeWindowAssociation(hwnd, 0
        | DXGI_MWA_NO_WINDOW_CHANGES
        | DXGI_MWA_NO_ALT_ENTER)))
        return false;


    hr = Device->QueryInterface(IID_ID3D11InfoQueue, (void**)&m_infoQueue);
    if (SUCCEEDED(hr))
    {
        m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, false);

        D3D11_INFO_QUEUE_FILTER filter;
        memset(&filter, 0, sizeof(filter));

        D3D11_MESSAGE_CATEGORY catlist[] =
        {
            D3D11_MESSAGE_CATEGORY_STATE_SETTING,
            D3D11_MESSAGE_CATEGORY_EXECUTION,
        };
        //filter.DenyList.NumCategories = COUNTOF(catlist);
        filter.DenyList.pCategoryList = catlist;
        m_infoQueue->PushStorageFilter(&filter);

        m_infoQueue = nullptr;
    }

    // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
    // ensures that the application will only render after each VSync, minimizing power consumption.
    DX::ThrowIfFailed(
        DXGIDevice->SetMaximumFrameLatency(1)
        );

    return true;
}

bool getIntelExtensions(ID3D11Device* _device)
{
    uint8_t temp[28];

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeof(temp);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &temp;
    initData.SysMemPitch = sizeof(temp);
    initData.SysMemSlicePitch = 0;

    //bx::StaticMemoryBlockWriter writer(&temp, sizeof(temp));
    //bx::write(&writer, "INTCEXTNCAPSFUNC", 16);
    //bx::write(&writer, UINT32_C(0x00010000));
    //bx::write(&writer, UINT32_C(0));
    //bx::write(&writer, UINT32_C(0));
    //
    //ID3D11Buffer* buffer;
    //HRESULT hr = _device->CreateBuffer(&desc, &initData, buffer.GetAddressOf());
    //
    //if (SUCCEEDED(hr))
    //{
    //    buffer->Release();
    //
    //    bx::MemoryReader reader(&temp, sizeof(temp));
    //    bx::skip(&reader, 16);
    //
    //    uint32_t version;
    //    bx::read(&reader, version);
    //
    //    uint32_t driverVersion;
    //    bx::read(&reader, driverVersion);
    //
    //    return version <= driverVersion;
    //}

    return false;
};

//! initialises the Direct3D API
bool CD3D11Driver::initDriver(HWND hwnd, bool pureSoftware)
{
    CNullDriver::initDriver();

	HRESULT hr;

    D3DLibrary = LoadLibrary(__TEXT("d3d11.dll"));
    if (!D3DLibrary)
    {
        os::Printer::log("Error, could not load d3d11.dll.", ELL_ERROR);
        return false;
    }

    InitAdapters();

    // Device flags
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT deviceFlags = 0; // D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (!Params.DriverMultithreaded)
        deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
#if IRR_DIRECTX_DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // | D3D11_CREATE_DEVICE_DEBUGGABLE | D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif

    // Get function address
    PFN_D3D11_CREATE_DEVICE CreateDeviceFunc = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(D3DLibrary, "D3D11CreateDevice");
    if (!CreateDeviceFunc)
    {
        os::Printer::log("Error, could not get D3D11CreateDevice function.", ELL_ERROR);
        return false;
    }

    // Try creating hardware device
    static D3D_FEATURE_LEVEL RequestedLevels[] = {
#ifdef _WIN32_WINNT_WIN10
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
#endif
#ifdef _WIN32_WINNT_WINBLUE
        D3D_FEATURE_LEVEL_11_1,
#endif
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ID3D11Device* device;
    ID3D11DeviceContext* context;

    u32 RequestedLevelsSize = sizeof(RequestedLevels) / sizeof(RequestedLevels[0]);
#if	IRR_DIRECTX_DEBUG
    //DriverType = D3D_DRIVER_TYPE_WARP;
    hr = CreateDeviceFunc(NULL/*Adapter*/, D3D_DRIVER_TYPE_WARP, NULL, deviceFlags, RequestedLevels, RequestedLevelsSize, D3D11_SDK_VERSION, &device, &FeatureLevel, &context);
#else
    hr = CreateDeviceFunc(Adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN/*DriverType*/, NULL, deviceFlags, RequestedLevels, RequestedLevelsSize, D3D11_SDK_VERSION, &device, &FeatureLevel, &context);
#endif
    if (FAILED(hr))
    {
        //printf("[HLSL SHADER] Error: %s error description: %s\n",
        //    DXGetErrorString(hr), DXGetErrorDescription(hr));
        os::Printer::log("Error, could not create device.", ELL_ERROR);
        throw std::runtime_error("Error, could not create device.");
        return false;
    }

    DX::ThrowIfFailed(device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(Device.GetAddressOf())));
    DX::ThrowIfFailed(context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(ImmediateContext.GetAddressOf())));
    DX::ThrowIfFailed(Device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(DXGIDevice.GetAddressOf())));
#ifdef IRR_DIRECTX_INSTRUMENTED_ENABLED
	DX::ThrowIfFailed(ImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**) GroupMarker.GetAddressOf()));
#endif

	// Printing type of Direct3D 11 device and feature level
    char msg[512];
    sprintf(msg, "Using %s device, feature level %s",
        (DriverType == D3D_DRIVER_TYPE_HARDWARE) ? "Hardware" : "WARP",
#ifdef _WIN32_WINNT_WIN10
        (FeatureLevel == D3D_FEATURE_LEVEL_12_1) ? "12.1" :
        (FeatureLevel == D3D_FEATURE_LEVEL_12_0) ? "12.0" :
#endif
#ifdef _WIN32_WINNT_WINBLUE
        (FeatureLevel == D3D_FEATURE_LEVEL_11_1) ? "11.1" :
#endif
        (FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? "11.0" :
        (FeatureLevel == D3D_FEATURE_LEVEL_10_1) ? "10.1" : "10.0");
    os::Printer::log(msg, ELL_INFORMATION);

    initOutput(hwnd, pureSoftware);
    //getIntelExtensions(Device);

    // Get default render target
    ID3D11Texture2D* backBuffer = NULL;
    hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr))
    {
        os::Printer::log("Error, could not get back buffer.", ELL_ERROR);
        return false;
    }
    hr = Device->CreateRenderTargetView(backBuffer, NULL, DefaultBackBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        os::Printer::log("Error, could not create render target view.", ELL_ERROR);
        return false;
    }
    backBuffer->Release();

    // creating depth buffer
    DefaultDepthBuffer = createDepthStencilView(Params.WindowSize);

    // set exposed data
    ExposedData.D3D11.D3DDev11 = Device.Get();
    ExposedData.D3D11.SwapChain = SwapChain.Get();
    ExposedData.D3D11.HWnd = hwnd;

    // init states description
    if (!BlendStateChanged)
        BlendStateChanged = true;

    if (!RasterizerStateChanged)
        RasterizerStateChanged = true;

    if (!DepthStencilStateChanged)
        DepthStencilStateChanged = true;

    BlendDesc.reset();
    RasterizerDesc.reset();
    DepthStencilDesc.reset();

    for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
    {
        SamplerStateChanged[i] = true;
        SamplerDesc[i].reset();
    }

    // Init dynamic buffers
    reallocateDynamicBuffers(100 * sizeof(S3DVertex2TCoords), 100 * sizeof(short));

    // With all DX 11 objects created, init driver states

    // Only enable multisampling if needed
    disableFeature(EVDF_TEXTURE_MULTISAMPLING, true);

    // Set render targets
    setRenderTarget(0, true, true);

    // set fog mode
    //setFog(FogColor, FogType, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);
    setFog(FogColor, (E_FOG_TYPE)0, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

    ResetRenderStates = true;

    // create materials
    createMaterialRenderers();

    // clear textures
    for (u16 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
        setActiveTexture(i, 0);

    blankImage->grab();
    blankTexture = irr::Ptr<ITexture>(*createDeviceDependentTexture(blankImage, "internal_null_texture"));

//#ifdef HAVE_CSYSTEM_EXTENSION
    auto resource = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/Direct3D11;/HLSL/Default/d3d11.hlsl", true))->ToMemoryStreamReader();
	auto resquadvs = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/Direct3D11;/HLSL/Default/QuadVS.hlsl", true))->ToMemoryStreamReader();
	auto resresolveps = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/Direct3D11;/HLSL/Default/ResolvePS.hlsl", true))->ToMemoryStreamReader();
//#else
//    System::IO::StandardDataSource fileMgr;
//    auto resource = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Direct3D11/HLSL/Default/d3d11.hlsl");
//	auto resquadvs = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Direct3D11/HLSL/Default/QuadVS.hlsl");
//	auto resresolveps = fileMgr.OpenFile(_SOURCE_DIRECTORY "/Irrlicht/RenderEngines/Direct3D11/HLSL/Default/ResolvePS.hlsl");
//#endif

	{
		ShaderInitializerEntry shaderCI;

		shaderCI.AddShaderStage(resquadvs, E_SHADER_TYPES::EST_VERTEX_SHADER, "main", "vs_4_0");
		auto psEntry = shaderCI.AddShaderStage(resresolveps, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "main", "ps_4_0");

		psEntry->Macros.emplace_back();

		UINT counts[MSAA::Count] = { 1, 2, 4, 8, 16 };
		for (uint32_t i = 1, last = 0; i < MSAA::Count; i++)
		{
			std::string value = std::to_string(counts[i]);
			psEntry->Macros[0].Name = "NUM_SAMPLES";
			psEntry->Macros[0].Value = value.c_str();
			m_rtResolveShader[i].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));
            shaderCI.mShader.Reset();
		}
	}

    {
        ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main", "vs_4_0");
        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "ps_main", "ps_4_0");
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));

        irr::video::VertexDeclaration* vertDecl = GetVertexDeclaration(E_VERTEX_TYPE::EVT_STANDARD);
        m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD]->m_inputLayout = static_cast<CD3D11VertexDeclaration*>(vertDecl)->getInputLayout(m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD]);
    }

    {
        ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main_t2", "vs_4_0");
        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "ps_main", "ps_4_0");
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));

        irr::video::VertexDeclaration* vertDecl = GetVertexDeclaration(E_VERTEX_TYPE::EVT_2TCOORDS);
        m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS]->m_inputLayout = static_cast<CD3D11VertexDeclaration*>(vertDecl)->getInputLayout(m_defaultShader[E_VERTEX_TYPE::EVT_2TCOORDS]);
    }

    {
        ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main_sk", "vs_4_0");
        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "ps_main", "ps_4_0");
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));

        irr::video::VertexDeclaration* vertDecl = GetVertexDeclaration(E_VERTEX_TYPE::EVT_SKINNING);
        m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING]->m_inputLayout = static_cast<CD3D11VertexDeclaration*>(vertDecl)->getInputLayout(m_defaultShader[E_VERTEX_TYPE::EVT_SKINNING]);
    }

    {
        ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main_bt", "vs_4_0");
        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_FRAGMENT_SHADER, "ps_main", "ps_4_0");
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        shaderCI.Callback["PixelConstats"] = irr::MakePtr<IrrDefaultShaderFragmentCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_TANGENTS].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));

        irr::video::VertexDeclaration* vertDecl = GetVertexDeclaration(E_VERTEX_TYPE::EVT_TANGENTS);
        m_defaultShader[E_VERTEX_TYPE::EVT_TANGENTS]->m_inputLayout = static_cast<CD3D11VertexDeclaration*>(vertDecl)->getInputLayout(m_defaultShader[E_VERTEX_TYPE::EVT_TANGENTS]);
    }

    {
        ShaderInitializerEntry shaderCI;

        shaderCI.AddShaderStage(resource, E_SHADER_TYPES::EST_VERTEX_SHADER, "vs_main_sh", "vs_4_0");
        shaderCI.Callback["MatrixBuffer"] = irr::MakePtr<IrrDefaultShaderVertexCallBack>();
        m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW].Reset(static_cast<D3D11HLSLProgram*>(createShader(&shaderCI)));

        irr::video::VertexDeclaration* vertDecl = GetVertexDeclaration(E_VERTEX_TYPE::EVT_SHADOW);
        m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW]->m_inputLayout = static_cast<CD3D11VertexDeclaration*>(vertDecl)->getInputLayout(m_defaultShader[E_VERTEX_TYPE::EVT_SHADOW]);
    }

    delete resource;

    return true;
}

IShader * CD3D11Driver::createShader(ShaderInitializerEntry * shaderCreateInfo)
{
    irr::video::D3D11HLSLProgram* gpuProgram = shaderCreateInfo->mShader ? static_cast<irr::video::D3D11HLSLProgram*>(shaderCreateInfo->mShader.GetPtr()) : new irr::video::D3D11HLSLProgram(this);

    gpuProgram->Reset();
    for (auto stage : shaderCreateInfo->mStages)
    {
        if (!stage->DataStream)
            continue;

        const char* ShaderModel = stage->ShaderModel.c_str();
        const char* ShaderEntryPoint = stage->IsByteCode ?  nullptr : stage->EntryPoint.c_str();

        stage->DataStream->Seek(stage->DataStreamOffset, false);

        D3D_SHADER_MACRO* Macros = nullptr;
        if (!stage->Macros.empty())
        {
            Macros = new D3D_SHADER_MACRO[stage->Macros.size() + 1];
            Macros[stage->Macros.size()].Name = nullptr;
            Macros[stage->Macros.size()].Definition = nullptr;

            for (u32 i = 0; i < stage->Macros.size(); ++i)
            {
                Macros[i].Name = stage->Macros[i].Name.c_str();
                Macros[i].Definition = stage->Macros[i].Value.c_str();
            }
        }

        switch (stage->ShaderStageType)
        {
            case E_SHADER_TYPES::EST_VERTEX_SHADER:
                gpuProgram->createVertexShader(Device.Get(), stage->DataStream, ShaderEntryPoint, ShaderModel, Macros, !!shaderCreateInfo->mShader);
				DX_NAME(gpuProgram->m_vertexShader.Get(), "%s_VS", stage->DataStream->FileName.c_str());
                break;
            case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
                gpuProgram->createGeometryShader(Device.Get(), stage->DataStream, ShaderEntryPoint, ShaderModel, Macros, !!shaderCreateInfo->mShader);
				DX_NAME(gpuProgram->m_geometryShader.Get(), "%s_GS", stage->DataStream->FileName.c_str());
				break;
            case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
                gpuProgram->createPixelShader(Device.Get(), stage->DataStream, ShaderEntryPoint, ShaderModel, Macros, !!shaderCreateInfo->mShader);
				DX_NAME(gpuProgram->m_pixelShader.Get(), "%s_PS", stage->DataStream->FileName.c_str());
				break;
        }

		if (Macros)
			delete[]Macros;
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
    return shaderCreateInfo->mShader = *gpuProgram;
}

bool CD3D11Driver::SyncShaderConstant(CD3D11HardwareBuffer * HWBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
    bool needUpdateResource[EST_HIGH_LEVEL_SHADER];
    ::ZeroMemory(needUpdateResource, sizeof(needUpdateResource));

    bool haveResource[EST_HIGH_LEVEL_SHADER];
    ::ZeroMemory(haveResource, sizeof(haveResource));

    irr::video::D3D11HLSLProgram* hlsl = static_cast<irr::video::D3D11HLSLProgram*>(GetActiveShader());
    _IRR_DEBUG_BREAK_IF(!hlsl);

    for (u32 ib = 0; ib != hlsl->mBuffers.size(); ++ib)
    {
        irr::video::SConstantBuffer* cbuffer = hlsl->mBuffers[ib];

        if (cbuffer->mCallBack)
            cbuffer->mCallBack->OnSetConstants(cbuffer, HWBuffer ? HWBuffer->GetBuffer() : nullptr, mesh, node);

        E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
        s8* dataBuffer;
        u32 dataBufferSize;
        u32 changeId = cbuffer->getChangedID();

        CD3D11HardwareBuffer * constantBuffer;
        if (HWBuffer && HWBuffer->GetBuffer() && HWBuffer->GetBuffer()->GetShaderConstantBuffers() && HWBuffer->GetBuffer()->GetShaderConstantBuffers()->size() > ib)
        {
            SConstantBuffer* bufferCache = static_cast<SConstantBuffer*>((*HWBuffer->GetBuffer()->GetShaderConstantBuffers())[cbuffer->getBindingIndex()]);
            constantBuffer = static_cast<CD3D11HardwareBuffer*>(bufferCache->mHwObject);
            changeId = bufferCache->getChangedID();
            dataBuffer = (s8*)bufferCache->mHostMemory.data();
            dataBufferSize = bufferCache->mHostMemory.size();

            if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STATIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
            else if (bufferCache->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_SYSTEM_MEMORY;

        }
        else
        {
            constantBuffer = static_cast<CD3D11HardwareBuffer*>(cbuffer->mHwObject);

            changeId = cbuffer->getChangedID();
            dataBuffer = (s8*)cbuffer->mHostMemory.data();
            dataBufferSize = cbuffer->mHostMemory.size();

            if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_DYNAMIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
            else if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STATIC)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
            else if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STREAM)
                MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_SYSTEM_MEMORY;

        }

        if (cbuffer->getHardwareMappingHint() == scene::EHM_NEVER)
            cbuffer->setHardwareMappingHint(scene::EHM_STATIC);

        {
            if (constantBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS) != changeId)
            {
                if (constantBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, MemoryAccess, dataBuffer,
                    dataBufferSize, 0, dataBufferSize))
                {
                    //cbuffer->ChangeStartOffset = cbuffer->DataBuffer.size();
                    //cbuffer->ChangeEndOffset = 0;
                    constantBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, changeId);
                }
            }
        }

        u32 minSize = std::max(cbuffer->getBindingIndex(), (u32)0);
        if ( BindedBuffers[cbuffer->mShaderStage].size() <= (int)minSize )
        {
            for ( s32 i = 0; i <= (((s32)minSize - (s32)BindedBuffers[cbuffer->mShaderStage].size()) + 1); ++i )
                BindedBuffers[cbuffer->mShaderStage].push_back(nullptr);
        }

        haveResource[cbuffer->mShaderStage] = true;
        if ( BindedBuffers[cbuffer->mShaderStage][cbuffer->getBindingIndex()] != constantBuffer->getBufferResource(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS))
        {
            needUpdateResource[cbuffer->mShaderStage] = true;
            BindedBuffers[cbuffer->mShaderStage][cbuffer->getBindingIndex()] = constantBuffer->getBufferResource(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS);
        }
    }

    if ( needUpdateResource[EST_VERTEX_SHADER] && BindedBuffers[EST_VERTEX_SHADER].size() )
        ImmediateContext->VSSetConstantBuffers(0, BindedBuffers[EST_VERTEX_SHADER].size(), BindedBuffers[EST_VERTEX_SHADER].pointer());
    else if ( !haveResource[EST_VERTEX_SHADER] && BindedBuffers[EST_VERTEX_SHADER].size())
    {
        ::ZeroMemory(BindedBuffers[EST_VERTEX_SHADER].pointer(), BindedBuffers[EST_VERTEX_SHADER].size() * sizeof(ID3D11Buffer*));
        ImmediateContext->VSSetConstantBuffers(0, BindedBuffers[EST_VERTEX_SHADER].size(), BindedBuffers[EST_VERTEX_SHADER].pointer());
        BindedBuffers[EST_VERTEX_SHADER].set_used(0);
    }
    if ( needUpdateResource[EST_GEOMETRY_SHADER] && BindedBuffers[EST_GEOMETRY_SHADER].size() )
        ImmediateContext->GSSetConstantBuffers(0, BindedBuffers[EST_GEOMETRY_SHADER].size(), BindedBuffers[EST_GEOMETRY_SHADER].pointer());
    else if ( !haveResource[EST_GEOMETRY_SHADER] && BindedBuffers[EST_GEOMETRY_SHADER].size() )
    {
        ::ZeroMemory(BindedBuffers[EST_GEOMETRY_SHADER].pointer(), BindedBuffers[EST_GEOMETRY_SHADER].size() * sizeof(ID3D11Buffer*));
        ImmediateContext->GSSetConstantBuffers(0, BindedBuffers[EST_GEOMETRY_SHADER].size(), BindedBuffers[EST_GEOMETRY_SHADER].pointer());
        BindedBuffers[EST_GEOMETRY_SHADER].set_used(0);
    }
    if ( needUpdateResource[EST_FRAGMENT_SHADER] && BindedBuffers[EST_FRAGMENT_SHADER].size() )
        ImmediateContext->PSSetConstantBuffers(0, BindedBuffers[EST_FRAGMENT_SHADER].size(), BindedBuffers[EST_FRAGMENT_SHADER].pointer());
    else if ( !haveResource[EST_FRAGMENT_SHADER] && BindedBuffers[EST_FRAGMENT_SHADER].size() )
    {
        ::ZeroMemory(BindedBuffers[EST_FRAGMENT_SHADER].pointer(), BindedBuffers[EST_FRAGMENT_SHADER].size() * sizeof(ID3D11Buffer*));
        ImmediateContext->PSSetConstantBuffers(0, BindedBuffers[EST_FRAGMENT_SHADER].size(), BindedBuffers[EST_FRAGMENT_SHADER].pointer());
        BindedBuffers[EST_FRAGMENT_SHADER].set_used(0);
    }
    return true;
}

void CD3D11Driver::useShader(IShader* gpuProgram)
{
    if (gpuProgram)
    {
        if ( gpuProgram->getShaderType() == ESV_HLSL_HIGH_LEVEL )
        {
            irr::video::D3D11HLSLProgram* hlsl = static_cast<irr::video::D3D11HLSLProgram*>(GetActiveShader());
            // Set the vertex input layout.
            //if ( hlsl->GetInputLayout() != CurrentInputLayout )
            //{
            //    CurrentInputLayout = hlsl->GetInputLayout();
            //    ImmediateContext->IASetInputLayout(hlsl->GetInputLayout());
            //}

            if ( hlsl->m_vertexShader )
            {
                shaderPtr[EST_VERTEX_SHADER] = hlsl->m_vertexShader.Get();
                ImmediateContext->VSSetShader(hlsl->m_vertexShader.Get(), nullptr, 0);
            }
            else if ( shaderPtr[EST_VERTEX_SHADER] )
            {
                shaderPtr[EST_VERTEX_SHADER] = nullptr;
                ImmediateContext->VSSetShader(nullptr, nullptr, 0);
            }

            if ( hlsl->m_hullShader )
            {
                shaderPtr[EST_HULL_SHADER] = hlsl->m_hullShader.Get();
                ImmediateContext->HSSetShader(hlsl->m_hullShader.Get(), nullptr, 0);
            }
            else if ( shaderPtr[EST_HULL_SHADER] )
            {
                shaderPtr[EST_HULL_SHADER] = nullptr;
                ImmediateContext->HSSetShader(nullptr, nullptr, 0);
            }

            if ( hlsl->m_domainShader )
            {
                shaderPtr[EST_DOMAIN_SHADER] = hlsl->m_domainShader.Get();
                ImmediateContext->DSSetShader(hlsl->m_domainShader.Get(), nullptr, 0);
            }
            else if ( shaderPtr[EST_DOMAIN_SHADER] )
            {
                shaderPtr[EST_DOMAIN_SHADER] = nullptr;
                ImmediateContext->DSSetShader(nullptr, nullptr, 0);
            }

            if ( hlsl->m_geometryShader )
            {
                shaderPtr[EST_GEOMETRY_SHADER] = hlsl->m_geometryShader.Get();
                ImmediateContext->GSSetShader(hlsl->m_geometryShader.Get(), nullptr, 0);
            }
            else if ( shaderPtr[EST_GEOMETRY_SHADER] )
            {
                shaderPtr[EST_GEOMETRY_SHADER] = nullptr;
                ImmediateContext->GSSetShader(nullptr, nullptr, 0);
            }

            if ( hlsl->m_pixelShader )
            {
                shaderPtr[EST_FRAGMENT_SHADER] = hlsl->m_pixelShader.Get();
                ImmediateContext->PSSetShader(hlsl->m_pixelShader.Get(), nullptr, 0);
            }
            else if ( shaderPtr[EST_FRAGMENT_SHADER] )
            {
                shaderPtr[EST_FRAGMENT_SHADER] = nullptr;
                ImmediateContext->PSSetShader(nullptr, nullptr, 0);
            }
        }
        else
        {
            _IRR_DEBUG_BREAK_IF(true);
        }
    }
    else
    {
        //ImmediateContext->IASetInputLayout(nullptr);
        //ImmediateContext->VSSetShader(nullptr, nullptr, 0);
        //ImmediateContext->DSSetShader(nullptr, nullptr, 0);
        //ImmediateContext->HSSetShader(nullptr, nullptr, 0);
        //ImmediateContext->GSSetShader(nullptr, nullptr, 0);
        //ImmediateContext->PSSetShader(nullptr, nullptr, 0);
    }
}

void CD3D11Driver::HandleDeviceLost()
{
    for (D3D11DeviceResource* source : ResourceList)
    {
        source->OnDeviceLost(this);
    }

    ReleaseDriver();
    initDriver((HWND)ExposedData.D3D11.HWnd, false);
    reset();

    for (D3D11DeviceResource* source : ResourceList)
    {
        source->OnDeviceRestored(this);
    }
}

ID3D11BlendState* CD3D11Driver::getBlendState()
{
    // Blend state
    if (!BlendState || BlendStateChanged /*lastBlendDesc != BlendDesc*/ )
    {
        //std::map<SD3D11_BLEND_DESC, ID3D11BlendState*>::const_iterator iBlend = BlendStateCache.find(BlendDesc);
        //if ( iBlend != BlendStateCache.end() )
        //{
        //    BlendState = iBlend->second;
        //    lastBlendDesc = BlendDesc;
        //}
        //else
        //{
            HRESULT hr = Device->CreateBlendState(&BlendDesc, &BlendState);
            if ( FAILED(hr) )
            {
                //printf("[HLSL SHADER] Error: %s error description: %s\n",
                //    DXGetErrorString(hr), DXGetErrorDescription(hr));
                os::Printer::log("Error, failed to create blend state", ELL_ERROR);
                throw std::runtime_error("Error, failed to create blend state");
            }

            //BlendStateCache[BlendDesc] = BlendState;
            //lastBlendDesc = BlendDesc;
            BlendStateChanged = false;
        //}
    }

    return BlendState;
}

ID3D11RasterizerState* CD3D11Driver::getRasterizerState()
{
    // Rasterizer state
    if (!RasterizerState || RasterizerStateChanged /*lastRasterizerDesc != RasterizerDesc*/ )
    {
        //auto iBlend = RasterizerStateCache.find(RasterizerDesc);
        //if ( iBlend != RasterizerStateCache.end() )
        //{
        //    RasterizerState = iBlend->second;
        //    lastRasterizerDesc = RasterizerDesc;
        //}
        //else
        //{
            HRESULT hr = Device->CreateRasterizerState(&RasterizerDesc, &RasterizerState);
            if ( FAILED(hr) )
            {
                //printf("[Stencil] Error: %s error description: %s\n",
                //    DXGetErrorString(hr), DXGetErrorDescription(hr));
                os::Printer::log("Error, failed to create rasterizer state", ELL_ERROR);
                throw std::runtime_error("Error, failed to create rasterizer state");
            }

            //RasterizerStateCache[RasterizerDesc] = RasterizerState;
            //lastRasterizerDesc = RasterizerDesc;
            RasterizerStateChanged = false;
        //}
    }

    return RasterizerState;
}

ID3D11DepthStencilState* CD3D11Driver::getDepthStencilState()
{
    // Depth stencil state
    if ( !DepthStencilState || DepthStencilStateChanged /*lastDepthStencilDesc != DepthStencilDesc*/ )
    {
        //auto iDepthStencil = DepthStencilStateCache.find(DepthStencilDesc);
        //if ( iDepthStencil != DepthStencilStateCache.end() )
        //{
        //    DepthStencilState = iDepthStencil->second;
        //    lastDepthStencilDesc = DepthStencilDesc;
        //}
        //else
        //{
            HRESULT hr = Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
            if ( FAILED(hr) )
            {
                os::Printer::log("Error, failed to create depth stencil state", ELL_ERROR);
                throw std::runtime_error("Error, failed to create depth stencil state");
            }

            //DepthStencilStateCache[DepthStencilDesc] = DepthStencilState;
            //lastDepthStencilDesc = DepthStencilDesc;
            DepthStencilStateChanged = false;
        //}
    }

    return DepthStencilState;
}

ID3D11SamplerState* CD3D11Driver::getSamplerState(u32 idx)
{
    if ( !SamplerState[idx] || (SamplerStateChanged[idx] /*&& lastSamplerDesc[idx] != SamplerDesc[idx]*/) )
    {
        //auto iSamplerState = SamplerStateCache.find(SamplerDesc[idx]);
        //if ( iSamplerState != SamplerStateCache.end() )
        //{
        //    SamplerState[idx] = iSamplerState->second;
        //    lastSamplerDesc[idx] = SamplerDesc[idx];
        //}
        //else
        //{
            HRESULT hr = Device->CreateSamplerState(&SamplerDesc[idx], &SamplerState[idx]);
            if ( FAILED(hr) )
            {
                //printf("[Sampler] Error: %s error description: %s\n",
                //    DXGetErrorString(hr), DXGetErrorDescription(hr));
                os::Printer::log("Error, failed to create sampler state", ELL_ERROR);
                throw std::runtime_error("Error, failed to create sampler state");
            }

            //SamplerStateCache[SamplerDesc[idx]] = SamplerState[idx];
            //lastSamplerDesc[idx] = SamplerDesc[idx];
            SamplerStateChanged[idx] = false;
        //}
    }

    return SamplerState[idx];
}

//! applications must call this method before performing any rendering. returns false if failed.
bool CD3D11Driver::beginScene(bool backBuffer, bool zBuffer, SColor color,
                              const SExposedVideoData& videoData, core::rect<s32>* sourceRect)
{
    CNullDriver::beginScene(backBuffer, zBuffer, color, videoData, sourceRect);

    if( backBuffer )
    {
        SColorf fCol( color );
        FLOAT c[4] = { fCol.r, fCol.g, fCol.b, fCol.a };        // don't swizzle clear color for default back buffer
        ImmediateContext->ClearRenderTargetView( DefaultBackBuffer.Get(), c );
    }

    if( zBuffer && DefaultDepthBuffer )
    {
        ImmediateContext->ClearDepthStencilView( DefaultDepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
    }

    // For safety, unset all shader resource views
    // If some is bound to shader stages, can cause error when bound to Output Merger too
    //ID3D11ShaderResourceView* views[1] = { NULL };
    //ImmediateContext->VSSetShaderResources( 0, 1, views );
    //ImmediateContext->GSSetShaderResources( 0, 1, views );
    //ImmediateContext->PSSetShaderResources( 0, 1, views );

    return true;
}

//! applications must call this method after performing any rendering. returns false if failed.
bool CD3D11Driver::endScene()
{
    CNullDriver::endScene();

    // The application may optionally specify "dirty" or "scroll" rects to improve efficiency
    // in certain scenarios.  In this sample, however, we do not utilize those features.
    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

    HRESULT hr = S_OK;
    
    hr = SwapChain->Present1( Params.Vsync ? 1 : 0, 0 , &parameters);

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be removed.
    if (CurrentBackBuffer)
        ImmediateContext->DiscardView(CurrentBackBuffer.Get());

    // Discard the contents of the depth stencil.
    if (CurrentDepthBuffer)
        ImmediateContext->DiscardView(CurrentDepthBuffer.Get());

    // If the device was removed either by a disconnect or a driver upgrade, we
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        HandleDeviceLost();
    }
    else if( FAILED( hr ) )
    {
        DX::ThrowIfFailed(hr);
    }

    return true;
}

void CD3D11Driver::beginInstrumentEvent(const wchar_t* wlabel, const char* label, SColor color)
{
	DX_BEGIN_EVENT(wlabel)
}

void CD3D11Driver::endInstrumentEvent()
{
	DX_END_EVENT()
}

bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
{
    switch( feature )
    {
    case EVDF_TEXTURE_COMPRESSION:
        return true;

    case EVDF_MULTIPLE_RENDER_TARGETS:
        return DriverType == D3D_DRIVER_TYPE_HARDWARE;

    case EVDF_MRT_BLEND:
    case EVDF_ALPHA_TO_COVERAGE:
    case EVDF_MRT_BLEND_FUNC:    
    case EVDF_TEXTURE_NSQUARE:
    case EVDF_VERTEX_BUFFER_OBJECT:
    case EVDF_COLOR_MASK:
    case EVDF_RENDER_TO_TARGET:
    case EVDF_MULTITEXTURE:
    case EVDF_BILINEAR_FILTER:
    case EVDF_TEXTURE_NPOT:
    case EVDF_STENCIL_BUFFER:
    case EVDF_HLSL:
    case EVDF_MIP_MAP:
    case EVDF_MIP_MAP_AUTO_UPDATE:
    case EVDF_TEXTURE_MULTISAMPLING:
    case EVDF_BLEND_OPERATIONS:
    case EVDF_POLYGON_OFFSET:
        return true;

    case EVDF_VERTEX_SHADER_4_0:
    case EVDF_PIXEL_SHADER_4_0:
    case EVDF_GEOMETRY_SHADER_4_0:
    case EVDF_STREAM_OUTPUT_4_0:
        return FeatureLevel == D3D_FEATURE_LEVEL_10_0 ||
                FeatureLevel == D3D_FEATURE_LEVEL_10_1 ||
                FeatureLevel == D3D_FEATURE_LEVEL_11_0;

    case EVDF_VERTEX_SHADER_4_1:
    case EVDF_PIXEL_SHADER_4_1:
    case EVDF_GEOMETRY_SHADER_4_1:
    case EVDF_STREAM_OUTPUT_4_1:
        return FeatureLevel == D3D_FEATURE_LEVEL_10_1 ||
               FeatureLevel == D3D_FEATURE_LEVEL_11_0;

    case EVDF_VERTEX_SHADER_5_0:
    case EVDF_PIXEL_SHADER_5_0:
    case EVDF_GEOMETRY_SHADER_5_0:
    case EVDF_STREAM_OUTPUT_5_0:
    case EVDF_TESSELATION_SHADER_5_0:
        return FeatureLevel == D3D_FEATURE_LEVEL_11_0;
    
    case EVDF_COMPUTING_SHADER_4_0:
    case EVDF_COMPUTING_SHADER_4_1:
        {
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS opts;
            if (SUCCEEDED( Device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, 
                        &opts, sizeof(D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS)) ))
            {
                return opts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x != 0;
            }
            return false;
        }

    case EVDF_COMPUTING_SHADER_5_0:
        return FeatureLevel == D3D_FEATURE_LEVEL_11_0;
    }

    return false;
}

//! sets transformation
void CD3D11Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
    Transformation3DChanged = state == ETS_PROJECTION || state == ETS_VIEW || state == ETS_WORLD;
    Matrices[state] = mat;

    if ( state == ETS_PROJECTION || state == ETS_VIEW)
        Matrices[ETS_PROJECTION_VIEW] = Matrices[ETS_PROJECTION] * Matrices[ETS_VIEW];
}

bool CD3D11Driver::setActiveTexture(u32 stage, const video::ITexture* texture)
{
    if (texture && texture->getDriverType() != EDT_DIRECT3D11)
    {
        os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
        return false;
    }

    // For first stage, set NullTexture if texture is NULL
    CurrentTexture[stage] = (stage == 0 && texture == 0) ? nullptr : texture;

    return true;
}

//! sets a material
void CD3D11Driver::setMaterial(const SMaterial& material)
{
    Material = material;
    //OverrideMaterial.apply(Material);

    // set textures
    for(u16 i = 0; i < MATERIAL_MAX_TEXTURES; i ++)
    {    
        setActiveTexture(i, Material.getTexture(i));
        setTransform((E_TRANSFORMATION_STATE) ( ETS_TEXTURE_0 + i ), material.TextureLayer[i].getTextureMatrixConst());
    }
}

IRenderTarget * CD3D11Driver::addRenderTarget()
{
    CD3D11RenderTarget* renderTarget = new CD3D11RenderTarget(this);
    RenderTargets.insert(renderTarget);
    return renderTarget;
}

bool CD3D11Driver::setRenderTarget(ITexture* texture, u16 clearFlag /*= ECBF_COLOR | ECBF_DEPTH*/, SColor clearColor /*= SColor(255, 0, 0, 0)*/,
                                    f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    // check for right driver type
    if (texture && texture->getDriverType() != EDT_DIRECT3D11)
    {
        os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
        return false;
    }

    // check for valid render target
    if (texture && !texture->isRenderTarget())
    {
        os::Printer::log("Fatal Error: Tried to set a non render target texture as render target.", ELL_ERROR);
        return false;
    }

    CD3D11Texture* tex = static_cast<CD3D11Texture*>(texture);

    // Discard the contents of the depth stencil.
    //if (DefaultBackBuffer)
    //    ImmediateContext->DiscardView(DefaultBackBuffer.Get());

    // check if we should set the previous RT back
    if (tex == 0)
    {
        CurrentBackBuffer = DefaultBackBuffer;
        CurrentDepthBuffer = DefaultDepthBuffer;
        ImmediateContext->OMSetRenderTargets( 1, CurrentBackBuffer.GetAddressOf(), CurrentDepthBuffer.Get());
        CurrentRendertargetSize = core::dimension2d<u32>(0,0);
    }
    else
    {
        CurrentBackBuffer = tex->getRenderTargetView();
        CurrentDepthBuffer = tex->DepthSurface->Surface;
        ImmediateContext->OMSetRenderTargets( 1, CurrentBackBuffer.GetAddressOf(), CurrentDepthBuffer.Get());
        CurrentRendertargetSize = tex->getSize();
    }

    // clear views
    if (clearFlag & ECBF_COLOR)
    {
        SColorf fCol(clearColor);
        // swizzle clear color is texture is passed
        // i.e.: is is default render target, don't swizzle
        FLOAT c[4] = { fCol.r, 
                       fCol.g, 
                       fCol.b,
                       fCol.a };
        ImmediateContext->ClearRenderTargetView( CurrentBackBuffer.Get(), c );
    }

    // clear depth
    if (clearFlag & (ECBF_DEPTH | ECBF_STENCIL))
    {
        u16 _dxClearFlags = 0;
        if (clearFlag & ECBF_DEPTH)
            _dxClearFlags |= D3D11_CLEAR_DEPTH;
        if (clearFlag & ECBF_STENCIL)
            _dxClearFlags |= D3D11_CLEAR_STENCIL;

        if (CurrentDepthBuffer)
            ImmediateContext->ClearDepthStencilView(CurrentDepthBuffer.Get(), _dxClearFlags, clearDepth, clearStencil);
    }

    // set blend
    if (BlendDesc.IndependentBlendEnable)
    {
        if (!BlendStateChanged)
            BlendStateChanged = true;

        BlendDesc.IndependentBlendEnable = FALSE;
    }

    // don't forget to set viewport
    setViewPort(core::rect<s32>(0, 0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height));

    return true;
}

bool CD3D11Driver::setRenderTargetEx(IRenderTarget* target, u16 clearFlag, SColor clearColor /*= SColor(255, 0, 0, 0)*/, f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/, core::array<core::recti>* scissors)
{
	u32 i;

	// Resolve MSAA
	if (ActiveRenderTarget && !target)
	{
		DX_BEGIN_EVENT(L"Resolve");

		// max number of render targets is 8 for DX11 feature level
		u32 maxMultipleRTTs = core::min_((u32)D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, ActiveRenderTarget->getTexture().size());

		auto prevShader = GetActiveShader();

		u32 msaa = 0;
		for (i = 0; i < maxMultipleRTTs; ++i)
		{
			CD3D11Texture* dxTex = static_cast<CD3D11Texture*>(ActiveRenderTarget->getTexture()[i]);
			if (!dxTex || dxTex->SampleCount <= 1)
				continue;

			u32 _current_msaa = DirectXUtil::ToMSAA(dxTex->SampleCount);
			if (msaa != _current_msaa)
			{
				msaa = _current_msaa;
				Material.MaterialType = E_MATERIAL_TYPE::EMT_SOLID;
				Material.BlendOperation = E_BLEND_OPERATION::EBO_NONE;
				Material.ColorMask = 0xF;

				Material.IsCCW = false;
				Material.ScissorEnable = scissors && !scissors->empty();
				Material.Wireframe = false;
				Material.ZWriteEnable = false;
				Material.DepthClipEnable = true;
				Material.FrontfaceCulling = false;
				Material.BackfaceCulling = false;
				Material.ZBuffer = irr::video::E_COMPARISON_FUNC::ECFN_NEVER;

				Material.StencilTest = false;
				Material.StencilFront.Mask = 0xFF;
				Material.StencilFront.WriteMask = 0xFF;
				Material.StencilFront.Reference = 0;

				setRenderStates3DMode();
				SetShader(nullptr);
				InitDrawStates(nullptr);

				ImmediateContext->IASetInputLayout(nullptr);
				_IRR_DEBUG_BREAK_IF(msaa >= MSAA::Enum::Count);
				SetShader(m_rtResolveShader[msaa]);
			}

			ImmediateContext->OMSetRenderTargets(1, dxTex->RTTextureView.GetAddressOf(), 0);
			ImmediateContext->PSSetShaderResources(0, 1, dxTex->SRMSAAView.GetAddressOf());

			if (scissors)
			{
				for (uint32_t i = 0; i < scissors->size(); i++)
				{
					setScissorRect((*scissors)[i]);
					ImmediateContext->Draw(3, 0);
				}
			}
			else
			{
				ImmediateContext->Draw(3, 0);
			}

			// clear views
			if (clearFlag& ECBF_COLOR)
			{
				SColorf fCol(clearColor);
				// swizzle clear color is texture is passed
				// i.e.: is is default render target, don't swizzle
				FLOAT c[4] = { fCol.r,
							   fCol.g,
							   fCol.b,
							   fCol.a };
				ImmediateContext->ClearRenderTargetView(dxTex->RTTextureView.Get(), c);
			}
		}


		if (clearFlag& (ECBF_DEPTH | ECBF_STENCIL) && ActiveRenderTarget->getDepthStencil())
		{
			u16 _dxClearFlags = 0;
			if (clearFlag& ECBF_DEPTH)
				_dxClearFlags |= D3D11_CLEAR_DEPTH;
			if (clearFlag& ECBF_STENCIL)
				_dxClearFlags |= D3D11_CLEAR_STENCIL;

			CD3D11Texture* depthTex = static_cast<CD3D11Texture*>(ActiveRenderTarget->getDepthStencil());
			if (depthTex->DepthSurface&& depthTex->DepthSurface->Surface)
				ImmediateContext->ClearDepthStencilView(depthTex ->DepthSurface->Surface.Get(), _dxClearFlags, clearDepth, clearStencil);
		}

		ActiveRenderTarget = nullptr;
		views[0] = nullptr;
		ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
		SetShader(prevShader);

		DX_END_EVENT();
	}

    // If no targets, set default render target
    if (!target || target->getTexture().size() == 0)
    {        
        // set default render target
        bool result = setRenderTarget(nullptr, clearFlag, clearColor, clearDepth, clearStencil);
		DX_END_EVENT();
		return result;
    }

	DX_BEGIN_EVENT(L"RTT");

    CD3D11RenderTarget* dxRTT = static_cast<CD3D11RenderTarget*>(target);

	// max number of render targets is 8 for DX11 feature level
	u32 maxMultipleRTTs = core::min_((u32)D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, target->getTexture().size());

    // validation
    for (i = 0; i < maxMultipleRTTs; ++i)
    {
        CD3D11Texture* dxTex = static_cast<CD3D11Texture*>(dxRTT->getTexture()[i]);
        if (!dxTex)
        {
            maxMultipleRTTs = i;
            os::Printer::log("Missing texture for MRT.", ELL_WARNING);
            break;
        }
    
        // check for right driver type
        if (dxTex->getDriverType() != EDT_DIRECT3D11)
        {
            maxMultipleRTTs = i;
            os::Printer::log("Tried to set a texture not owned by this driver.", ELL_WARNING);
            break;
        }
    
        // check for valid render target
        if (!dxTex->isRenderTarget())
        {
            maxMultipleRTTs = i;
            os::Printer::log("Tried to set a non render target texture as render target.", ELL_WARNING);
            break;
        }
    
        // check for valid size
        if (static_cast<CD3D11Texture*>(dxRTT->getTexture()[0])->getSize() != dxTex->getSize())
        {
            maxMultipleRTTs = i;
            os::Printer::log("Render target texture has wrong size.", ELL_WARNING);
            break;
        }
    }
    if (maxMultipleRTTs==0)
    {
        os::Printer::log("Fatal Error: No valid MRT found.", ELL_ERROR);
        return false;
    }
    
    CD3D11Texture* tex = static_cast<CD3D11Texture*>(dxRTT->getTexture()[0]);
    
    ID3D11RenderTargetView* RTViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];

    for (i = 0; i < maxMultipleRTTs; ++i)
    {
        // store render target view in array
        RTViews[i] = dxRTT->getSurface(i);

        // clear views
        if (clearFlag & ECBF_COLOR)
        {
            SColorf fCol(clearColor);
            // swizzle clear color is texture is passed
            // i.e.: is is default render target, don't swizzle
            FLOAT c[4] = { fCol.r,
                           fCol.g,
                           fCol.b,
                           fCol.a };
            ImmediateContext->ClearRenderTargetView(RTViews[i], c);
        }
    }
   
    // set depth buffer
    CurrentDepthBuffer = static_cast<CD3D11Texture*>(dxRTT->getDepthStencil())->DepthSurface->Surface;

    if (clearFlag & (ECBF_DEPTH | ECBF_STENCIL))
    {
        u16 _dxClearFlags = 0;
        if (clearFlag & ECBF_DEPTH)
            _dxClearFlags |= D3D11_CLEAR_DEPTH;
        if (clearFlag & ECBF_STENCIL)
            _dxClearFlags |= D3D11_CLEAR_STENCIL;

        if (CurrentDepthBuffer)
            ImmediateContext->ClearDepthStencilView(CurrentDepthBuffer.Get(), _dxClearFlags, clearDepth, clearStencil);
    }
    
    // set targets
    ImmediateContext->OMSetRenderTargets( maxMultipleRTTs, RTViews, CurrentDepthBuffer.Get());
    
    // don't forget to set viewport
    CurrentRendertargetSize = tex->getSize();
    setViewPort(core::rect<s32>(0, 0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height));

	ActiveRenderTarget = irr::Ptr<IRenderTarget>(dxRTT);
    return true;
}

//---------------------------------------------------------------------
bool CD3D11Driver::_getErrorsFromQueue() const
{
    if (m_infoQueue != NULL)
    {
        UINT64 numStoredMessages = m_infoQueue->GetNumStoredMessages();

        for (UINT64 i = 0; i < numStoredMessages; i++)
        {
            // Get the size of the message
            SIZE_T messageLength = 0;
            m_infoQueue->GetMessage(i, NULL, &messageLength);
            // Allocate space and get the message
            D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(messageLength);
            m_infoQueue->GetMessage(i, pMessage, &messageLength);

            bool res = false;
            switch (pMessage->Severity)
            {
            case D3D11_MESSAGE_SEVERITY_CORRUPTION:
                //if (D3D_CORRUPTION == mExceptionsErrorLevel)
                //{
                //    res = true;
                //}
                break;
            case D3D11_MESSAGE_SEVERITY_ERROR:
                //switch (mExceptionsErrorLevel)
                //{
                //case D3D_INFO:
                //case D3D_WARNING:
                //case D3D_ERROR:
                //    res = true;
                //}
                break;
            case D3D11_MESSAGE_SEVERITY_WARNING:
                //switch (mExceptionsErrorLevel)
                //{
                //case D3D_INFO:
                //case D3D_WARNING:
                //    res = true;
                //}
                break;
            }

            free(pMessage);
            if (res)
            {
                // we don't need to loop anymore...
                return true;
            }

        }

        //clearStoredErrorMessages();

        return false;

    }
    else
    {
        return false;
    }
}

void CD3D11Driver::setViewPort(const core::rect<s32>& area)
{
    core::rect<s32> vp = area;
    core::rect<s32> rendert(0,0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);
    vp.clipAgainst(rendert);

    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = (FLOAT)vp.UpperLeftCorner.X;
    viewPort.TopLeftY = (FLOAT)vp.UpperLeftCorner.Y;
    viewPort.Width = (FLOAT)vp.getWidth();
    viewPort.Height = (FLOAT)vp.getHeight();
    //if (requiresTextureFlipping())
    //{
    //    // Convert "top-left" to "bottom-left"
    //    viewPort.TopLeftY = target->getHeight() - viewPort.Height - viewPort.TopLeftY;
    //}
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    ImmediateContext->RSSetViewports( 1, &viewPort );

    this->ViewPort = vp;
}

void CD3D11Driver::setScissorRect(const core::rect<s32>& _rect)
{
    D3D11_RECT rect;
    rect.left = _rect.UpperLeftCorner.X;
    rect.top = _rect.UpperLeftCorner.Y;
    rect.right = _rect.LowerRightCorner.X;
    rect.bottom = _rect.LowerRightCorner.Y;
    ImmediateContext->RSSetScissorRects(1, &rect);
}

const core::rect<s32>& CD3D11Driver::getViewPort() const
{
    return ViewPort;
}

bool CD3D11Driver::updateVertexHardwareBuffer(CD3D11HardwareBuffer *hwBuffer, E_HARDWARE_BUFFER_TYPE Type)
{
    if ( !hwBuffer )
        return false;

    E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;

    const scene::IMeshBuffer* mb = hwBuffer->GetBuffer();
    const E_VERTEX_TYPE vType = mb->getVertexType();

    if ( Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX )
    {
        if (hwBuffer->GetBuffer()->getHardwareMappingHint_Vertex() == scene::EHM_DYNAMIC)
            MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

        const void* vertices = mb->getVertices();
        const u32 vertexCount = mb->getVertexCount();
        const u32 vertexSize = hwBuffer->GetBuffer()->GetVertexStride() ? hwBuffer->GetBuffer()->GetVertexStride() : getVertexPitchFromType(vType);
        const u32 bufSize = vertexSize * vertexCount;

        hwBuffer->UpdateBuffer(Type, MemoryAccess, vertices, bufSize);
    }
    else if ( Type == E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM )
    {
        irr::scene::IStreamBuffer * streamBuffer = mb->getStreamBuffer();
        const void* vertices = streamBuffer->getData();
        const u32 vertexCount = streamBuffer->size();
        const u32 vertexSize = streamBuffer->stride();
        const u32 vertexBufSize = vertexSize * vertexCount;

        if (hwBuffer->GetBuffer()->getHardwareMappingHint_Instance() == scene::EHM_DYNAMIC)
            MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

        hwBuffer->UpdateBuffer(Type, MemoryAccess, vertices, vertexBufSize);
    }
    return true;
}

bool CD3D11Driver::updateIndexHardwareBuffer(CD3D11HardwareBuffer* hwBuffer)
{
    if ( !hwBuffer )
        return false;

    const scene::IMeshBuffer* mb = hwBuffer->GetBuffer();
    const u16* indices = mb->getIndices();
    const u32 indexCount = mb->getIndexCount();
    const u32 indexStride = video::getIndexSize(mb->getIndexType());
    const u32 bufSize = indexStride * indexCount;

    E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
    if ( hwBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_DYNAMIC)
        MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;

    hwBuffer->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, MemoryAccess, indices, bufSize);
    return true;
}

bool CD3D11Driver::updateHardwareBuffer(IHardwareBuffer *hwBuffer)
{
    if ( !hwBuffer )
        return false;

    //if ( !hwBuffer->GetBuffer()->GetGPUProgram() )
    //{
    //    hwBuffer->GetBuffer()->SetGPUProgram(m_defaultShader[hwBuffer->GetBuffer()->getVertexType()]);
    //    _IRR_DEBUG_BREAK_IF(!hwBuffer->GetBuffer()->GetGPUProgram());
    //}

    if ( hwBuffer->GetBuffer()->getHardwareMappingHint_Vertex() == scene::EHM_NEVER || hwBuffer->GetBuffer()->getHardwareMappingHint_Index() == scene::EHM_NEVER )
        hwBuffer->GetBuffer()->setHardwareMappingHint(scene::EHM_DYNAMIC);

    {
        if ( hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX) != hwBuffer->GetBuffer()->getChangedID_Vertex() )
        {
            if ( !updateVertexHardwareBuffer((CD3D11HardwareBuffer*)hwBuffer, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX) )
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX, hwBuffer->GetBuffer()->getChangedID_Vertex());
        }

        if ( hwBuffer->GetBuffer()->getStreamBuffer() && hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM) != hwBuffer->GetBuffer()->getStreamBuffer()->getChangedID() )
        {
            if ( !updateVertexHardwareBuffer((CD3D11HardwareBuffer*)hwBuffer, E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM) )
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_VERTEX_INSTANCE_STREAM, hwBuffer->GetBuffer()->getStreamBuffer()->getChangedID());
        }
    }

    {
        if ( hwBuffer->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX) != hwBuffer->GetBuffer()->getChangedID_Index() )
        {
            if ( !updateIndexHardwareBuffer((CD3D11HardwareBuffer*)hwBuffer) )
                return false;

            hwBuffer->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_INDEX, hwBuffer->GetBuffer()->getChangedID_Index());
        }
    }

    return true;
}

IHardwareBuffer* CD3D11Driver::createHardwareBuffer(const scene::IMeshBuffer* mb)
{
#ifdef _DEBUG
    if (!STATHREADiD)
        STATHREADiD = GetCurrentThreadId();
    assert(STATHREADiD == GetCurrentThreadId());
#endif

    if ( !mb )
        return 0;

    CD3D11HardwareBuffer *hwBuffer = new CD3D11HardwareBuffer(this, (scene::IMeshBuffer*)mb,
        mb->getIndexType() == video::EIT_16BIT ?
        (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_16_BITS : (u32)E_HARDWARE_BUFFER_FLAGS::EHBF_INDEX_32_BITS, mb->getVertexType());

    if (!mb->GetVertexDeclaration())
        mb->SetVertexDeclaration(this->GetVertexDeclaration(E_VERTEX_TYPE::EVT_STANDARD));

    if ( !updateHardwareBuffer(hwBuffer) )
    {
        deleteHardwareBuffer(hwBuffer);
        return 0;
    }

    hwBuffer->Initialize();
    return hwBuffer;
}

void CD3D11Driver::deleteHardwareBuffer(IHardwareBuffer *_HWBuffer)
{
    if (!_HWBuffer)
        return;
    
    CNullDriver::deleteHardwareBuffer(_HWBuffer);
}

void CD3D11Driver::InitDrawStates(const scene::IMeshBuffer * mb, video::E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
    // Bind textures and samplers
    bool viewChanged = false;
    bool stateChanged = false;
    for (int i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        if (CurrentTexture[i])
        {
            ID3D11ShaderResourceView* view = static_cast<const CD3D11Texture*>(CurrentTexture[i])->getShaderResourceView();
            viewChanged |= views[i] != view;
            if (viewChanged)
                views[i] = view;

            ID3D11SamplerState* state = getSamplerState(i);
            stateChanged |= samplers[i] != state;
            if (stateChanged)
                samplers[i] = state;
            
        }
        else
        {
            ID3D11ShaderResourceView* view = static_cast<CD3D11Texture*>(blankTexture.GetPtr())->getShaderResourceView();
            viewChanged |= views[i] != view;
            if (viewChanged)
                views[i] = view;

            ID3D11SamplerState* state = getSamplerState(i);
            stateChanged |= samplers[i] != state;
            if (stateChanged)
                samplers[i] = state;
        }
    }

    // Set the vertex input layout.
    if (mb)
    {
        // Performance Note: Use dedicate layout if have
        if (mb->GetHWBuffer() && static_cast<CD3D11HardwareBuffer*>(mb->GetHWBuffer())->GetInputLayout())
        {
            auto inputLayout = static_cast<CD3D11HardwareBuffer*>(mb->GetHWBuffer())->GetInputLayout();
            if (inputLayout != CurrentInputLayout.Get())
            {
                CurrentInputLayout = inputLayout;
                ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
            }
        }
        else
        {
			D3D11HLSLProgram* hlsl = static_cast<D3D11HLSLProgram*>(GetActiveShader());
			if (hlsl)
			{
				if (!hlsl->GetInputLayout())
				{
					CD3D11VertexDeclaration* decl = static_cast<CD3D11VertexDeclaration*>(GetVertexDeclaration(mb->getVertexType()));
					if (decl)
					{
						hlsl->m_inputLayout = decl->getInputLayout(hlsl);
					}
				}

				if (hlsl->GetInputLayout() != CurrentInputLayout.Get())
				{
					CurrentInputLayout = hlsl->GetInputLayout();
					ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
				}
			}

			//{
			//	auto inputLayout = static_cast<CD3D11VertexDeclaration*>(mb->GetVertexDeclaration())->getInputLayout(static_cast<D3D11HLSLProgram*>(GetActiveShader()));
			//	if (inputLayout != CurrentInputLayout.Get())
			//	{
			//		CurrentInputLayout = inputLayout;
			//		ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
			//	}
			//}
        }
    }
    else
    {
		D3D11HLSLProgram* hlsl = static_cast<D3D11HLSLProgram*>(GetActiveShader());
		if (hlsl)
		{
			if (!hlsl->GetInputLayout())
			{
				CD3D11VertexDeclaration* decl = static_cast<CD3D11VertexDeclaration*>(GetVertexDeclaration(vType));
				if (decl)
				{
					hlsl->m_inputLayout = decl->getInputLayout(hlsl);
				}
			}

			if (hlsl->GetInputLayout() != CurrentInputLayout.Get())
			{
				CurrentInputLayout = hlsl->GetInputLayout();
				ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
			}
		}
    }

    if (viewChanged)
        ImmediateContext->PSSetShaderResources(0, MATERIAL_MAX_TEXTURES, views);

    if ( stateChanged )
        ImmediateContext->PSSetSamplers(0, MATERIAL_MAX_TEXTURES, samplers);

    // Bind depth-stencil view
    ID3D11DepthStencilState* DepthStencilState = getDepthStencilState();
    if ( LastDepthStencilState.Get() != DepthStencilState )
    {
        LastDepthStencilState = DepthStencilState;
        ImmediateContext->OMSetDepthStencilState(DepthStencilState, Material.StencilFront.Reference);
    }

    // Bind blend state
    ID3D11BlendState* BlendStateState = getBlendState();
    if ( LastBlendState.Get() != BlendStateState )
    {
        LastBlendState = BlendStateState;
        ImmediateContext->OMSetBlendState(BlendStateState, 0, 0xffffffff);
    }

    // Bind rasterizer state
    ID3D11RasterizerState* RasterizerState = getRasterizerState();
    if ( LastRasterizerState.Get() != RasterizerState )
    {
        LastRasterizerState = RasterizerState;
        ImmediateContext->RSSetState(RasterizerState);
    }

    // finally, draw
    D3D_PRIMITIVE_TOPOLOGY Topology = DirectXUtil::getTopology(mb ? mb->getRenderPrimitive() : pType);
    if ( lastTopology != Topology )
    {
        lastTopology = Topology;
        ImmediateContext->IASetPrimitiveTopology(Topology);
    }
}

void CD3D11Driver::drawMeshBuffer(const scene::IMeshBuffer * mb, scene::IMesh * mesh, scene::ISceneNode * node)
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

#ifdef _DEBUG
    if (!STATHREADiD)
        STATHREADiD = GetCurrentThreadId();
    assert(STATHREADiD == GetCurrentThreadId());
#endif

    if (!mb->GetGPUProgram())
    {
        SetShader(m_defaultShader[mb->getVertexType()]);
        _IRR_DEBUG_BREAK_IF(!GetActiveShader());

        //bool shaderOK = MaterialRenderers[Material.MaterialType].Renderer->OnRender(this, mb->getVertexType());
        //_IRR_DEBUG_BREAK_IF(!shaderOK);
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

    blockgpuprogramchange = true;

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
    }

    blockgpuprogramchange = false;

#ifdef _DEBUG
    if (!checkPrimitiveCount(mb->getVertexCount()))
    {
        if (!HWBuffer->IsManualBind())
            HWBuffer->Unbind();
        return;
    }
#endif

    SyncShaderConstant(static_cast<CD3D11HardwareBuffer*>(HWBuffer), mesh, node);
    InitDrawStates(mb);

    u32 instanceCount = (!mesh || mesh->IsInstanceModeAvailable()) && mb->getStreamBuffer() ? mb->getStreamBuffer()->size() : 1;

    if (instanceCount > 1)
    {
        if (mb->GetSubBufferCount())
            ImmediateContext->DrawIndexedInstanced(mb->GetIndexRangeCount(), instanceCount, mb->GetIndexRangeStart(), mb->GetBaseVertexLocation(), 0);
        else
            ImmediateContext->DrawIndexedInstanced(mb->getIndexCount(), instanceCount, 0, 0, 0);
    }
    else
    {
        if (mb->GetSubBufferCount())
            ImmediateContext->DrawIndexed(mb->GetIndexRangeCount(), mb->GetIndexRangeStart(), mb->GetBaseVertexLocation());
        else
            ImmediateContext->DrawIndexed(mb->getIndexCount(), 0, 0);
    }

    if (HWBuffer && !HWBuffer->IsManualBind())
        HWBuffer->Unbind();
}

void CD3D11Driver::drawHardwareBuffer(IHardwareBuffer* _HWBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
}

void CD3D11Driver::drawHardwareBuffer(IHardwareBuffer* vertices,
                IHardwareBuffer* indices, E_VERTEX_TYPE vType, 
                scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType, u32 numInstances)
{
}

void CD3D11Driver::drawAuto(IHardwareBuffer* vertices, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
}

void CD3D11Driver::drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType)
{
    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType,iType);

    if (vertexCount || primitiveCount)
        draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount,
            vType, pType, iType, true);
}

void CD3D11Driver::draw2DVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType)
{
    if (!checkPrimitiveCount(primitiveCount))
        return;

    CNullDriver::draw2DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType,iType);

    if (vertexCount || primitiveCount)
        draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount,
            vType, pType, iType, false);
}

//Note: deprecated after rewrite draw methods will be remove this
void CD3D11Driver::draw2D3DVertexPrimitiveList(const void* vertices,
        u32 vertexCount, const void* indexList, u32 primitiveCount,
        E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
        E_INDEX_TYPE iType, bool is3D)
{
    if (pType == scene::EPT_TRIANGLE_FAN)
        return;

    if (is3D)
    {
        if (!setRenderStates3DMode())
            return;
    }

    if (vType < E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE)
        SetShader(m_defaultShader[vType]);
    _IRR_DEBUG_BREAK_IF(!GetActiveShader());

    SyncShaderConstant(nullptr);
    InitDrawStates(nullptr, vType, pType);

    // copy vertices to dynamic buffers, if needed
    if(vertices || indexList)
        uploadVertexData(vertices, vertexCount, indexList, DirectXUtil::getIndexAmount(pType, primitiveCount), vType, iType);

    if (pType == scene::EPT_POINTS || pType == scene::EPT_POINT_SPRITES || !indexList)
        ImmediateContext->Draw( vertexCount, 0 );
    else if (vertexCount == -1)
        ImmediateContext->DrawAuto();
    else
        ImmediateContext->DrawIndexed(DirectXUtil::getIndexAmount(pType, primitiveCount), 0, 0 );
}

void CD3D11Driver::drawPixel(u32 x, u32 y, const SColor & color)
{
    os::Printer::log("\"drawPixel\" method not supported in Direct3D 11", ELL_ERROR);
}

const wchar_t* CD3D11Driver::getName() const
{
    return DriverAndFeatureName.c_str(); //L"Direct3D 11.0";
}

void CD3D11Driver::deleteAllDynamicLights()
{
    RequestedLights.clear();

    CNullDriver::deleteAllDynamicLights();
}

s32 CD3D11Driver::addDynamicLight(const SLight& light)
{
    CNullDriver::addDynamicLight(light);

    RequestedLights.push_back(RequestedLight(light));

    return (s32)(RequestedLights.size() - 1);
}

void CD3D11Driver::turnLightOn(s32 lightIndex, bool turnOn)
{
    if(lightIndex < 0 || lightIndex >= (s32)RequestedLights.size())
        return;

    RequestedLight & requestedLight = RequestedLights[lightIndex];
    requestedLight.DesireToBeOn = turnOn;
}

u32 CD3D11Driver::getMaximalDynamicLightAmount() const
{
    return MaxActiveLights;
}

void CD3D11Driver::setAmbientLight(const SColorf& color)
{
    AmbientLight = color;
}

void CD3D11Driver::drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail, u32 debugDataVisible)
{
    if (!Params.Stencilbuffer)
        return;

    if (triangles.empty())
        return;

    ID3D11RenderTargetView* nullBuffer = nullptr;
    ImmediateContext->OMSetRenderTargets(1, &nullBuffer, CurrentDepthBuffer.Get());

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
    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT;
    Material.StencilFront.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;
    
    Material.StencilBack.FailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.DepthFailOp = E_STENCIL_OPERATION::ESO_KEEP;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_INCREMENT;
    Material.StencilBack.Comparsion = E_COMPARISON_FUNC::ECFN_ALWAYS;

    //RasterizerDesc.AntialiasedLineEnable = false;
    RasterizerDesc.MultisampleEnable = true;
    //RasterizerDesc.ScissorEnable = false;

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), nullptr, 0,
        E_VERTEX_TYPE::EVT_SHADOW, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    Material.FrontfaceCulling = zfail;
    Material.BackfaceCulling = !zfail;

    Material.StencilFront.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT;
    Material.StencilBack.PassOp = E_STENCIL_OPERATION::ESO_DECREMENT;

    draw2D3DVertexPrimitiveList(triangles.const_pointer(), triangles.size(), nullptr, 0,
        E_VERTEX_TYPE::EVT_SHADOW, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    CurrentRenderMode = zfail ? ERM_SHADOW_VOLUME_ZFAIL : ERM_SHADOW_VOLUME_ZPASS;
}

void CD3D11Driver::drawStencilShadow(bool clearStencilBuffer,
            video::SColor leftUpEdge, video::SColor rightUpEdge,
            video::SColor leftDownEdge, video::SColor rightDownEdge)
{
    if (!Params.Stencilbuffer)
    	return;

    ImmediateContext->OMSetRenderTargets(1, CurrentBackBuffer.GetAddressOf(), CurrentDepthBuffer.Get());

    SetShader(m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD]);
    
    S3DVertex vtx[4];
    vtx[0] = S3DVertex(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, leftUpEdge, 0.0f, 0.0f);
    vtx[1] = S3DVertex(1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, rightUpEdge, 0.0f, 1.0f);
    vtx[2] = S3DVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, leftDownEdge, 1.0f, 0.0f);
    vtx[3] = S3DVertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, rightDownEdge, 1.0f, 1.0f);
    
    s16 indices[6] = {0,1,2,1,3,2};
    
    setActiveTexture(0,0);

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

    draw2D3DVertexPrimitiveList(vtx, 4, indices, 6,
        E_VERTEX_TYPE::EVT_STANDARD, scene::E_PRIMITIVE_TYPE::EPT_TRIANGLES, E_INDEX_TYPE::EIT_16BIT, true);

    setTransform(E_TRANSFORMATION_STATE::ETS_PROJECTION, proj);
    setTransform(E_TRANSFORMATION_STATE::ETS_VIEW, view);

    Material.StencilTest = false;
    Material.StencilFront.Mask = 0x0;
    Material.StencilFront.WriteMask = 0x0;
    Material.StencilFront.Reference = 0;

    if (clearStencilBuffer && DefaultDepthBuffer)
    {
        ImmediateContext->ClearDepthStencilView(DefaultDepthBuffer.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    CurrentRenderMode = ERM_STENCIL_FILL;
}

u32 CD3D11Driver::getMaximalPrimitiveCount() const
{
    return 0x7fffffff;
}


void CD3D11Driver::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)
{
    if (flag == video::ETCF_CREATE_MIP_MAPS && !queryFeature(EVDF_MIP_MAP))
        enabled = false;

    CNullDriver::setTextureCreationFlag(flag, enabled);
}


void CD3D11Driver::setFog(SColor color, E_FOG_TYPE fogType, f32 start,
            f32 end, f32 density, bool pixelFog, bool rangeFog)
{
    // just forward method, configurations will be applyed in the material renderer
    CNullDriver::setFog(color, fogType, start, end, density, pixelFog, rangeFog);


}


void CD3D11Driver::OnResize(const core::dimension2d<u32>& size)
{
    if (!Device || !SwapChain)
        return;

    CNullDriver::OnResize(size);
    present.Width  = size.Width;
    present.Height = size.Height;

    //present.BufferDesc.Width = size.Width;
    //present.BufferDesc.Height = size.Height;
    //ScreenSize = size;

    reset();
}

//! sets the needed renderstates
bool CD3D11Driver::setRenderStates3DMode()
{
    if (!STATHREADiD)
        STATHREADiD = GetCurrentThreadId();
    assert(STATHREADiD == GetCurrentThreadId());

    if (!Device)
        return false;

    //LastVertexType = vType;

    if (CurrentRenderMode != ERM_3D)
    {
        ResetRenderStates = true;
    }

    if (ResetRenderStates || LastMaterial != Material)
    {
        // unset old material
        //if (CurrentRenderMode == ERM_3D &&
        //    LastMaterial.MaterialType != Material.MaterialType &&
        //    LastMaterial.MaterialType >= 0 && LastMaterial.MaterialType < (s32)MaterialRenderers.size())
        //    MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();
        //
        //// set new material.
        //if (Material.MaterialType >= 0 && Material.MaterialType < (s32)MaterialRenderers.size())
        //    MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(Material, LastMaterial, ResetRenderStates, this);

        irr::video::D3D11HLSLProgram* hlsl = static_cast<irr::video::D3D11HLSLProgram*>(GetActiveShader());
        if (!hlsl)
            hlsl = m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD];

        if (hlsl)
        {
            for (int i = 0; i < hlsl->mBuffers.size(); ++i)
            {
                irr::video::SConstantBuffer* cbuffer = hlsl->mBuffers[i];
                if (cbuffer->mCallBack)
                    cbuffer->mCallBack->OnSetMaterial(cbuffer, Material);
            }
        }

        setBasicRenderStates(Material, LastMaterial, ResetRenderStates);

        LastMaterial = Material;
        ResetRenderStates = false;
    }

    //bool shaderOK = true;
    //if (Material.MaterialType >= 0 && Material.MaterialType < (s32)MaterialRenderers.size())
    //    shaderOK = MaterialRenderers[Material.MaterialType].Renderer->OnRender(this, LastVertexType);

    CurrentRenderMode = ERM_3D;

    return true;
}

void CD3D11Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
{
    if (CurrentRenderMode != ERM_2D || Transformation3DChanged)
    {
        // Need rework render state change when render mode switch because required here which shader used
        SetShader(nullptr);

        // Set world to identity
        core::matrix4 m;
        setTransform(ETS_WORLD, m);

        // Adjust projection
        const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
        m.buildProjectionMatrixOrthoLH(f32(renderTargetSize.Width), f32(-(s32)(renderTargetSize.Height)), -1.0, 1.0);
        m.setTranslation(core::vector3df(-1, 1, 0));
        setTransform(ETS_PROJECTION, m);

        Transformation3DChanged = false;
    }

    // Set view to translate a little forward
    //m.setTranslation(core::vector3df(-0.5f, -0.5f, 0));
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
    Material.Lighting=false;
    Material.ZBuffer=ECFN_NEVER;
    Material.ZWriteEnable=false;

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
        irr::video::D3D11HLSLProgram* hlsl = static_cast<irr::video::D3D11HLSLProgram*>(GetActiveShader());
        if (!hlsl)
            hlsl = m_defaultShader[E_VERTEX_TYPE::EVT_STANDARD];

        if (hlsl)
        {
            for (int i = 0; i < hlsl->mBuffers.size(); ++i)
            {
                irr::video::SConstantBuffer* cbuffer = hlsl->mBuffers[i];
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

void CD3D11Driver::setBasicRenderStates(const SMaterial& material, const SMaterial& lastMaterial,
            bool resetAllRenderstates)
{
    // init states description
    //BlendDesc.reset();
    //RasterizerDesc.reset();
    //DepthStencilDesc.reset();
    //for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
    //    SamplerDesc[i].reset();

    // This needs only to be updated onresets
    //if ( Params.HandleSRGB && resetAllRenderstates )
    //    //TODO: no information

    // fillmode
    if ( resetAllRenderstates || lastMaterial.Wireframe != material.Wireframe || lastMaterial.PointCloud != material.PointCloud )
    {
        if (!RasterizerStateChanged)
            RasterizerStateChanged = true;

        if ( material.Wireframe )
            RasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
        else
            RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    }

    if (resetAllRenderstates || lastMaterial.IsCCW != material.IsCCW)
    {
        if (!RasterizerStateChanged)
            RasterizerStateChanged = true;

        RasterizerDesc.FrontCounterClockwise = material.IsCCW;
    }

    if (resetAllRenderstates || lastMaterial.ScissorEnable != material.ScissorEnable)
    {
        if (!RasterizerStateChanged)
            RasterizerStateChanged = true;

        RasterizerDesc.ScissorEnable = material.ScissorEnable;
    }

    // multisample
    //if (FeatureEnabled[EVDF_TEXTURE_MULTISAMPLING])
    //    RasterizerDesc.MultisampleEnable = true;

    // material properties (ambient, diffuse, etc, and lighthing)
    // handled in MaterialRenderers

    // shademode (Flat or Gouraud)
    // handled in MaterialRenderers

    // lighting 
    // handled in MaterialRenderers

    // fog
    // handled in MaterialRenderers

    // specular highlights
    // handled in MaterialRenderers

    // normalization
    // handled in MaterialRenderers (shader)

    // thickness
    // handled in MaterialRenderers

    // Stencil
    if (resetAllRenderstates || lastMaterial.StencilTest != material.StencilTest ||
         (*(u32*)&lastMaterial.StencilFront) != (*(u32*)&material.StencilFront) ||
         (*(u32*)&lastMaterial.StencilBack) != (*(u32*)&material.StencilBack))
    {
        if (!DepthStencilStateChanged)
            DepthStencilStateChanged = true;
    
        DepthStencilDesc.StencilEnable = material.StencilTest;
        DepthStencilDesc.StencilWriteMask = material.StencilFront.WriteMask;
        DepthStencilDesc.StencilReadMask = material.StencilFront.Mask;
    
        DepthStencilDesc.FrontFace.StencilFailOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilFront.FailOp);
        DepthStencilDesc.FrontFace.StencilDepthFailOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilFront.DepthFailOp);
        DepthStencilDesc.FrontFace.StencilPassOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilFront.PassOp);
        DepthStencilDesc.FrontFace.StencilFunc = DirectXUtil::getDepthFunction((E_COMPARISON_FUNC)material.StencilFront.Comparsion);
    
        DepthStencilDesc.BackFace.StencilFailOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilBack.FailOp);
        DepthStencilDesc.BackFace.StencilDepthFailOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilBack.DepthFailOp);
        DepthStencilDesc.BackFace.StencilPassOp = DirectXUtil::getStencilOp((E_STENCIL_OPERATION)material.StencilBack.PassOp);
        DepthStencilDesc.BackFace.StencilFunc = DirectXUtil::getDepthFunction((E_COMPARISON_FUNC)material.StencilBack.Comparsion);
    }

    if (resetAllRenderstates || lastMaterial.DepthClipEnable != material.DepthClipEnable)
    {
        if (!DepthStencilStateChanged)
            DepthStencilStateChanged = true;

        RasterizerDesc.DepthClipEnable = material.DepthClipEnable;
    }

    // zbuffer
    if ( resetAllRenderstates || (lastMaterial.ZBuffer != material.ZBuffer) )
    {
        if (!DepthStencilStateChanged)
            DepthStencilStateChanged = true;

        DepthStencilDesc.DepthEnable = (material.ZBuffer == ECFN_NEVER) ? false : true;
        DepthStencilDesc.DepthFunc = DirectXUtil::getDepthFunction((E_COMPARISON_FUNC)material.ZBuffer);
    }

    // zwrite
    if ( resetAllRenderstates || (lastMaterial.ZWriteEnable != material.ZWriteEnable) )
    {
        if (!DepthStencilStateChanged)
            DepthStencilStateChanged = true;

        // zwrite
        if ( material.ZWriteEnable && (AllowZWriteOnTransparent || !material.isTransparent()) )
            DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        else
            DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    }

    // back face culling
    if ( resetAllRenderstates || (lastMaterial.FrontfaceCulling != material.FrontfaceCulling) || (lastMaterial.BackfaceCulling != material.BackfaceCulling) )
    {
        if (!RasterizerStateChanged)
            RasterizerStateChanged = true;

        if ( material.FrontfaceCulling && !material.Wireframe )
            RasterizerDesc.CullMode = D3D11_CULL_FRONT;
        else if ( material.BackfaceCulling && !material.Wireframe )
            RasterizerDesc.CullMode = D3D11_CULL_BACK;
        else
            RasterizerDesc.CullMode = D3D11_CULL_NONE;
    }

    // Color Mask
    if ( queryFeature(EVDF_COLOR_MASK) &&
        (resetAllRenderstates || ResetBlending || lastMaterial.ColorMask != material.ColorMask) )
    {
        // multiple color masks and independent blend will be handled in setRenderTarget
        const UINT8 flag =
            ((material.ColorMask & ECP_RED) ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
            ((material.ColorMask & ECP_GREEN) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
            ((material.ColorMask & ECP_BLUE) ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
            ((material.ColorMask & ECP_ALPHA) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);

        if (!BlendStateChanged)
            BlendStateChanged = true;

        BlendDesc.RenderTarget[0].RenderTargetWriteMask = flag;
    }

    // Anti Aliasing
    if ( AlphaToCoverageSupport && (resetAllRenderstates || (lastMaterial.AntiAliasing != material.AntiAliasing)) )
    {
        if (!BlendStateChanged)
            BlendStateChanged = true;

        if ( (material.AntiAliasing & EAAM_ALPHA_TO_COVERAGE) )
            BlendDesc.AlphaToCoverageEnable = TRUE;
        else if (lastMaterial.AntiAliasing & EAAM_ALPHA_TO_COVERAGE)
            BlendDesc.AlphaToCoverageEnable = FALSE;
    }

    if (resetAllRenderstates || lastMaterial.MaterialType != material.MaterialType || lastMaterial.BlendOperation != material.BlendOperation || lastMaterial.uMaterialTypeParam != material.uMaterialTypeParam)
    {
        if (!BlendStateChanged)
            BlendStateChanged = true;


        bool blendEnabled = material.BlendOperation != E_BLEND_OPERATION::EBO_NONE;
        E_BLEND_FACTOR srcFact = E_BLEND_FACTOR::EBF_ONE, dstFact = E_BLEND_FACTOR::EBF_ZERO, srcFactAlpha = E_BLEND_FACTOR::EBF_ONE, dstFactAlpha = E_BLEND_FACTOR::EBF_ZERO;
        E_BLEND_OPERATION blendOp = E_BLEND_OPERATION::EBO_ADD, blendOpAlpha = E_BLEND_OPERATION::EBO_ADD;
        E_MODULATE_FUNC modulate = E_MODULATE_FUNC::EMFN_MODULATE_1X;
        u32 alphaSource = EAS_TEXTURE;

        D3D11_BLEND_DESC& blendDesc = getBlendDesc();

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

        auto& renderTarget0 = blendDesc.RenderTarget[0];
        auto SrcBlend = DirectXUtil::getD3DBlend(srcFact); //SrcBlendAlpha
        auto DestBlend = DirectXUtil::getD3DBlend(dstFact);
        auto BlendOp = DirectXUtil::getD3DBlendOp(blendOp);
        auto SrcBlendAlpha = DirectXUtil::getD3DBlend(srcFactAlpha); //SrcBlendAlpha
        auto DestBlendAlpha = DirectXUtil::getD3DBlend(dstFactAlpha);
        auto BlendOpAlpha = DirectXUtil::getD3DBlendOp(blendOpAlpha);

        blendDesc.RenderTarget[0].BlendEnable = blendEnabled;
        //renderTarget0.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        renderTarget0.SrcBlend = SrcBlend;
        renderTarget0.DestBlend = DestBlend;
        renderTarget0.BlendOp = BlendOp;
        renderTarget0.SrcBlendAlpha = SrcBlendAlpha;
        renderTarget0.DestBlendAlpha = DestBlendAlpha;
        renderTarget0.BlendOpAlpha = BlendOpAlpha;
    }

    // Polygon offset
    if ( queryFeature(EVDF_POLYGON_OFFSET) && (resetAllRenderstates ||
        lastMaterial.PolygonOffsetDirection != material.PolygonOffsetDirection ||
        lastMaterial.PolygonOffsetFactor != material.PolygonOffsetFactor) )
    {
        if (!RasterizerStateChanged)
            RasterizerStateChanged = true;

        if ( material.PolygonOffsetFactor )
        {
            if ( material.PolygonOffsetDirection == EPO_BACK )
            {
                RasterizerDesc.SlopeScaledDepthBias = 1.f;
                RasterizerDesc.DepthBias = F2DW((FLOAT)material.PolygonOffsetFactor);
            }
            else
            {
                RasterizerDesc.SlopeScaledDepthBias = -1.f;
                RasterizerDesc.DepthBias = F2DW((FLOAT)-material.PolygonOffsetFactor);

            }
        }
        else
        {
            RasterizerDesc.SlopeScaledDepthBias = 0;
            RasterizerDesc.DepthBias = 0;
        }
    }

    // enable antialiasing
    if ( resetAllRenderstates || lastMaterial.AntiAliasing != material.AntiAliasing )
    {
        //RasterizerDesc.ada
        if ( Params.AntiAlias )
        {
            if (!RasterizerStateChanged)
                RasterizerStateChanged = true;

            if ( material.AntiAliasing & (EAAM_SIMPLE | EAAM_QUALITY) )
                RasterizerDesc.MultisampleEnable = TRUE;
            else if ( lastMaterial.AntiAliasing & (EAAM_SIMPLE | EAAM_QUALITY) )
                RasterizerDesc.MultisampleEnable = FALSE;

            if ( material.AntiAliasing & (EAAM_LINE_SMOOTH) )
                RasterizerDesc.AntialiasedLineEnable = TRUE;
            else if ( lastMaterial.AntiAliasing & (EAAM_LINE_SMOOTH) )
                RasterizerDesc.AntialiasedLineEnable = FALSE;
        }
    }

    // texture address mode
    for (u32 st=0; st<MaxTextureUnits; ++st)
    {
        bool useMipMap = material.getTexture(st) ? material.getTexture(st)->hasMipMaps() : material.UseMipMaps;

        //if ( resetAllRenderstates && Params.HandleSRGB )
        // TODO:

        if ( resetAllRenderstates || lastMaterial.TextureLayer[st].LODBias != material.TextureLayer[st].LODBias )
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;
            SamplerDesc[st].MinLOD = 0;
            SamplerDesc[st].MaxLOD = D3D11_FLOAT32_MAX;

            const float tmp = material.TextureLayer[st].LODBias * 0.125f;
            SamplerDesc[st].MipLODBias = tmp;
        }

        // Describe the Sample State (Review stage.samplerDesc.ComparisonFunc = D3D11Mappings::get(mSceneAlphaRejectFunc);)
        if (SamplerDesc[st].ComparisonFunc != D3D11_COMPARISON_NEVER)
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;
            SamplerDesc[st].ComparisonFunc = D3D11_COMPARISON_NEVER;
        }

        if (resetAllRenderstates || lastMaterial.TextureLayer[st].TextureWrapU != material.TextureLayer[st].TextureWrapU)
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;
            SamplerDesc[st].AddressW = DirectXUtil::getTextureWrapMode(material.TextureLayer[st].TextureWrapU);
        }

        if (resetAllRenderstates || lastMaterial.TextureLayer[st].TextureWrapU != material.TextureLayer[st].TextureWrapU)
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;
            SamplerDesc[st].AddressU = DirectXUtil::getTextureWrapMode(material.TextureLayer[st].TextureWrapU);
        }
        if (resetAllRenderstates || lastMaterial.TextureLayer[st].TextureWrapV != material.TextureLayer[st].TextureWrapV)
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;
            SamplerDesc[st].AddressV = DirectXUtil::getTextureWrapMode(material.TextureLayer[st].TextureWrapV);
        }

        // Bilinear, trilinear, and anisotropic filter
        if ( resetAllRenderstates ||
            lastMaterial.TextureLayer[st].BilinearFilter != material.TextureLayer[st].BilinearFilter ||
            lastMaterial.TextureLayer[st].TrilinearFilter != material.TextureLayer[st].TrilinearFilter ||
            lastMaterial.TextureLayer[st].AnisotropicFilter != material.TextureLayer[st].AnisotropicFilter ||
            lastMaterial.UseMipMaps != useMipMap)
        {
            if (!SamplerStateChanged[st])
                SamplerStateChanged[st] = true;

            if ( material.TextureLayer[st].BilinearFilter || material.TextureLayer[st].TrilinearFilter || material.TextureLayer[st].AnisotropicFilter )
            {
                D3D11_FILTER filter = ( Params.AntiAlias &&
                    material.TextureLayer[st].AnisotropicFilter) ? D3D11_FILTER_ANISOTROPIC : 
                    (material.TextureLayer[st].TrilinearFilter && useMipMap ?
                        D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);

                if ( filter == D3D11_FILTER_ANISOTROPIC )
                    SamplerDesc[st].MaxAnisotropy = (DWORD)material.TextureLayer[st].AnisotropicFilter;
                else
                    SamplerDesc[st].MaxAnisotropy = 0;

                SamplerDesc[st].Filter = filter;
            }
            else
            {
                SamplerDesc[st].Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            }
        }
    }
}


E_DRIVER_TYPE CD3D11Driver::getDriverType() const
{
    return EDT_DIRECT3D11;
}


const core::matrix4& CD3D11Driver::getTransform(E_TRANSFORMATION_STATE state) const
{
    return Matrices[state];
}
//! Creates a render target texture.
ITexture* CD3D11Driver::addRenderTargetTexture(const core::dimension2d<u32>& size,
                const io::path& name, const ECOLOR_FORMAT format, u8 sampleCount)
{
	DXGI_SAMPLE_DESC sampleDesc = present.SampleDesc;
	if (sampleCount)
		sampleDesc = mSampleDescs[(u32)DirectXUtil::ToMSAA(sampleCount)];

    ITexture* tex = new CD3D11Texture(this, size, name, format, 1, sampleDesc.Count, sampleDesc.Quality);
    if (tex)
    {
		if (IImage::isRenderTargetOnlyFormat(format))
			checkDepthBuffer(tex);
        addTexture(tex);
        tex->drop();
    }
    return tex;
}

video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage* surface, const io::path& name)
{
    return new CD3D11Texture(surface, this, TextureCreationFlags, name, 1, nullptr, 1, 0);
}

IHardwareBuffer* CD3D11Driver::createHardwareBuffer(E_HARDWARE_BUFFER_TYPE type, 
                                                    E_HARDWARE_BUFFER_ACCESS accessType, 
                                                    u32 size, u32 flags, const void* initialData)
{
    return new CD3D11HardwareBuffer(this, type, accessType, size, flags, initialData);
}

video::VertexDeclaration * CD3D11Driver::createVertexDeclaration()
{
    return new CD3D11VertexDeclaration(this);
}

E_VERTEX_TYPE CD3D11Driver::registerVertexType(core::array<SVertexElement>& elements)
{
    return E_VERTEX_TYPE(0);
}

u32 CD3D11Driver::queryMultisampleLevels(ECOLOR_FORMAT format, u32 numSamples) const
{
    UINT quality = 0;
    if (SUCCEEDED(Device->CheckMultisampleQualityLevels(DirectXUtil::getD3DFormatFromColorFormat(format), 4, &quality)))
    {
        return quality;
    }

    return 0;
}

void CD3D11Driver::clearZBuffer(f32 clearDepth /*= 1.f*/, u8 clearStencil /*= 0*/)
{
    if( CurrentDepthBuffer )
        ImmediateContext->ClearDepthStencilView( CurrentDepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

bool CD3D11Driver::ResolveMSAA(Msw::ComPtr<ID3D11Texture2D> pStaging, Msw::ComPtr<ID3D11Texture2D> pResolveTex, uint8_t msaa)
{
    // Resolve MSAA
    DX_BEGIN_EVENT(L"Resolve");

    Msw::ComPtr<ID3D11RenderTargetView> RTTextureView;
    HRESULT hr = Device->CreateRenderTargetView(pStaging.Get(), 0, RTTextureView.GetAddressOf());

    Msw::ComPtr<ID3D11ShaderResourceView> SRMSAAView;
    hr = Device->CreateShaderResourceView(pResolveTex.Get(), 0, SRMSAAView.GetAddressOf());

    auto prevShader = GetActiveShader();

        u32 _current_msaa = DirectXUtil::ToMSAA(msaa);
        Material.MaterialType = E_MATERIAL_TYPE::EMT_SOLID;
        Material.BlendOperation = E_BLEND_OPERATION::EBO_NONE;
        Material.ColorMask = 0xF;

        Material.IsCCW = false;
        Material.ScissorEnable = false;
        Material.Wireframe = false;
        Material.ZWriteEnable = false;
        Material.DepthClipEnable = true;
        Material.FrontfaceCulling = false;
        Material.BackfaceCulling = false;
        Material.ZBuffer = irr::video::E_COMPARISON_FUNC::ECFN_NEVER;

        Material.StencilTest = false;
        Material.StencilFront.Mask = 0xFF;
        Material.StencilFront.WriteMask = 0xFF;
        Material.StencilFront.Reference = 0;

        setRenderStates3DMode();
        SetShader(nullptr);
        InitDrawStates(nullptr);

        ImmediateContext->IASetInputLayout(nullptr);
        SetShader(m_rtResolveShader[_current_msaa]);

        ImmediateContext->OMSetRenderTargets(1, RTTextureView.GetAddressOf(), 0);
        ImmediateContext->PSSetShaderResources(0, 1, SRMSAAView.GetAddressOf());

        ImmediateContext->Draw(3, 0);


    ImmediateContext->IASetInputLayout(CurrentInputLayout.Get());
    SetShader(prevShader);

    DX_END_EVENT();
    return true;
}

static HRESULT CaptureTexture(CD3D11Driver* driver,
    _In_ ID3D11DeviceContext* pContext,
    _In_ ID3D11Resource* pSource,
    _Inout_ D3D11_TEXTURE2D_DESC& desc,
    _Inout_ Msw::ComPtr<ID3D11Texture2D>& pStaging)
{
    if (!pContext || !pSource)
        return E_INVALIDARG;

    D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    pSource->GetType(&resType);

    if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

    Msw::ComPtr<ID3D11Texture2D> pTexture;
    HRESULT hr = pSource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pTexture.GetAddressOf()));
    if (FAILED(hr))
        return hr;

    assert(pTexture);

    pTexture->GetDesc(&desc);

    Msw::ComPtr<ID3D11Device> d3dDevice;
    pContext->GetDevice(d3dDevice.GetAddressOf());

    uint8_t msaa = desc.SampleDesc.Count;

    if (desc.SampleDesc.Count > 1)
    {
        // MSAA content must be resolved before being copied to a staging texture
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        Msw::ComPtr<ID3D11Texture2D> pTemp;
        hr = d3dDevice->CreateTexture2D(&desc, 0, pTemp.GetAddressOf());
        if (FAILED(hr))
            return hr;

        assert(pTemp);

        DXGI_FORMAT fmt = desc.Format;

        UINT support = 0;
        hr = d3dDevice->CheckFormatSupport(fmt, &support);
        if (FAILED(hr))
            return hr;

        if (support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE)
        {
            for (UINT item = 0; item < desc.ArraySize; ++item)
            {
                for (UINT level = 0; level < desc.MipLevels; ++level)
                {
                    UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
                    pContext->ResolveSubresource(pTemp.Get(), index, pSource, index, fmt);
                }
            }
        }
        else
        {
            return -1;
            //ID3D11Texture2D* bufftex = 0;
            //D3D11_TEXTURE2D_DESC tex_desc = desc; //(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, present.SampleDesc.Count, present.SampleDesc.Quality);
            //tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            //tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            //tex_desc.SampleDesc.Count = msaa;
            //tex_desc.Usage = D3D11_USAGE_DYNAMIC;
            //
            //// First, create a texture buffer
            //HRESULT hr = d3dDevice->CreateTexture2D(&tex_desc, NULL, &bufftex);
            //if (FAILED(hr))
            //    return NULL;
            //
            //// Copy back buffer to our texture buffer
            //pContext->CopyResource(bufftex, pTexture.Get());
            //
            //driver->ResolveMSAA(pTemp, bufftex, msaa);
        }

        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;

        hr = d3dDevice->CreateTexture2D(&desc, 0, pStaging.GetAddressOf());
        if (FAILED(hr))
            return hr;

        assert(pStaging);

        pContext->CopyResource(pStaging.Get(), pTemp.Get());
    }
    else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
    {
        // Handle case where the source is already a staging texture we can use directly
        pStaging = pTexture;
    }
    else
    {
        // Otherwise, create a staging texture from the non-MSAA source
        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;

        hr = d3dDevice->CreateTexture2D(&desc, 0, pStaging.GetAddressOf());
        if (FAILED(hr))
            return hr;

        assert(pStaging);

        pContext->CopyResource(pStaging.Get(), pSource);
    }
}

IImage* CD3D11Driver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)
{
    CD3D11_TEXTURE2D_DESC output_desc;
    Msw::ComPtr<ID3D11Texture2D> output;
    // Copy back buffer to our texture buffer
    ID3D11Resource* backBuffer = 0;
    D3D11_MAPPED_SUBRESOURCE mappedData;
    DefaultBackBuffer->GetResource(&backBuffer);
    
    if (FAILED(CaptureTexture(this, ImmediateContext.Get(), backBuffer, output_desc, output)))
        return nullptr;

    backBuffer->Release();

    u32 width = getCurrentRenderTargetSize().Width;
    u32 height = getCurrentRenderTargetSize().Height;

    auto irrformat = DirectXUtil::getColorFormatFromD3DFormat(output_desc.Format);
    IImage* img = createImage(irrformat, getCurrentRenderTargetSize());
    IImage* imgTo = createImage(video::ECOLOR_FORMAT::ECF_A8R8G8B8, getCurrentRenderTargetSize());

    if( !img )
        return NULL;

    // Get buffer data
    if( FAILED( ImmediateContext->Map(output.Get(), 0, D3D11_MAP_READ, 0, &mappedData ) ) )
        return NULL;

    // Process data in place, handling alpha channel
    //for (u32 y = 0; y < height; ++y)
    //{
    //    u8* ptr = (u8*)mappedData.pData + y * mappedData.RowPitch + 3;
    //    for (u32 x = 0; x < width; ++x)
    //    {
    //        *ptr = 0xFF;
    //        ptr += 4;
    //    }
    //}
    
    // Copy data to image
    ::CopyMemory( img->lock(), mappedData.pData, width * height * 4 );
    
    // Unlock image and texture
    img->unlock();
    ImmediateContext->Unmap(output.Get(), 0 );

    img->copyTo(imgTo);

    // Return image
    return imgTo;
}

bool CD3D11Driver::setClipPlane(u32 index, const core::plane3df& plane, bool enable)
{
    if(index > 3)
        return false;

    ClipPlanes[index] = plane;
    
    enableClipPlane(index, enable);

    return true;
}

void CD3D11Driver::enableClipPlane(u32 index, bool enable)
{
    ClipPlaneEnabled[index] = enable;
}

void CD3D11Driver::getClipPlane(u32 index, core::plane3df& plane, bool& enable)
{
    plane = ClipPlanes[index];
    enable = ClipPlaneEnabled[index];
}

void CD3D11Driver::enableMaterial2D(bool enable)
{
    CNullDriver::enableMaterial2D(enable);
} 

ECOLOR_FORMAT CD3D11Driver::getColorFormat() const
{
    return ColorFormat;
}

DXGI_FORMAT CD3D11Driver::getD3DColorFormat() const
{
    return D3DColorFormat;
}

core::dimension2du CD3D11Driver::getMaxTextureSize() const
{
    // Maximal value depends of driver type.
    switch( FeatureLevel )
    {
    case D3D_FEATURE_LEVEL_11_0:
        return core::dimension2du( 16384, 16384 );

    case D3D10_FEATURE_LEVEL_10_1:
    case D3D10_FEATURE_LEVEL_10_0:
        return core::dimension2du( 8192, 8192 );

    case D3D10_FEATURE_LEVEL_9_3:
        return core::dimension2du( 4096, 4096 );
    
    case D3D10_FEATURE_LEVEL_9_2:
    case D3D10_FEATURE_LEVEL_9_1:
        return core::dimension2du( 2048, 2048 );
    }

    return core::dimension2du( 16384, 16384 );
}

bool CD3D11Driver::querySupportForColorFormat(DXGI_FORMAT format, D3D11_FORMAT_SUPPORT support)
{
    UINT values = 0;
    if( FAILED( Device->CheckFormatSupport(format, &values) ) )
        return false;

    if( values && support )
        return true;

    return false;
}

void CD3D11Driver::checkDepthBuffer(ITexture* tex)
{
    if (!tex)
        return;

    const core::dimension2du optSize = tex->getSize().getOptimalSize(!queryFeature(EVDF_TEXTURE_NPOT), !queryFeature(EVDF_TEXTURE_NSQUARE), true);
    SDepthSurface11* depth=0;
    core::dimension2du destSize(0x7fffffff, 0x7fffffff);
    for (u32 i=0; i<DepthBuffers.size(); ++i)
    {
        if ((DepthBuffers[i]->Size.Width>=optSize.Width) &&
            (DepthBuffers[i]->Size.Height>=optSize.Height))
        {
            if ((DepthBuffers[i]->Size.Width<destSize.Width) &&
                (DepthBuffers[i]->Size.Height<destSize.Height))
            {
                depth = DepthBuffers[i];
                destSize=DepthBuffers[i]->Size;
            }
        }
    }
    
    // Create a depth buffer for this texture
    if (!depth)
    {
        // create depth buffer
        DepthBuffers.push_back(irr::MakePtr<SDepthSurface11>());
        DepthBuffers.getLast()->Surface = createDepthStencilView(optSize);
        if ( DepthBuffers.getLast()->Surface )
        {
            depth=DepthBuffers.getLast();
            depth->Size.set(optSize.Width, optSize.Height);
        }
        else
        {
            char buffer[128];
            sprintf(buffer,"Could not create DepthBuffer of %ix%i",optSize.Width,optSize.Height);
            os::Printer::log(buffer,ELL_ERROR);
            DepthBuffers.erase(DepthBuffers.size()-1);
        }
    }

    static_cast<CD3D11Texture*>(tex)->DepthSurface = irr::Ptr<SDepthSurface11>(depth);
}

void CD3D11Driver::removeDepthSurface(SDepthSurface11* depth)
{
    for (u32 i=0; i<DepthBuffers.size(); ++i)
    {
        if (DepthBuffers[i]==depth)
        {
            DepthBuffers.erase(i);
            return;
        }
    }
}

bool CD3D11Driver::uploadVertexData(const void* vertices, u32 vertexCount,
                                    const void* indexList, u32 indexCount,
                                    E_VERTEX_TYPE vType, E_INDEX_TYPE iType)
{
    // Parse information about buffers
    u32 vertexStride;
    {
        if (vType >= E_VERTEX_TYPE::EVT_MAX_VERTEX_TYPE)
        {
            vertexStride = GetVertexDeclaration(vType)->GetVertexPitch(0);
        }
        else
        {
            vertexStride = getVertexPitchFromType(vType);
        }
    }

    const u32 indexStride = video::getIndexSize(iType); // == video::EIT_16BIT ? 2 : 4;
    const DXGI_FORMAT indexFormat = iType == video::EIT_16BIT ? DXGI_FORMAT::DXGI_FORMAT_R16_UINT : DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
    const UINT offset = 0;

    // reallocated if needed
    if( !reallocateDynamicBuffers( vertexCount * vertexStride, indexCount * indexStride ) )
        return false;

    HRESULT hr = S_OK;
    if(vertices)
    {        
        D3D11_MAPPED_SUBRESOURCE mappedData;
        ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));

        hr = ImmediateContext->Map( DynVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
        if(SUCCEEDED(hr))
        {
            ::CopyMemory(mappedData.pData, vertices, vertexCount * vertexStride);
            ImmediateContext->Unmap( DynVertexBuffer.Get(), 0 );
        }
        else
        {
            os::Printer::log("Error, could not upload dynamic vertex data to GPU", ELL_ERROR);
            return false;
        }

        // set vertex buffer
        ImmediateContext->IASetVertexBuffers( 0, 1, DynVertexBuffer.GetAddressOf(), &vertexStride, &offset );
    }
    if(indexList)
    {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));

        hr = ImmediateContext->Map( DynIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
        if(SUCCEEDED(hr))
        {
            ::CopyMemory(mappedData.pData, indexList, indexCount * indexStride);
            ImmediateContext->Unmap( DynIndexBuffer.Get(), 0 );
        }
        else
        {
            os::Printer::log("Error, could not upload dynamic index data to GPU", ELL_ERROR);
            return false;
        }

        // set index buffer
        ImmediateContext->IASetIndexBuffer( DynIndexBuffer.Get(), indexFormat, 0 );
    }
    else
    {
        ImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
    }

    return true;
}

bool CD3D11Driver::reallocateDynamicBuffers( u32 vertexBufferSize, u32 indexBufferSize )
{
    HRESULT hr = S_OK;

    if( !DynVertexBuffer || DynVertexBufferSize < vertexBufferSize )
    {
        D3D11_BUFFER_DESC desc;
        ::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

        // Release old buffer if small
        if( DynVertexBufferSize < vertexBufferSize )
			DynVertexBuffer = nullptr;

        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = vertexBufferSize;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        hr = Device->CreateBuffer( &desc, NULL, DynVertexBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            os::Printer::log("Error, failed to create dynamic vertex buffer", ELL_ERROR);
            return false;
        }

        DynVertexBufferSize = vertexBufferSize;
    }

    if( !DynIndexBuffer || DynIndexBufferSize < indexBufferSize )
    {
        D3D11_BUFFER_DESC desc;
        ::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

        // Release old buffer if small
        if( DynIndexBufferSize < indexBufferSize ) 
			DynIndexBuffer = nullptr;

        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = indexBufferSize;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        hr = Device->CreateBuffer( &desc, NULL, DynIndexBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            os::Printer::log("Error, failed to create dynamic index buffer", ELL_ERROR);
            return false;
        }

        DynIndexBufferSize = indexBufferSize;
    }

    return true;
}

ID3D11DepthStencilView* CD3D11Driver::createDepthStencilView(core::dimension2d<u32> size)
{
    bool canbeShaderResource = false;
    DXGI_FORMAT DepthViewFormat = DepthStencilFormat; 
    // check stencil buffer format
    if( DepthStencilFormat == DXGI_FORMAT_UNKNOWN )
    {
        DepthStencilFormat = DXGI_FORMAT_R32G8X24_TYPELESS; // DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        DepthViewFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        canbeShaderResource = true;
        UINT formatSupport = 0;
        if( Params.Stencilbuffer )
        {
            Device->CheckFormatSupport( DepthStencilFormat, &formatSupport );
            if( ( formatSupport && D3D11_FORMAT_SUPPORT_DEPTH_STENCIL ) == 0 )
            {
                DepthStencilFormat = DXGI_FORMAT_R24G8_TYPELESS;
                canbeShaderResource = false;
                Device->CheckFormatSupport( DepthStencilFormat, &formatSupport );
                if( ( formatSupport && D3D11_FORMAT_SUPPORT_DEPTH_STENCIL ) == 0 )
                {
                    os::Printer::log("Device does not support stencilbuffer, disabling stencil buffer.", ELL_WARNING);
                    Params.Stencilbuffer = false;
                }
            }        
        }
        if (!Params.Stencilbuffer)    // do not use else here to cope with flag change in previous block
        {
            DepthStencilFormat = DXGI_FORMAT_R32_TYPELESS;
            DepthViewFormat = DXGI_FORMAT_D32_FLOAT;
            Device->CheckFormatSupport( DepthStencilFormat, &formatSupport );
            if( ( formatSupport && D3D11_FORMAT_SUPPORT_DEPTH_STENCIL ) == 0 )
            {
                DepthStencilFormat = DXGI_FORMAT_R16_TYPELESS;
                canbeShaderResource = false;
                Device->CheckFormatSupport( DepthStencilFormat, &formatSupport );
                if( ( formatSupport && D3D11_FORMAT_SUPPORT_DEPTH_STENCIL ) == 0 )
                {
                    os::Printer::log("Device does not support required depth buffer.", ELL_WARNING);
                    return NULL;
                }
            }
        }
    }

	DepthViewFormat = DirectXUtil::getD3DFormatFromTypeless(DepthStencilFormat);

    // create depth buffer
    ID3D11DepthStencilView* dsView = NULL;
    ID3D11Texture2D* depthTexture = NULL;
    D3D11_TEXTURE2D_DESC dsTexDesc;
    ::ZeroMemory( &dsTexDesc, sizeof( dsTexDesc ) );
    dsTexDesc.ArraySize = 1;
    dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    //// If we tell we want to use it as a Shader Resource when in MSAA, we will fail
    //// This is a recomandation from NVidia.
    if (canbeShaderResource && FeatureLevel >= D3D_FEATURE_LEVEL_10_0 && present.SampleDesc.Count == 1)
        dsTexDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    dsTexDesc.CPUAccessFlags = 0;
    if (FeatureLevel < D3D_FEATURE_LEVEL_10_0)
        dsTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    else
        dsTexDesc.Format = DepthStencilFormat;

	dsTexDesc.MipLevels = 1;
    dsTexDesc.MiscFlags = 0;
    dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
    dsTexDesc.SampleDesc = present.SampleDesc;
    dsTexDesc.Width = (UINT)size.Width;
    dsTexDesc.Height = (UINT)size.Height;

    if (dsTexDesc.ArraySize == 6)
    {
        dsTexDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    HRESULT hr = Device->CreateTexture2D( &dsTexDesc, NULL, &depthTexture );
    if(FAILED(hr))
    {
        os::Printer::log("Error, could not create depth texture.", ELL_WARNING);
        return NULL;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
    ::ZeroMemory( &dsDesc, sizeof( dsDesc ) );

    if (FeatureLevel < D3D_FEATURE_LEVEL_10_0)
        dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    else
        dsDesc.Format = DepthViewFormat;

	dsDesc.Flags = 0;  /* D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL */;    // TODO: Allows bind depth buffer as depth view AND texture simultaneously.
                                                                                         // TODO: Decide how to expose this feature
    dsDesc.ViewDimension = (present.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsDesc.Texture2D.MipSlice = 0;
    hr = Device->CreateDepthStencilView( depthTexture, &dsDesc, &dsView );
    depthTexture->Release();
    if(FAILED(hr))
    {
        os::Printer::log("Error, could not create depth stencil view.", ELL_WARNING);
        return NULL;
    }

    return dsView;
}

void CD3D11Driver::reset()
{
//    u32 i;
    os::Printer::log("Resetting D3D11 device.", ELL_INFORMATION);

    ImmediateContext->OMSetRenderTargets(0, 0, 0);
    
    HRESULT hr = S_OK;

    for (auto iTex : Textures)
    {
        if ( iTex.second->isRenderTarget())
        {
            ID3D11Resource* tex = ((CD3D11Texture*)(iTex.second))->getTextureResource();
            if (tex)
                tex->Release();
        }
    }

    // clear textures
    for (u16 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
        setActiveTexture(i, 0);

    // unbind render targets
    ID3D11RenderTargetView* views[] = { NULL };
    ImmediateContext->OMSetRenderTargets(1, views, NULL);
    SetShader(nullptr);

    // Unbound all shader resources
    ID3D11ShaderResourceView* tviews[1] = { NULL };
    ImmediateContext->VSSetShaderResources(0, 1, tviews);
    ImmediateContext->HSSetShaderResources(0, 1, tviews);
    ImmediateContext->DSSetShaderResources(0, 1, tviews);
    ImmediateContext->GSSetShaderResources(0, 1, tviews);
    ImmediateContext->PSSetShaderResources(0, 1, tviews);

    DefaultBackBuffer = nullptr;
    DefaultDepthBuffer = nullptr;
	CurrentBackBuffer = nullptr;
	CurrentDepthBuffer = nullptr;

    // If fullscreen, do it
    if (Params.Fullscreen)
    {
        SwapChain->SetFullscreenState(TRUE, Output.Get());
    }
    else
    {
        SwapChain->SetFullscreenState(FALSE, NULL);
    }

    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = SwapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    if(FAILED(hr))
    {
        os::Printer::log("Error resizing back buffer", ELL_ERROR);
        return;
    }


    ID3D11Texture2D* backBuffer = NULL;
    hr = SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &backBuffer ) );
    if( FAILED(hr))
    {
        os::Printer::log("Error, could not get back buffer.", ELL_ERROR);
        return;
    }

    // Get default render target
    //D2DContext->SetDpi(0, 0);
    //D2DContext->SetTarget(nullptr);
    //
    //D2DContext->GetDpi(&m_DPI.X, &m_DPI.Y);

    hr = Device->CreateRenderTargetView( backBuffer, NULL, DefaultBackBuffer.GetAddressOf());
    if( FAILED(hr))
    {
        os::Printer::log("Error, could not create render target view.", ELL_ERROR);
        return;
    }
    SAFE_RELEASE(backBuffer);

    // create depth buffer
    DefaultDepthBuffer = createDepthStencilView(ScreenSize);

    // Set render targets
    setRenderTarget(0, true, true);

    //CurrentDepthBuffer = nullptr;
    //CurrentBackBuffer = nullptr;

    //IDXGISurface* dxgiBackBuffer;
    //DX::ThrowIfFailed(
    //    SwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
    //    );
    //
    ////// Create a Direct2D target bitmap associated with the
    //// swap chain back buffer and set it as the current target.
    //D2D1_BITMAP_PROPERTIES1 bitmapProperties =
    //    D2D1::BitmapProperties1(
    //        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
    //        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    //        , m_DPI.X, m_DPI.Y
    //        );
    //
    //DX::ThrowIfFailed(
    //    D2DContext->CreateBitmapFromDxgiSurface(
    //        dxgiBackBuffer,
    //        bitmapProperties,
    //        &D2DTargetBitmap
    //        )
    //    );
    //
    //D2DContext->SetTarget(D2DTargetBitmap);
    //
    //// Grayscale text anti-aliasing is recommended for all Windows Store apps.
    //D2DContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    for (u32 i = 0; i<MATERIAL_MAX_TEXTURES; ++i)
        CurrentTexture[i] = 0;

    // init states description
    if (!BlendStateChanged)
        BlendStateChanged = true;

    if (!RasterizerStateChanged)
        RasterizerStateChanged = true;

    if (!DepthStencilStateChanged)
        DepthStencilStateChanged = true;

    BlendDesc.reset();
    RasterizerDesc.reset();
    DepthStencilDesc.reset();

    for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
    {
        SamplerStateChanged[i] = true;
        SamplerDesc[i].reset();
    }

    // set fog mode
    setFog(FogColor, (E_FOG_TYPE)0, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

    ResetRenderStates = true;

    setRenderStates2DMode(true, false, true);
    setRenderStates3DMode();
    setFog(FogColor, FogType, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);
    setAmbientLight(AmbientLight);
}

ITexture * CD3D11Driver::addTextureCubemap(const io::path & name, IImage * imagePosX, IImage * imageNegX, IImage * imagePosY, IImage * imageNegY, IImage * imagePosZ, IImage * imageNegZ)
{
    return nullptr;
}

ITexture * CD3D11Driver::addTextureArray(const io::path & name, irr::core::array<IImage*> images)
{
    return nullptr;
}

// returns the current size of the screen or rendertarget
const core::dimension2d<u32>& CD3D11Driver::getCurrentRenderTargetSize() const
{
    if ( CurrentRendertargetSize.Width == 0 )
        return ScreenSize;
    else
        return CurrentRendertargetSize;
}

////////////// IGPUProgrammingServices methods start ////////////////////////////////////////////

//s32 CD3D11Driver::addHighLevelShaderMaterial(
//            const c8* vertexShaderProgram,
//            const c8* vertexShaderEntryPointName,
//            E_VERTEX_SHADER_TYPE vsCompileTarget,
//            const c8* pixelShaderProgram,
//            const c8* pixelShaderEntryPointName,
//            E_PIXEL_SHADER_TYPE psCompileTarget,
//            const c8* geometryShaderProgram,
//            const c8* geometryShaderEntryPointName,
//            E_GEOMETRY_SHADER_TYPE gsCompileTarget,
//            scene::E_PRIMITIVE_TYPE inType,
//            scene::E_PRIMITIVE_TYPE outType,
//            u32 verticesOut,
//            E_VERTEX_TYPE vertexTypeOut,
//            IShaderConstantSetCallBack* callback,
//            E_MATERIAL_TYPE baseMaterial,
//            s32 userData)
//{
//    s32 id = 0;
//    //CD3D11CustomMaterialRenderer* rend = 
//    //        new CD3D11CustomMaterialRenderer(Device, this,
//    //                                         id,
//    //                                         vertexShaderProgram,
//    //                                         vertexShaderEntryPointName,
//    //                                         vsCompileTarget,
//    //                                         pixelShaderProgram,
//    //                                         pixelShaderEntryPointName,
//    //                                         psCompileTarget,
//    //                                         geometryShaderProgram,
//    //                                         geometryShaderEntryPointName,
//    //                                         gsCompileTarget,
//    //                                         inType,
//    //                                         outType,
//    //                                         verticesOut,
//    //                                         vertexTypeOut,
//    //                                         callback,
//    //                                         MaterialRenderers[baseMaterial].Renderer,
//    //                                         userData);
//    return id;
//}

////////////// IGPUProgrammingServices methods end ////////////////////////////////////////////

////////////// IMaterialRenderer methods start ////////////////////////////////////////////

void CD3D11Driver::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
    os::Printer::log("\"setVertexShaderConstant\" with offset is not supported", ELL_ERROR);
}

void CD3D11Driver::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
    os::Printer::log("\"setPixelShaderConstant\" with offset is not supported", ELL_ERROR);
}

bool CD3D11Driver::setVertexShaderConstant(const c8* name, const f32* floats, int count)
{
    static_cast<CD3D11MaterialRenderer*>(MaterialRenderers[Material.MaterialType].Renderer)->setVariable(name, floats, count);
    return true;
}

bool CD3D11Driver::setPixelShaderConstant(const c8* name, const f32* floats, int count)
{
    static_cast<CD3D11MaterialRenderer*>(MaterialRenderers[Material.MaterialType].Renderer)->setVariable(name, floats, count);
    return true;
}

bool CD3D11Driver::setStreamOutputBuffer(IHardwareBuffer* buffer)
{
    ID3D11Buffer* buffers = 0;
    UINT offset = 0;

    // If buffer is null, remove from pipeline
    if (!buffer)
    {
        ImmediateContext->SOSetTargets(1, &buffers, &offset);
        return true;
    }

    // Validate buffer
    if (buffer->getDriverType() != EDT_DIRECT3D11)
    {
        os::Printer::log("Fatal Error: Tried to set a buffer not owned by this driver.", ELL_ERROR);
        return false;
    }
    if (buffer->getType(E_HARDWARE_BUFFER_TYPE::EHBT_STREAM_OUTPUT) != E_HARDWARE_BUFFER_TYPE::EHBT_STREAM_OUTPUT)
    {
        os::Printer::log("Fatal Error: Tried to set a buffer that is not for stream output.", ELL_ERROR);
        return false;
    }

    // Set stream output buffer
    buffers = static_cast<CD3D11HardwareBuffer*>(buffer)->getBufferResource(E_HARDWARE_BUFFER_TYPE::EHBT_STREAM_OUTPUT);
    ImmediateContext->SOSetTargets(1, &buffers, &offset);

    return true;
}

IVideoDriver* CD3D11Driver::getVideoDriver()
{
    return this;
}

////////////// IMaterialRenderer methods end ////////////////////////////////////////////

} // end namespace video
} // end namespace irr

#endif

extern "C" void D3DLoadShaderCache(System::IO::IFileReader* file);

namespace irr
{
namespace video
{

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
//! creates a video driver
IVideoDriver* createDirectX11Driver(const SIrrlichtCreationParameters& params,
    io::IFileSystem* io, HWND hwnd)
{
    bool pureSoftware = false;
    CD3D11Driver* dx11 = new CD3D11Driver(params, io, pureSoftware);
    if (!dx11->initDriver(hwnd, pureSoftware))
    {
        dx11->drop();
        dx11 = 0;
    }

    return dx11;
}

void loadDirectX11ShaderCache(System::IO::IFileReader* file)
{
    D3DLoadShaderCache(file);
}
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_

} // end namespace video
} // end namespace irr