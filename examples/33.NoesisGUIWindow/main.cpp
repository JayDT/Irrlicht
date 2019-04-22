#include <signal.h>
#include "standard/Platform/Common.h"
#include "Log/Log.h"
#include "System/Resource/ResourceManager.h"
#include "App.xaml.h"
#include <NsApp/IrrNsGuiBindings.h>

// We'll also define this to stop MSVC complaining about sprintf().
#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

TC_API_IMPORT void Static_Init_CFramework();

#ifdef WIN32
void SetWorkingDirectory()
{
    // Setup working dir
    {
        HMODULE hModule = GetModuleHandleW(NULL);
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);

        std::wstring wide(path);
        std::string str(wide.begin(), wide.end());
        while (str.back() != '\\')
            str.pop_back();
        SetCurrentDirectory(str.c_str());

        char cCurrentPath[FILENAME_MAX];

        if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
        {
            return; //errno
        }

        cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

        System::Console()->WriteLine("The current working directory is %s\n", cCurrentPath);
    }
}
#endif

/// Handle termination signals
void _OnSignal(int s)
{
    switch (s)
    {
        case SIGTERM:
#ifdef _WIN32
        case SIGBREAK:
#endif
            System::Console()->WriteLine("SIGTERM");
        case SIGINT:
        {
            System::Console()->WriteLine("SIGINT");
            break;
        }

#ifndef _WIN32
        case SIGKILL:
        case SIGBUS:
#endif
        case SIGABRT:
        case SIGILL:
        case SIGFPE:
        case SIGSEGV:
        {
            break;
        }
    }

    signal(s, _OnSignal);
}

/// Define hook '_OnSignal' for all termination signals
void _HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
#endif
#ifndef _WIN32
    signal(SIGKILL, _OnSignal); //only unix
    signal(SIGBUS, _OnSignal);
#endif
    signal(SIGABRT, _OnSignal);
    signal(SIGILL, _OnSignal);
    signal(SIGFPE, _OnSignal);
    signal(SIGSEGV, _OnSignal);
}

/// Unhook the signals before leaving
void _UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
#ifdef _WIN32
    signal(SIGBREAK, 0);
#endif
#ifndef _WIN32
    signal(SIGKILL, 0); //only unix
    signal(SIGBUS, 0);
#endif
    signal(SIGABRT, 0);
    signal(SIGILL, 0);
    signal(SIGFPE, 0);
    signal(SIGSEGV, 0);
}

int main(int argc, char** argv)
{
    _HookSignals();

    setlocale(LC_ALL, "");
    // inherit locale from environment
    setlocale(LC_NUMERIC, "French_Canada.1252");
    //setlocale(LC_NUMERIC, "");

    /* initialize random seed: */
    srand(time(NULL));

#ifdef WIN32
    SetWorkingDirectory();
#endif

    System::Console()->WriteLine("      :::::::::  :::::::::: :::     :::      ::::::::::: ::::::::   ::::::::  ::: ");
    System::Console()->WriteLine("     :+:    :+: :+:        :+:     :+:          :+:    :+:    :+: :+:    :+: :+:  ");
    System::Console()->WriteLine("    +:+    +:+ +:+        +:+     +:+          +:+    +:+    +:+ +:+    +:+ +:+   ");
    System::Console()->WriteLine("   +#+    +:+ +#++:++#   +#+     +:+          +#+    +#+    +:+ +#+    +:+ +#+    ");
    System::Console()->WriteLine("  +#+    +#+ +#+         +#+   +#+           +#+    +#+    +#+ +#+    +#+ +#+     ");
    System::Console()->WriteLine(" #+#    #+# #+#          #+#+#+#            #+#    #+#    #+# #+#    #+# #+#      ");
    System::Console()->WriteLine("#########  ##########     ###              ###     ########   ########  ##########");

	System::Resource::ResourceManager::Instance()->LoadResources("Data");

	NoesisApp::CommandLine arguments;

	WorldClient::App::StaticInitializeComponents();

	auto _Application = Noesis::DynamicPtrCast<WorldClient::App>(Noesis::GUI::LoadXaml("pack://application:,,,/33.NoesisGUIWindow;/App.xaml"));
	if (_Application == nullptr)
	{
		NS_FATAL("File '%s' does not define a valid application file.", "App.xaml");
	}

	_Application->Initialize(arguments);
	_Application->Run();

    _UnhookSignals();
    return -1;
}
