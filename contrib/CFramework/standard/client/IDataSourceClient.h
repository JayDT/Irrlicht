#ifndef __CLIENT_DATASOURCE_INTERFACE_H__
#define __CLIENT_DATASOURCE_INTERFACE_H__

#include "standard/Platform/Common.h"
#include "standard/filesystem.h"
#include "standard/threading.h"

namespace System
{
    namespace Memory
    {
        struct MemoryBlock
        {
            MemoryBlock() : ptr(NULL), size(0) {}
            MemoryBlock(uint8_t*p, uint32_t s) : ptr(p), size(s) {}
            ~MemoryBlock() { free(); }

            void alloc(uint32_t s) { size = s; ptr = new uint8_t[s]; }
            void free(void)
            {
                if (ptr)
                    delete[] ptr;
            }

            uint8* operator[](int seek)
            {
                if (seek >= size)
                    throw std::runtime_error("invalid operation");
                return &ptr[seek];
            }

            uint8_t*ptr;
            uint32_t size;
        };
    }

    namespace IO
    {
        struct IFileReader
        {
            std::string FileName;

            virtual ~IFileReader() {}
            virtual unsigned long Seek(size_t offset, bool relative) = 0;
            virtual uint32_t Read(uint8_t* lpBuffer, size_t size) = 0;
            virtual uint8_t* Read(size_t size) = 0;
            virtual uint64_t Size() = 0;
            virtual void Close() = 0;
            virtual uint64_t Position() = 0;

            int Read(Memory::MemoryBlock& block)
            {
                return Read(block.ptr, block.size);
            }

            uint32_t Read(Memory::MemoryBlock* mem, size_t size)
            {
                return mem->size = Read(mem->ptr, size);
            }

            uint32_t ReadToEnd(Memory::MemoryBlock* mem)
            {
                return mem->size = Read(mem->ptr, Size());
            }

            template<typename T, unsigned int Size>
            IFileReader& operator >> (T(&buffer)[Size])
            {
                Read((uint8_t*)buffer, sizeof(T) * Size);
                return *this;
            }

            template<typename T>
            IFileReader& operator >> (T& buffer)
            {
                Read((uint8_t*)& buffer, sizeof(T));
                return *this;
            }
        };

        struct IFileWriter
        {
            std::string FileName;

            virtual ~IFileWriter() {}
            virtual unsigned long Seek(size_t offset, bool relative) = 0;
            virtual uint32_t Write(uint8_t* lpBuffer, size_t size) = 0;
            virtual uint64_t Size() = 0;
            virtual void Close() = 0;
            virtual uint64_t Position() = 0;

            int Write(Memory::MemoryBlock& block)
            {
                return Write(block.ptr, block.size);
            }

            uint32_t Write(Memory::MemoryBlock* mem, size_t size)
            {
                return mem->size = Write(mem->ptr, size);
            }

            template<typename T, unsigned int Size>
            IFileWriter& operator << (T(&buffer)[Size])
            {
                Write((uint8_t*)buffer, sizeof(T) * Size);
                return *this;
            }

            template<typename T>
            IFileWriter& operator << (const T& buffer)
            {
                Write((uint8_t*)& buffer, sizeof(T));
                return *this;
            }
        };

        struct IDataSource
        {
            typedef std::list<std::string> TFileList;

            virtual ~IDataSource() {}
            virtual bool HasLocale(int locale) const = 0;
            virtual size_t GetLocaleCount() const = 0;
            virtual const char* GetLocaleName(int locale) const = 0;
            virtual uint32_t GetFileID(const char* filename) { return 0; }
            virtual IO::IFileReader* OpenFile(const char* filename, int locale = -1) = 0;
            virtual void ListFiles(int locale, const char* pattern, IDataSource::TFileList& files) = 0;
            //virtual IO::IFileReader* CreateMemoryStream(const char* filename, int length) override
        };
    }
}

#endif