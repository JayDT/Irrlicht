#include "App.xaml.h"

#include <NsCore/Delegate.h>
#include <NsGui/UIElementEvents.h>
#include <NsCore/Kernel.h>
#include <NsCore/Error.h>
#include <NsCore/Log.h>
#include <NsCore/StringUtils.h>
#include <NsApp/CommandLine.h>

#include <NsApp/IrrNsGuiBindings.h>

#include "Log/Log.h"
#include "standard/Timer.h"
#include "standard/String/atoi_imp.h"
#include "Threading/Threading.h"

#include "include/irrlicht.h"
#include "standard/BasicPrimities.h"
#include <System/BaseTypes.h>
#include "UI/MainMenu.xaml.h"
#include "Config/Config.h"

using namespace irr;
using namespace WorldClient;

#include "NsImpl/IrrRenderContext.h"
#include "NsImpl/IrrOsDeviceContext.h"
#include "NsApp/Window.h"
#include "UI/Extension/ElementExtensions.h"
#include "UI/Extension/ViewModel.h"

namespace
{
	IrrlichtDevice* gDevice;
    WorldClient::App* gInstance;
    NoesisApp::CommandLine gCommandLine;

	void LogHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message)
	{
		gInstance->LoggingHandler(file, line, level, channel, message);
	};

	void ErrorHandler(const char* file, uint32_t line, const char* message, bool fatal)
	{
		char out[512];
		static const char* prefixes[] = { "E", "F" };
		const char* prefix = prefixes[fatal ? 1 : 0];
		Noesis::String::FormatBuffer(out, sizeof(out), "[NOESIS/%s] %s\n", prefix, message);
		System::Console()->WriteErrorLine(out);
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, App)
{
	NS_REGISTER_COMPONENT(App)

	NsRegisterComponent<WorldClient::ElementExtensions>();
	NsRegisterComponent<Noesis::EnumConverter<WorldClient::State>>();
	//NsRegisterComponent<WorldClient::StartMenu>();
	//NsRegisterComponent<WorldClient::SettingsMenu>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PACKAGE_EXPORT void NsRegisterReflection_WorldClient(Noesis::ComponentFactory* factory, bool doRegister)
{
	PACKAGE_REGISTER(WorldClient, App);
	PACKAGE_REGISTER(WorldClient, MainMenu);
	PACKAGE_REGISTER(WorldClient, MainWindow);
	PACKAGE_REGISTER(WorldClient, OptionSelector);
	PACKAGE_REGISTER(WorldClient, StartMenu);
	PACKAGE_REGISTER(WorldClient, SettingsMenu);
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
    NoesisApp::Noesis_Init(&ErrorHandler, &LogHandler, nullptr);
	NsRegisterReflection_WorldClient(nullptr, true);
}

void WorldClient::App::Initialize(const NoesisApp::CommandLine& arguments)
{
    SIrrlichtCreationParameters p;
    p.DriverType = video::EDT_VULKAN; // EDT_DIRECT3D11
    p.WindowSize = irr::core::dimension2d<u32>(1280, 720);
    p.Bits = (u8)16;
    p.Fullscreen = false;
    p.Stencilbuffer = false;
    p.Vsync = false;
    p.EventReceiver = nullptr;
    p.AntiAlias = 1;

    gDevice = createDeviceEx(p);
	SetIrrlichDevice(gDevice);

	mMainDisplay = NoesisApp::CreateDisplay(gDevice);
	Init(mMainDisplay, arguments);

	CreateNsWindow("pack://application:,,,/32.NoesisGUI;/UI/MainWindow.xaml");
}

void WorldClient::App::StartWoWEditModul()
{
}

void WorldClient::App::Run()
{
    irr::IrrlichtDevice* _device = gDevice;

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

        if (_device->run())
        {
            _device->getVideoDriver()->beginScene();

			Tick(_highResTime);
        
            _device->getVideoDriver()->endScene();
        }

		Idle(_highResTime);
        
        if (_passtimediff <= 5 + prevSleepTime)
        {
            prevSleepTime = 5 + prevSleepTime - _passtimediff;
            if (((int32)prevSleepTime) < 0)
                prevSleepTime = 0;
        }
        
        auto fps = _device->getVideoDriver()->getFPS();
        
        if (_prevFps != fps)
        {
            _debugTitle.clear();
            _debugTitle = L"IrrLicht [";
            _debugTitle += _device->getVideoDriver()->getName();
            _debugTitle += L"] FPS: [";
        
            _debugTitle += std::to_wstring(fps);
            _debugTitle += L"] Update Time: ";
            _debugTitle += std::to_wstring(_passtimediff);
            _device->setWindowCaption(_debugTitle.c_str());
        
            _prevFps = fps;
        }
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
        Noesis::String::FormatBuffer(out, sizeof(out), "[NOESIS/%s] %s\n", prefix, message);

		//if (level > 2)
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

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::App)
{
	NsMeta<Noesis::TypeId>("WorldClient.App");
}
