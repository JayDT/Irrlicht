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

#include "ResourceObject.h"

#include <zlib.h>

bool uncompress(void *dst, int total_size, void *src, int src_size)
{
    memset(dst, 0, total_size);

    z_stream c_stream;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    int z_res = inflateInit(&c_stream);

    c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = total_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;

    z_res = inflate(&c_stream, Z_FINISH);
    return z_res >= Z_OK;
}

System::Resource::ResourceObject::ResourceObject(const std::string& modulName)
    : mName(modulName)
{
}

void System::Resource::ResourceObject::Reset()
{
	mData.clear();
}

void System::Resource::ResourceObject::LoadCompressRawData(const uint8_t * src, const size_t src_size, const size_t total_size)
{
    size_t wpos = mData.size();
	mData.resize(mData.size() + total_size);
    bool success = uncompress((void*)& mData[wpos], total_size, (void*)src, src_size);
    System::ThrowIfFailed<System::NotSupportedException>(success);
}

void System::Resource::ResourceObject::LoadRawData(const uint8_t* src, const size_t src_size)
{
	size_t wpos = mData.size();
	mData.resize(mData.size() + src_size);
	memcpy((void*)& mData[wpos], src, src_size);
}

void System::Resource::ResourceObject::CreateResource()
{
}

std::string System::Resource::ResourceObject::ToString() const
{
    return std::string((const char*)mData.data(), mData.size());
}

std::vector<byte> const& System::Resource::ResourceObject::ToBytes() const
{
    return mData;
}

System::IO::MemoryStreamReader * System::Resource::ResourceObject::ToMemoryStreamReader() const
{
    auto stream = new System::IO::MemoryStreamReader((byte*)mData.data(), mData.size(), false);
    stream->FileName = mName;
    return stream;
}

std::shared_ptr<System::IO::MemoryStreamReader> System::Resource::ResourceObject::ToSharedMemoryStreamReader() const
{
    auto stream = std::make_shared<System::IO::MemoryStreamReader>((byte*)mData.data(), mData.size(), false);
    stream->FileName = mName;
    return stream;
}
