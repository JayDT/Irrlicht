#pragma once

#ifndef _WC_PCH_COMMON_H_
#define _WC_PCH_COMMON_H_

// Required for Boost (1.64)
#define _HAS_AUTO_PTR_ETC	1

namespace System::Reflection::Assembly
{
    class AssemblyModulworldclient;
}

#define NS_MINIMUM_LOG_LEVEL 0 // NS_LOG_LEVEL_TRACE

#include <future>
#include <atomic>

#include <standard/Platform/Common.h>
#include "standard/BasicPrimities.h"
#include <standard/Errors.h>
#include <standard/events.h>
#include <standard/callback.h>
#include <standard/misc.h>

#include <standard/client/DataSource_Standard.h>

#include <Noesis_pch.h>

#endif // _WC_PCH_COMMON_H_
