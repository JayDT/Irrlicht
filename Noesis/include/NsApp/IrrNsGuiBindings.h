#ifndef __GUI_GUIWRAPPER_H__
#define __GUI_GUIWRAPPER_H__

#include <NsCore/CompilerSettings.h>
#include <NsCore/ComponentFactory.h>
#include <NsCore/Log.h>
#include <NsCore/Memory.h>
#include <NsApp/IrrNoesis.h>

#ifdef NS_MULTIPLE_DLLS
#define PACKAGE_EXPORT extern "C" NS_DLL_EXPORT
#else
#define PACKAGE_EXPORT extern "C"
#endif

#define PACKAGE_REGISTER(MODULE, PACKAGE) \
    void NsRegisterReflection##MODULE##PACKAGE(Noesis::ComponentFactory*, bool); \
    NsRegisterReflection##MODULE##PACKAGE(factory, doRegister)

#define PACKAGE_INIT(MODULE, PACKAGE) \
    void NsInitPackage##MODULE##PACKAGE(); \
    NsInitPackage##MODULE##PACKAGE()

#define PACKAGE_SHUTDOWN(MODULE, PACKAGE) \
    void NsShutdownPackage##MODULE##PACKAGE(); \
    NsShutdownPackage##MODULE##PACKAGE()

////////////////////////////////////////////////////////////////////////////////////////////////////
#define PACKAGE_REGISTER_REFLECTION(module, package) \
    extern void NsRegisterReflection##module##package( \
    Noesis::ComponentFactory* factory, bool registerComponents) \

namespace NoesisApp
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Noesis kernel management
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //@{
    extern "C" NS_IRR_NOESIS_API void Noesis_Init(Noesis::ErrorHandler NsErrorHandler, Noesis::LogHandler NsLogHandler, Noesis::MemoryCallbacks* NsAllocator);
    extern "C" NS_IRR_NOESIS_API void Noesis_Shutdown();
}

#endif
