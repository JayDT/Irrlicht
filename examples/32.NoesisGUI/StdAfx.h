#pragma once

#ifndef _WC_PCH_COMMON_H_
#define _WC_PCH_COMMON_H_

// Required for Boost (1.64)
#ifdef WIN32
#define _HAS_AUTO_PTR_ETC	1
#endif

#define NS_MINIMUM_LOG_LEVEL 0 // NS_LOG_LEVEL_TRACE

#include <future>
#include <atomic>

#include <Noesis_pch.h>

#endif // _WC_PCH_COMMON_H_
