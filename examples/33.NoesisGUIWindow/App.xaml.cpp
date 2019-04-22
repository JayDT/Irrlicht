#include "App.xaml.h"
#include "Core/ThemeGenerator.h"

#include <NsCore/Delegate.h>
#include <NsGui/UIElementEvents.h>
#include <NsCore/Kernel.h>
#include <NsCore/Error.h>
#include <NsCore/Log.h>
#include <NsCore/StringUtils.h>
#include <NsCore/HighResTimer.h>
#include <NsApp/CommandLine.h>
#include <NsApp/IrrNsGuiBindings.h>
#include "NsImpl/IrrRenderContext.h"
#include "NsImpl/IrrOsDeviceContext.h"
#include "NsApp/Window.h"
#include "NsApp/IrrWindow.h"
#include "System/Resource/ResourceManager.h"

#include "standard/BasicPrimities.h"
#include "standard/Timer.h"
#include "standard/String/atoi_imp.h"
#include "Threading/Threading.h"
#include "Log/Log.h"
#include "Config/Config.h"

#include "include/irrlicht.h"

using namespace WorldClient;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, App)
{
	NS_REGISTER_COMPONENT(App)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PACKAGE_EXPORT void NsRegisterReflection_WorldClient(Noesis::ComponentFactory* factory, bool doRegister)
{
	PACKAGE_REGISTER(Material, ControlsHelper);
	PACKAGE_REGISTER(Material, ToggleButtonHelper);
	PACKAGE_REGISTER(Material, StringToVisibilityConverter);
	PACKAGE_REGISTER(Material, IsNullConverter);
	PACKAGE_REGISTER(Material, ItemHelper);
	PACKAGE_REGISTER(Material, MathConverter);
	PACKAGE_REGISTER(Material, TextBoxHelper);
	PACKAGE_REGISTER(WorldClient, App);
	PACKAGE_REGISTER(WorldClient, MainSettings);
	PACKAGE_REGISTER(WorldClient, MainWindow);
	PACKAGE_REGISTER(WorldClient, RootWindow);
    PACKAGE_REGISTER(WorldClient, FileDialog);
}

namespace
{
    WorldClient::App* gInstance;
    NoesisApp::CommandLine gCommandLine;
    irr::SIrrlichtCreationParameters gParams;
    irr::IrrlichtDevice* gDevice;

	void LogHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message)
	{
		gInstance->LoggingHandler(file, line, level, channel, message);
	};

	void ErrorHandler(const char* file, uint32_t line, const char* message, bool fatal)
	{
		char out[512];
		static const char* prefixes[] = { "E", "F" };
		const char* prefix = prefixes[fatal ? 1 : 0];
		snprintf(out, sizeof(out), "[NOESIS/%s] %s", prefix, message);
		System::Console()->WriteErrorLine(out);
	};
}

const irr::SIrrlichtCreationParameters& WorldClient::GetDefaultIrrDeviceParam()
{
	return gParams;
}

WorldClient::App::App()
{
    gInstance = this;
}

WorldClient::App::~App()
{
}

void WorldClient::App::StaticInitializeComponents()
{
    LoadShaderCache("ShaderCache.irsx");

    gParams.DriverType = irr::video::EDT_DIRECT3D11;
    gParams.WindowSize = irr::core::dimension2d<irr::u32>(1280, 720);
    gParams.Bits = (irr::u8)16;
    gParams.Fullscreen = false;
    gParams.Stencilbuffer = false;
    gParams.Vsync = false;
    gParams.EventReceiver = nullptr;
    gParams.AntiAlias = 4;

    gDevice = irr::createDeviceEx(gParams);

    NoesisApp::Noesis_Init(&ErrorHandler, &LogHandler, nullptr);
	NsRegisterReflection_WorldClient(nullptr, true);
	InitColorStyle();
}

void WorldClient::App::Initialize(const NoesisApp::CommandLine& arguments)
{
	SetIrrlichDevice(gDevice);

	mMainDisplay = NoesisApp::CreateDisplay(gDevice);
	Init(mMainDisplay, arguments);

	CreateNsWindow("pack://application:,,,/33.NoesisGUIWindow;/UI/RootWindow.xaml");
    AddWindow("pack://application:,,,/33.NoesisGUIWindow;/UI/MainWindow.xaml");

    SaveShaderCache("ShaderCache.irsx");
}

void WorldClient::App::Run()
{
    std::wstring _debugTitle;
    int32 _prevFps = -1, _lastpasstime = 0, _passtimediff = 0, prevSleepTime = 0;
    uint32 _curMSTime = System::Time::getMSTime();

	NS_MSVC_WARNING_SUPPRESS(4640)
	static uint64_t startTime = Noesis::HighResTimer::Ticks();

    while (m_running)
    {
        _lastpasstime = _curMSTime;
        _curMSTime = System::Time::getMSTime();
        _passtimediff = _curMSTime - _lastpasstime;
		double _highResTime = Noesis::HighResTimer::Seconds(Noesis::HighResTimer::Ticks() - startTime);

        if (gDevice->run())
        {
            gDevice->getVideoDriver()->beginScene();
        
			Tick(_highResTime);
        
            //if (_clientPlatform)
            //    _clientPlatform->OnRenderConsole();
        
            gDevice->getVideoDriver()->endScene();
        }

		Idle(_highResTime);
       
        if (_passtimediff <= 5 + prevSleepTime)
        {
            prevSleepTime = 5 + prevSleepTime - _passtimediff;
            if (((int32)prevSleepTime) < 0)
                prevSleepTime = 0;
        }
        
        auto fps = gDevice->getVideoDriver()->getFPS();
        
        if (_prevFps != fps)
        {
            _debugTitle.clear();
            _debugTitle = L"IrrLicht [";
            _debugTitle += gDevice->getVideoDriver()->getName();
            _debugTitle += L"] FPS: [";
        
            _debugTitle += std::to_wstring(fps);
            _debugTitle += L"] Update Time: ";
            _debugTitle += std::to_wstring(_passtimediff);
            gDevice->setWindowCaption(_debugTitle.c_str());
        
            _prevFps = fps;
        }
        
        if (prevSleepTime > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(prevSleepTime));
    }
}

void WorldClient::App::LoadShaderCache(const char* filename)
{
    System::IO::StandardDataSource fileMgr;
    auto reader = fileMgr.OpenFile(filename);
    if (reader)
    {
        irr::preloadShaderCache(reader);
        delete reader;
    }
}

void WorldClient::App::SaveShaderCache(const char* filename) const
{
    System::IO::StandardDataSource fileMgr;
    auto writer = fileMgr.OpenFileToWrite(filename);
    if (writer)
    {
        gDevice->getVideoDriver()->getGPUProgrammingServices()->WriteShaderCache(writer);
        delete writer;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldClient::App::LoggingHandler(const char* file, uint32_t line, uint32_t level, const char* channel,
    const char* message)
{
    NS_UNUSED(file, line);

    // By default only global channel is dumped
    bool filter = !Noesis::String::IsNullOrEmpty(channel);

    // Enable "Binding" channel by command line
    if (gCommandLine.HasOption("log_binding"))
    {
        if (Noesis::String::Equals(channel, "Binding"))
        {
            filter = false;
        }
    }

    if (!filter)
    {
        char out[512];
        static const char* prefixes[] = { "T", "D", "I", "W", "E" };
        const char* prefix = level < NS_COUNTOF(prefixes) ? prefixes[level] : " ";
        snprintf(out, sizeof(out), "[NOESIS/%s] %s", prefix, message);

		System::Console()->WriteErrorLine(out);
#if defined(NS_PLATFORM_WINDOWS_DESKTOP)
        OutputDebugString(out);
#elif defined(NS_PLATFORM_ANDROID)
        switch (level)
        {
        case NS_LOG_LEVEL_TRACE:
        case NS_LOG_LEVEL_DEBUG: LOGD("%s", message); break;
        case NS_LOG_LEVEL_INFO: LOGI("%s", message); break;
        case NS_LOG_LEVEL_WARNING: LOGW("%s", message); break;
        case NS_LOG_LEVEL_ERROR: LOGE("%s", message); break;
        }
#else
        fprintf(stderr, "%s", out);
#endif
    }
}

void WorldClient::App::AddWindow(const char* url)
{
    Noesis::Ptr<BaseComponent> root = Noesis::GUI::LoadXaml(url);
    auto mMainWindow = Noesis::DynamicPtrCast<NoesisApp::IrrWindow>(root);
    AddWindow(mMainWindow.GetPtr());
}

void WorldClient::App::AddWindow(NoesisApp::IrrWindow* wnd)
{
    Noesis::Panel* holder = Noesis::DynamicCast<Noesis::Panel*>(FindName("PART_CONTENTHOLDER"));
    if (holder)
        holder->GetChildren()->Add(wnd);
}

NoesisApp::IrrWindow* WorldClient::App::GetWindow(const char* wndName) const
{
    return Noesis::DynamicCast<NoesisApp::IrrWindow*>(FindNodeName(wndName));
}

App* WorldClient::App::Instance()
{
    return gInstance;
}

void WorldClient::App::InitColorStyle()
{
	auto parameters = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/33.NoesisGUIWindow;/Theme/Template/GeneratorParameters.json", false), 0);
	auto colorTemplate = System::Resource::ResourceManager::Instance()->FindResource(System::URI("pack://application:,,,/33.NoesisGUIWindow;/Theme/Template/Theme.Template.xaml", false), 0);

	WorldClient::Core::XamlThemeDeserializer deserializer;
	std::string json((const char*)parameters->ToBytes().data(), parameters->ToBytes().size());
	std::string colorTheme((const char*)colorTemplate->ToBytes().data(), colorTemplate->ToBytes().size());
	deserializer.Parse(json);
	deserializer.Use(colorTheme, "DefaultValues");
	deserializer.Use(colorTheme, "BaseColorSchemes.Dark");
	deserializer.Use(colorTheme, "ColorSchemes.Steel");

	std::map<std::string, std::string> metaData;
	metaData["ThemeName"] = "Base";
	metaData["ThemeDisplayName"] = "Base";
	metaData["BaseColorScheme"] = "Dark";
	metaData["ColorScheme"] = "Steel";
	deserializer.Use(colorTheme, metaData);

	auto modul = System::Resource::ResourceManager::Instance()->GetResourceModul("33.NoesisGUIWindow");
	auto baseStyleRes = modul->GetResource("Theme/BaseStyle.xaml");
	if (!baseStyleRes)
		baseStyleRes = modul->CreateResourceObject("Theme/BaseStyle.xaml");
	baseStyleRes->Reset();
	baseStyleRes->LoadRawData((byte*)colorTheme.data(), colorTheme.size());

	//Noesis::Ptr<BaseComponent> root = Noesis::GUI::LoadXaml("pack://application:,,,/33.NoesisGUIWindow;/Theme/BaseStyle.xaml");
	//auto mResourceDictionary = NsDynamicCast<Noesis::Ptr<Noesis::ResourceDictionary>>(root);

	//auto merged = GetResources()->GetMergedDictionaries();
	//merged->Add(mResourceDictionary);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::App)
{
	NsMeta<Noesis::TypeId>("WorldClient.App");
}
