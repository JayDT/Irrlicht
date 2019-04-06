#ifndef __WCLIENT_APP_H__
#define __WCLIENT_APP_H__

#include "NsApp/Application.h"
#include <NsCore/Ptr.h>

namespace NoesisApp
{
	class CommandLine;
	class IrrNsDeviceStub;
}

namespace WorldClient
{
    class App : public NoesisApp::Application
    {
    private:

        bool m_running = true;

    public:
		App();
		virtual ~App();

		static void StaticInitializeComponents();
		void Initialize(const NoesisApp::CommandLine& arguments);
        void StartWoWEditModul();
        void Run();

        void LoggingHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message);

    private:

		NS_DECLARE_REFLECTION(App, Application)
    };
}

#endif // !__WCLIENT_APP_H__
