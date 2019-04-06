/*
* Copyright (C) 2017-2018 Tauri JayD <https://www.>
* Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "standard/Platform/Common.h"
#include "Log/Log.h"

#include <signal.h>
#include <chrono>
#include <fstream>
#include <future>
#include <algorithm>
#include <iostream>

#include "ResourceBuilder.h"

std::map<std::string, std::string> commands;
std::map<std::string, std::string> cmdLine;

bool kIsInTestMode = false;

void AddCommandLineOptions()
{
	SWITCH_OPTION(TargetName);
	SWITCH_OPTION(SourceRoot);
	SWITCH_OPTION(InputSource);
	SWITCH_OPTION(OutputModuleSource);
	SWITCH_OPTION(OutputModuleFileDirectory);
	SWITCH_OPTION(ForceRebuild);
	SWITCH_OPTION(DisplayDiagnostics);
}

void ParseCommandLineArgs(int argc, char *argv[])
{
	//argc = 12;
	//argv = new char* [13]
	//{
	//	"e:/Project/WoWClient/moduls/Irrlicht/win/bin/Debug/rs.exe",
	//	"--target-name",
	//	"LibWoW",
	//	"--source-root",
	//	"e:/Project/WoWClient/moduls/Irrlicht/Irrlicht/RenderEngines/Direct3D11",
	//	"--in-source",
	//	"e:/Project/WoWClient/moduls/Irrlicht/win/Irrlicht/RenderEngines/Direct3D11/.Resource/Resource_LibWoW.dsc",
	//	"--out-source",
	//	"e:/Project/WoWClient/moduls/Irrlicht/win/LibWoW",
	//	"--out-dir",
	//	"e:/Project/WoWClient/moduls/Irrlicht/win/Irrlicht/RenderEngines/Direct3D11/.Resource",
	//	"--display-diagnostics",
	//	nullptr
	//};

	for (int i = 1; i < argc; i += 2)
	{
		auto swc = commands.find(argv[i]);
		if (swc != commands.end())
			cmdLine[swc->second] = argv[i + 1] ? argv[i + 1] : "";
		else
			cmdLine[argv[i]] = argv[i + 1] ? argv[i + 1] : "";
	}
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "French_Canada.1252");
    
    /* initialize random seed: */
    srand(time(NULL));

    AddCommandLineOptions();
    ParseCommandLineArgs(argc, argv);
    
    if (cmdLine.count("help"))
    {
        //std::cout << program << std::endl;
        return EXIT_SUCCESS;
    }
    
    System::Resource::ResourceBuilder::Parse(cmdLine);

    return EXIT_SUCCESS;
}
