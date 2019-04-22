#include <NsApp/IrrNsGuiBindings.h>
#include <NsProvider/IrrNsGuiResourceProvider.h>
#include <NsGui/IntegrationAPI.h>

#include "include/irrlicht.h"

namespace
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void InstallResourceProviders()
    {
        // Each time a resource (xaml, texture, font) is needed, the corresponding provider is invoked
        // to get a stream to the content. You must install a provider for each needed resource. There
        // are a few implementations available in the app framework (like LocalXamlProvider to load
        // from disk). For this sample, we are embedding needed resources in a C array.
        Noesis::GUI::SetXamlProvider(Noesis::MakePtr<NoesisApp::EmbeddedXamlProvider>());
        Noesis::GUI::SetFontProvider(Noesis::MakePtr<NoesisApp::EmbeddedFontProvider>());
        Noesis::GUI::SetTextureProvider(Noesis::MakePtr<NoesisApp::EmbeddedTextureProvider>());
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
PACKAGE_EXPORT void NsRegisterReflection_NoesisApp(Noesis::ComponentFactory* factory, bool doRegister)
{
	PACKAGE_REGISTER(Render, IrrRenderContext);
	PACKAGE_REGISTER(Render, IrrRenderDevice);
	PACKAGE_REGISTER(Application, Application);
	PACKAGE_REGISTER(Application, Window);
	PACKAGE_REGISTER(Application, IrrWindow);
}

extern "C"
void NoesisApp::Noesis_Init(Noesis::ErrorHandler NsErrorHandler, Noesis::LogHandler NsLogHandler, Noesis::MemoryCallbacks* NsAllocator)
{
    Noesis::GUI::Init(NsErrorHandler, NsLogHandler, NsAllocator);
    InstallResourceProviders();

	NsRegisterReflection_NoesisApp(nullptr, true);
}

extern "C"
void NoesisApp::Noesis_Shutdown()
{

}
