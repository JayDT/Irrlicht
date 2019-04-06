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

#include "ResourceManager.h"
#include <regex>

using namespace System::Resource;

System::Resource::ResourceManager* System::Resource::ResourceManager::Instance()
{
    static System::Resource::ResourceManager _instance;
    return &_instance;
}

void System::Resource::ResourceManager::Initialize()
{
    //std::list<ResourceDatabase*> moduls;
    //while (!InitResourceModule.empty())
    //{
    //    moduls.push_back(InitResourceModule.front());
    //    InitResourceModule.pop();
    //}
	//
    //for (auto const& modul : moduls)
    //{
    //    modul->CreateResourceDescriptor();
    //}
	//
    //for (auto const& modul : moduls)
    //{
    //    modul->Initialize();
    //}
}

std::vector<std::string> System::Resource::ResourceManager::GetFileList(std::string const& pattern, std::string const& modulRef) const
{
	std::vector<std::string> list;

	const char* patternStr = pattern[0] == '/' ? &pattern[1] : pattern.c_str();

	std::regex re(patternStr);
	std::cmatch m;

	if (!modulRef.empty())
	{
		const char* modulStr = modulRef[0] == '/' ? &modulRef[1] : modulRef.c_str();
		ResourceDatabase* modul = GetResourceModul(modulStr);
		if (modul)
		{
			for (const auto& path : modul->GetResourceMap())
			{
				if (pattern.empty())
				{
					list.push_back(System::String::format("%s;/%s", modulStr, path.first.c_str()));
					continue;
				}

				if (std::regex_search(path.first.c_str(), m, re))
					list.push_back(System::String::format("%s;/%s", modulStr, path.first.c_str()));
			}
		}
		return list;
	}


	for (const auto& modul : mResourceModules)
	{
		for (const auto& path : modul.second->GetResourceMap())
		{
			if (pattern.empty())
			{
				list.push_back(System::String::format("%s;/%s", modul.first.c_str(), path.first.c_str()));
				continue;
			}

			if (std::regex_search(path.first.c_str(), m, re))
				list.push_back(System::String::format("%s;/%s", modul.first.c_str(), path.first.c_str()));
		}
	}

	return list;
}

ResourceDatabase * System::Resource::ResourceManager::GetResourceModul(std::string const& name) const
{
    auto modul = mResourceModules.find(name);
    return modul != mResourceModules.end() ? modul->second.get() : nullptr;
}

void System::Resource::ResourceManager::LoadResources(const char* resourceRoot)
{
	std::regex filter(R"(.*\.resx)");
	System::FileSystem::TFileList flist;
	System::FileSystem::GetFileList(resourceRoot, flist, false, &filter);

	for (auto file : flist)
	{
		ResxFile resx;
		LoadResxFile(file.string().c_str(), &resx);

		uint32_t mid = 0;
		for (auto& modul : resx.ModulList)
		{
			if (!strlen(modul.ModulEntry.ModulName))
				continue;

			ResourceDatabase* db = GetResourceModul(modul.ModulEntry.ModulName);
			if (!db)
			{
				db = mResourceModulesById[modul.ModulEntry.ModulId] = (mResourceModules[modul.ModulEntry.ModulName] = std::make_shared<ResourceDatabase>(modul.ModulEntry.ModulName, modul.ModulEntry.ModulId)).get();
			}

			db->ParseResx(resx, mid++);
		}
	}
}

void System::Resource::ResourceManager::LoadResxFile(const char* file, ResxFile* fileData)
{
	ResxFile _cache;
	if (!fileData)
		fileData = &_cache;

	System::IO::StandardDataSource fileMgr;
	fileData->File = fileMgr.OpenFile(file);
	if (!fileData->File)
		return;

	ResxHeader * modulHeader;
	do
	{
		fileData->ModulList.emplace_back();
		auto& modul = fileData->ModulList.back();
		memset(&modul.ModulEntry, 0, sizeof(modul.ModulEntry));
		modulHeader = &modul.ModulEntry;
		*fileData->File >> *modulHeader;
		if (modulHeader->Signature != 'RSXA')
			break;

		if (!modulHeader->FileListTable)
			break;

		fileData->File->Seek(modulHeader->FileListTable, false);
		size_t nextHEAD = fileData->File->Position() + modulHeader->NextHeader;

		ResxFileListEntry* fileEntry;
		do
		{
			modul.FileList.emplace_back();
			fileEntry = &modul.FileList.back();
			memset(fileEntry, 0, sizeof(fileEntry));
			*fileData->File >> *fileEntry;
			fileData->File->Seek(fileEntry->NextFileHeader, false);
		} while (fileEntry->NextFileHeader);

		fileData->File->Seek(nextHEAD, false);
	} while (modulHeader->NextHeader);
}

ResourceObject * System::Resource::ResourceManager::_findResource(System::URI const& uri, uint64_t modulID) const
{
    if (uri.GetDescriptor().m_scheme != "pack")
        return nullptr;

    const char* resourceRef = uri.GetDescriptor().m_path.LocalPath[0] == '/' ? &uri.GetDescriptor().m_path.LocalPath[1] : uri.GetDescriptor().m_path.LocalPath.c_str();

    ResourceDatabase * modul = nullptr;
    if (!uri.GetDescriptor().m_path.Assembly.empty())
    {
        const char* modulRef = uri.GetDescriptor().m_path.Assembly[0] == '/' ? &uri.GetDescriptor().m_path.Assembly[1] : uri.GetDescriptor().m_path.Assembly.c_str();

        modul = GetResourceModul(modulRef);
        if (!modul)
            return nullptr;

        auto resource = modul->GetResource(resourceRef);
        if (!resource)
            return nullptr;
        return resource;
    }

    auto iModul = mResourceModulesById.find(modulID);

    if (iModul != mResourceModulesById.end())
    {
        ResourceObject* resource = iModul->second->GetResource(resourceRef);
        if (resource)
            return resource;
    }

    for (auto db : mResourceModules)
    {
        ResourceObject* resource = db.second->GetResource(resourceRef);
        if (resource)
            return resource;
    }

    return nullptr;
}
