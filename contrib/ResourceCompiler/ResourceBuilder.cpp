/*
* Copyright (C) 2017-2018 Tauri JayD <https://www.>
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

#include "ResourceBuilder.h"
#include "ResourceCompiler.h"
#include "System/Resource/ResourceDatabase.h"
#include <standard/client/DataSource_Standard.h>

#include <chrono>
#include <fstream>
#include <future>
#include <algorithm>

#include <filesystem>
#include <regex>

#include <Wrappers/Mustache.h>

const char* kVersion = "0.0.1a";

void ConvertFileName(std::string const& _filename, std::string& resourcefile);

//namespace MustacheExt
//{
//    void FatalError(const std::string &error);
//    void LoadText(const std::string &filename, std::string &output);
//    Mustache::Mustache<std::string> LoadTemplate(const std::string & name);
//    void WriteText(const std::string &filename, const std::string &text, bool writeIfChecksumDif);
//    std::filesystem::path MakeRelativePath(const std::filesystem::path &from, const std::filesystem::path &to);
//    std::filesystem::path change_extension(const std::filesystem::path & p, const std::filesystem::path & new_extension);
//}

namespace
{
    const std::regex kSpecialCharsRegex("[^a-zA-Z0-9]+");
}

System::IO::StandardDataSource fileSystem;
ResourceOptions options;
Mustache::Mustache<std::string>* fileTemplate;

System::Concurrent::Queue<System::Resource::ResourceBuilder*> System::Resource::ResourceBuilder::_activeTasks;
System::Concurrent::Queue<System::Resource::ResourceBuilder*> System::Resource::ResourceBuilder::_resultTasks;

System::Resource::ResourceBuilder::ResourceBuilder(ResourceOptions options, std::string const& sourcefile)
    : _options(options)
{
    _options.inputSourceFile = sourcefile;
    AddToGroup(this);
}

void System::Resource::ResourceBuilder::Parse(const std::map<std::string, std::string>& cmdLine)
{
    extern bool kIsInTestMode;

    options.forceRebuild =
        cmdLine.count(kSwitchForceRebuild) > 0 || kIsInTestMode;

    options.displayDiagnostics =
        cmdLine.count(kSwitchDisplayDiagnostics) > 0 /*|| kIsInTestMode*/;

	options.targetName = cmdLine.find(kSwitchTargetName) != cmdLine.end() ? cmdLine.find(kSwitchTargetName)->second : "";

    options.sourceRoot = cmdLine.find(kSwitchSourceRoot) != cmdLine.end() ? cmdLine.find(kSwitchSourceRoot)->second : "";

    options.inputSourceFile = cmdLine.find(kSwitchInputSource) != cmdLine.end() ? cmdLine.find(kSwitchInputSource)->second : "";

    options.outputModuleSource = cmdLine.find(kSwitchOutputModuleSource) != cmdLine.end() ? cmdLine.find(kSwitchOutputModuleSource)->second : "";

    options.outputModuleFileDirectory = cmdLine.find(kSwitchOutputModuleFileDirectory) != cmdLine.end() ? cmdLine.find(kSwitchOutputModuleFileDirectory)->second : "";

	auto ifiles = cmdLine.find(kSwitchInputSource) != cmdLine.end() ? cmdLine.find(kSwitchInputSource)->second : "";
    std::ifstream includesFile(ifiles);

    std::map<std::string, ResourceBuilder*> GroupsResults;
    std::list<std::string> resource_files;
    std::string sourcefile;
    while (std::getline(includesFile, sourcefile))
        resource_files.push_back(sourcefile);

    for (std::string const& sourcefile : resource_files)
    {
        //std::filesystem::path srcfn(sourcefile.c_str());

        std::string resultFile;
        ConvertFileName(sourcefile, resultFile);

        // ToDo: rewrite this. Implement resource types
        ResourceBuilder*& groupParent = GroupsResults[resultFile];
        if (!groupParent)
        {
            groupParent = new ResourceBuilder(options, sourcefile);
            _activeTasks.push(groupParent);
        }
        else
        {
            groupParent->AddToGroup(new ResourceBuilder(options, sourcefile));
        }
    }

    //NOTE: can be async after
    ResourceBuilder* builder;
    while (_activeTasks.pop(builder))
    {
        for (ResourceBuilder* element : builder->GetGroupElements())
            element->Compile();
        _resultTasks.push(builder);
    }

    Generate();
}

void System::Resource::ResourceBuilder::Compile()
{
    std::string fileName = System::String::format("%s/%s", _options.sourceRoot.c_str(), _options.inputSourceFile.c_str());

    System::IO::IFileReader* resourceFile = fileSystem.OpenFile(fileName.c_str());

    _name = std::regex_replace(
        _options.inputSourceFile,
        kSpecialCharsRegex,
        "_"
    );

    uint64 inputsize = resourceFile->Size();

    while (inputsize > 0)
    {
        size_t chuckSize = std::min(size_t(inputsize), ResourceCompiler::MaxChunkSize);
        inputsize -= chuckSize;
        std::vector<byte>* buffer = new std::vector<byte>(chuckSize);
        resourceFile->Read(buffer->data(), chuckSize);
        _compiler.m_input.push_back(buffer);
    }

    delete resourceFile;

    _compiler.Compile(_resourceType);
}

void ConvertFileName(std::string const& _filename, std::string& resourcefile)
{
    System::String normalizedFn = _filename;
    std::filesystem::path filePath(normalizedFn.Replace('/', '\\').ToLowerFirst().c_str());

    auto fn = filePath.filename();
    auto filename = std::wstring(L"Resource_") + fn.c_str();
	resourcefile = std::filesystem::path(filename).replace_extension(".cpp").string();
}

size_t System::Resource::ResourceBuilder::GenerateFile(FILE* fresx)
{
	size_t HeadPosition = 0;
	bool haveBinary = false;
    for (ResourceBuilder* element : _group)
    {
		HeadPosition = ftell(fresx);
		System::Resource::ResxFileListEntry fileHead;
		memcpy(fileHead.Filename, element->_options.inputSourceFile.c_str(), std::min(uint32_t(element->_options.inputSourceFile.size()), 127u));
		fileHead.Filename[std::min(uint32_t(element->_options.inputSourceFile.size()), 127u)] = 0;
		fileHead.Type = uint32(_resourceType);
		fileHead.DecodedSize = 0;
		fileHead.Size = 0;
		fileHead.Offset = 0;
		fileHead.NextFileHeader = 0;

		fwrite(&fileHead, sizeof(System::Resource::ResxFileListEntry), 1, fresx);
		fileHead.Offset = ftell(fresx);

		uint32 nextChunkSize = 0;
		for (size_t id = 0; id < element->_compiler.m_output.size(); ++id)
		{
			fileHead.Size += nextChunkSize = element->_compiler.m_output[id].second->size();
			fwrite(&nextChunkSize, sizeof(uint32), 1, fresx);
			nextChunkSize = element->_compiler.m_output[id].first;
			fwrite(&nextChunkSize, sizeof(uint32), 1, fresx);
			fileHead.DecodedSize += nextChunkSize = element->_compiler.m_input[id]->size();
			fwrite(&nextChunkSize, sizeof(uint32), 1, fresx);

			byte* data = element->_compiler.m_output[id].second->data();
			fwrite(data, element->_compiler.m_output[id].second->size(), 1, fresx);
		}

		nextChunkSize = 0;
		fwrite(&nextChunkSize, sizeof(uint32), 1, fresx);

		size_t NextHeadPosition = ftell(fresx);
		fseek(fresx, HeadPosition + offsetof(System::Resource::ResxFileListEntry, NextFileHeader), SEEK_SET);
		fileHead.NextFileHeader = NextHeadPosition;
		fwrite(&fileHead.NextFileHeader, sizeof(uint32), 1, fresx);

		fseek(fresx, HeadPosition + offsetof(System::Resource::ResxFileListEntry, Size), SEEK_SET);
		fwrite(&fileHead.Size, sizeof(uint32), 1, fresx);

		fseek(fresx, HeadPosition + offsetof(System::Resource::ResxFileListEntry, DecodedSize), SEEK_SET);
		fwrite(&fileHead.DecodedSize, sizeof(uint32), 1, fresx);

		fseek(fresx, HeadPosition + offsetof(System::Resource::ResxFileListEntry, Offset), SEEK_SET);
		fwrite(&fileHead.Offset, sizeof(uint32), 1, fresx);

		fseek(fresx, NextHeadPosition, SEEK_SET);
    }

	return HeadPosition;
}

void System::Resource::ResourceBuilder::Generate()
{
	std::filesystem::path sourcefile(options.outputModuleFileDirectory);
	sourcefile /= (options.targetName + ".resx");

	/// -----------------------------------------
	std::filesystem::create_directory(sourcefile.parent_path());

	System::Resource::ResxHeader reshead;
	memcpy(reshead.ModulName, options.targetName.c_str(), std::min(uint32_t(options.targetName.size()), 31u));
	reshead.ModulName[std::min(uint32_t(options.targetName.size()), 31u)] = 0;
	reshead.Signature = 'RSXA';
	reshead.Version = 0;
	reshead.FileListTable = 0;
	reshead.ModulId = options.ModulId;
	reshead.NextHeader = 0;

	FILE* fresx = fopen(sourcefile.string().c_str(), "wb");

	fwrite(&reshead, sizeof(System::Resource::ResxHeader), 1, fresx);
	reshead.FileListTable = ftell(fresx);

	uint32 lastFileHead = 0;
	ResourceBuilder* builder;
    while (_resultTasks.pop(builder))
    {
		lastFileHead = builder->GenerateFile(fresx);
    }

	if (!!lastFileHead)
	{
		uint32_t endNextFileHeader = 0;
		fseek(fresx, lastFileHead + offsetof(System::Resource::ResxFileListEntry, NextFileHeader), SEEK_SET);
		fwrite(&endNextFileHeader, sizeof(uint32), 1, fresx);

		fseek(fresx, offsetof(System::Resource::ResxHeader, FileListTable), SEEK_SET);
		fwrite(&reshead.FileListTable, sizeof(uint32), 1, fresx);
	}

	fflush(fresx);
	fclose(fresx);
}
