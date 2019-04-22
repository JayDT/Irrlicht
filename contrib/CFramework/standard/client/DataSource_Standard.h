#pragma once
#ifndef __C_DATASOURCE_STANARD_H__
#define __C_DATASOURCE_STANARD_H__

#include <stdio.h>
#include "standard/client/IDataSourceClient.h"

#include <cstring>
#include <array>
#include <iostream>
#include <streambuf>

namespace System
{
    namespace IO
    {
        struct StandardFile : IFileReader
        {
            FILE* _file_resource;

            StandardFile() : _file_resource(nullptr)
            {

            }

            virtual ~StandardFile()
            {
                Close();
            }

            virtual void Close() override
            {
                if (_file_resource)
                    fclose(_file_resource);
                _file_resource = nullptr;
            }

            virtual uint64 Size() override
            {
                int current_pos = ftell(_file_resource);
                fseek(_file_resource, 0, SEEK_END);
                int end_position = ftell(_file_resource);
                fseek(_file_resource, current_pos, SEEK_SET);
                return end_position;
            }

            virtual uint64 Position()
            {
                return ftell(_file_resource);
            }

            virtual unsigned long Seek(size_t offset, bool relative) override
            {
                return fseek(_file_resource, offset, relative ? SEEK_CUR : SEEK_SET);
            }

            virtual uint32_t Read(byte* lpBuffer, size_t size) override
            {
                DWORD readBytes = 0;
                return fread(lpBuffer, size, 1, _file_resource);
            }

            virtual byte* Read(size_t size) override
            {
                byte* dataBlock = new byte[size];
                if (Read(dataBlock, size) == size)
                    return dataBlock;
                delete[] dataBlock;
                return nullptr;
            }
        };

        struct StandardFileWR : IFileWriter
        {
            FILE* _file_resource;

            StandardFileWR() : _file_resource(nullptr)
            {

            }

            virtual ~StandardFileWR()
            {
                Close();
            }

            virtual void Close() override
            {
                if (_file_resource)
                    fclose(_file_resource);
                _file_resource = nullptr;
            }

            virtual uint64 Size() override
            {
                int current_pos = ftell(_file_resource);
                fseek(_file_resource, 0, SEEK_END);
                int end_position = ftell(_file_resource);
                fseek(_file_resource, current_pos, SEEK_SET);
                return end_position;
            }

            virtual uint64 Position()
            {
                return ftell(_file_resource);
            }

            virtual unsigned long Seek(size_t offset, bool relative) override
            {
                return fseek(_file_resource, offset, relative ? SEEK_CUR : SEEK_SET);
            }

            virtual uint32_t Write(byte * lpBuffer, size_t size) override
            {
                uint32_t readBytes = 0;
                return fwrite(lpBuffer, size, 1, _file_resource);
            }
        };

		struct MemoryStreamReader : IFileReader
		{
			uint8_t* mbuffer;
			size_t length;
			size_t position;
			bool free;

			MemoryStreamReader(uint8_t* buffer, size_t length, bool free)
				: mbuffer(buffer)
				, length(length)
				, position(0)
				, free(free)
			{
			}

			virtual ~MemoryStreamReader()
			{
				Close();
			}

			virtual void Close()
			{
				if (free)
					delete[]mbuffer;
			}

			virtual uint64 Size()
			{
				return length;
			}

			virtual uint64 Position()
			{
				return position;
			}

			virtual unsigned long Seek(size_t offset, bool relative)
			{
				if (relative)
					position += offset;
				else
					position = offset;
				if (length <= position)
					position = length - 1;
				return Position();
			}

			virtual uint32_t Read(byte * lpBuffer, size_t size)
			{
				size_t _size = std::min(size, std::max(size_t(0), length - position));
				memcpy(lpBuffer, &mbuffer[position], _size);
				return _size;
			}

			virtual byte* Read(size_t size)
			{
				byte* dataBlock = new byte[size];
				if (Read(dataBlock, size) == size)
					return dataBlock;
				delete[] dataBlock;
				return nullptr;
			}
		};

        class StandardDataSource : public IDataSource
        {
        public:
            virtual bool HasLocale(int locale) const { return false; }
            size_t GetLocaleCount() const override final { return 0; }
            const char* GetLocaleName(int locale) const override final { return nullptr; }

            IO::IFileReader* OpenFile(const char* filename, int locale = -1) override
            {
                FILE* f = fopen(filename, "rb");
                if (!f)
                    return nullptr;

                StandardFile* _file = new StandardFile;
                _file->_file_resource = f;
                _file->FileName = filename;
                return _file;
            }

            IO::IFileWriter* OpenFileToWrite(const char* filename, int locale = -1)
            {
                FILE* f = fopen(filename, "wb");
                if (!f)
                    return nullptr;

                StandardFileWR* _file = new StandardFileWR;
                _file->_file_resource = f;
                _file->FileName = filename;
                return _file;
            }

            void ListFiles(int locale, const char* pattern, IDataSource::TFileList& files) override
            {

            }
        };
    }
}

#endif
