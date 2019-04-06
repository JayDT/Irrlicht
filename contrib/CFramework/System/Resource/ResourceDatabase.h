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

#ifndef __CF_RESOURCE_DATABASE_H__
#define __CF_RESOURCE_DATABASE_H__

#include "System/Resource/ResourceObject.h"
#include "standard/misc.h"

namespace System::IO
{
	struct IFileReader;
}

namespace System::Resource
{
	PACK_BEGIN(1)
	struct ResxHeader
	{
		ResxHeader()
		{}

		uint32_t Signature;
		uint32_t Version;
		char ModulName[32];
		uint32_t FileListTable;
		uint32_t ModulId;
		uint32_t NextHeader;
	};

	struct ResxFileListEntry
	{
		ResxFileListEntry()
		{}

		uint32_t Type;
		char Filename[128];
		uint32_t Offset;
		uint32_t Size;
		uint32_t DecodedSize;
		uint32_t NextFileHeader;
	};

	struct ResxFileChunkHead
	{
		uint32 Size;
		uint32 EncodedSize;
		uint32 DecodedSize;
	};
	PACK_END

	struct ResxFileModul final
	{
		ResxHeader ModulEntry;
		std::list<ResxFileListEntry> FileList;
	};

	struct ResxFile final
	{
		~ResxFile();

		System::IO::IFileReader* File = nullptr;
		std::list<ResxFileModul> ModulList;
	};

    class TC_CFRAMEWORK_API ResourceDatabase final
    {
        typedef std::map<std::string, std::shared_ptr<ResourceObject>> ResourceMap;

        uint64_t mModulId;
        std::string mModulName;
        ResourceMap mResourceMap;

    public:

        ResourceDatabase(const char* modulName, uint64_t modulId = 0);
		~ResourceDatabase() = default;

        uint64_t GetModulId() const { return mModulId; }
        const char* Name() const { return mModulName.c_str(); }

        ResourceObject* CreateResourceObject(std::string const& name);

        ResourceObject* GetResource(std::string const& name) const;
		const ResourceMap& GetResourceMap() const { return mResourceMap; }

		void ParseResx(ResxFile& modulResx, uint32_t mid);
    };
}

#endif //!__CF_RESOURCE_MANAGER_H__