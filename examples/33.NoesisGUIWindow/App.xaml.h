#ifndef __WCLIENT_APP_H__
#define __WCLIENT_APP_H__

#include "NsApp/Application.h"
#include "NsApp/IrrWindow.h"
#include <NsCore/Ptr.h>

namespace irr
{
    struct SIrrlichtCreationParameters;
}

namespace System
{
	class ServiceContainer;
}

namespace NoesisApp
{
	class CommandLine;
	class IrrNsDeviceStub;
}

namespace WorldClient
{
    class App final : public NoesisApp::Application
    {
    private:

        bool m_running = true;

    public:
		App();
		virtual ~App();

		static void StaticInitializeComponents();
		void Initialize(const NoesisApp::CommandLine& arguments);
        void Run();

        static void LoadShaderCache(const char* filename);
        void SaveShaderCache(const char* filename) const;

        void LoggingHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message);

        void AddWindow(const char*);
        void AddWindow(NoesisApp::IrrWindow*);
        NoesisApp::IrrWindow* GetWindow(const char* wndName) const;

        static App* Instance();

    private:

		static void InitColorStyle();

		NS_DECLARE_REFLECTION(App, Application)
    };

	const irr::SIrrlichtCreationParameters& GetDefaultIrrDeviceParam();
}

#endif // !__WCLIENT_APP_H__
