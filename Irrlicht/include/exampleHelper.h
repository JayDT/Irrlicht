// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __EXAMPLE_HELPER_H_INCLUDED__
#define __EXAMPLE_HELPER_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "path.h"
#include "buildin_data.h"

namespace irr
{
//#if defined(WIN32) and !defined(__MINGW32__)
//
//// We'll also define this to stop MSVC complaining about sprintf().
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#include <direct.h>
//#define GetCurrentDir _getcwd
//
//    inline void SetWorkingDirectory()
//    {
//        // Setup working dir
//        {
//            HMODULE hModule = GetModuleHandleW(NULL);
//            WCHAR path[MAX_PATH];
//            GetModuleFileNameW(hModule, path, MAX_PATH);
//
//            std::wstring wide(path);
//            std::string str(wide.begin(), wide.end());
//            while (str.back() != '\\')
//                str.pop_back();
//            SetCurrentDirectory(str.c_str());
//
//            char cCurrentPath[FILENAME_MAX];
//
//            if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
//            {
//                return; //errno
//            }
//
//            cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
//        }
//    }
//#endif

static io::path getExampleMediaPath()
{
//#if defined(WIN32) and !defined(__MINGW32__)
//    // Fix Visual Studio debugger working dir issues
//    //SetWorkingDirectory();
//    return io::path("media/");
//#endif
    return io::path(_BIN_OUTPUT_DIRECTORY) + "/media/";
}

} // end namespace irr

#endif
