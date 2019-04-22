#ifndef __IRR_NOESIS_H__
#define __IRR_NOESIS_H__

#include <Noesis_pch.h>
#include <include/irrlicht.h>

#ifdef _IRR_WINDOWS_API_
#ifdef NS_IRR_NOESISAPP_API
#ifdef TC_API_EXPORT_NOESISAPP
#define NS_IRR_NOESIS_API __declspec(dllexport)
#else
#define NS_IRR_NOESIS_API __declspec(dllimport)
#endif
#else
#define NS_IRR_NOESIS_API
#endif
#else // _IRR_WINDOWS_API_
// Force symbol export in shared libraries built with gcc.
#if (__GNUC__ >= 4) && defined(NS_IRR_NOESISAPP_API) && defined(TC_API_EXPORT_NOESISAPP)
#define NS_IRR_NOESIS_API __attribute__ ((visibility("default")))
#else
#define NS_IRR_NOESIS_API
#endif
#endif

template<size_t N>
constexpr size_t strlen_(const char(&data)[N]) noexcept {
	return N - 1;
}

#define NS_REGISTER_UI_CONTROL(VAR, NAME) VAR = FindName<std::remove_pointer<decltype(VAR)>::type>(NAME)

#define NS_REGISTER_UI_EVENT(UICLASS, EVENT, METHOD) \
if (strlen(event) == strlen_(#EVENT) && !strcmp(event, #EVENT) && strlen(handler) == strlen_(#METHOD) && !strcmp(handler, #METHOD))\
{\
	Noesis::DynamicCast<UICLASS*>(source)->EVENT() += Noesis::MakeDelegate(this, &SelfClass::METHOD); \
}

#endif