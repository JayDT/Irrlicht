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

#include "ResourceDatabase.h"
#include "ResourceManager.h"
#include "ResourceObject.h"
#include "standard/client/DataSource_Standard.h"

using namespace System::Resource;

System::Resource::ResourceDatabase::ResourceDatabase(const char* modulName, uint64_t modulId)
    : mModulName(modulName)
    , mModulId(modulId)
{
}

ResourceObject * System::Resource::ResourceDatabase::CreateResourceObject(std::string const & name)
{
    ResourceObject * res = GetResource(name);
    if (res)
        return res;

    auto& newres = mResourceMap[name];
    newres = std::make_shared<ResourceObject>(name);
    return newres.get();
}

ResourceObject * System::Resource::ResourceDatabase::GetResource(std::string const & name) const
{
    auto res = mResourceMap.find(name);
    return res != mResourceMap.end() ? res->second.get() : nullptr;
}

void System::Resource::ResourceDatabase::ParseResx(ResxFile& modulResx, uint32_t mid)
{
	std::vector<byte> _data;
	auto modul = std::next(modulResx.ModulList.begin(), mid);
	for (const ResxFileListEntry& file : modul->FileList)
	{
		if (!file.Offset)
			break;

		modulResx.File->Seek(file.Offset, false);

		ResourceObject* resource = CreateResourceObject(file.Filename);
		if (!resource)
			continue;

		resource->Reset();

		do
		{
			ResxFileChunkHead chunk;
			*modulResx.File >> chunk.Size;
			if (!chunk.Size)
				break;

			_data.resize(chunk.Size);
			*modulResx.File >> chunk.EncodedSize;
			*modulResx.File >> chunk.DecodedSize;
			modulResx.File->Read(_data.data(), chunk.Size);

			resource->LoadCompressRawData(_data.data(), chunk.EncodedSize, chunk.DecodedSize);

		} while (true);
	}
}

System::Resource::ResxFile::~ResxFile()
{
	if (File)
		delete File;
}
