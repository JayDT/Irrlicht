// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_NULL_SHADER_H_INCLUDED__
#define __C_VIDEO_NULL_SHADER_H_INCLUDED__

#include "IVideoDriver.h"

namespace irr
{
    namespace video
    {
        class IHardwareBuffer;
        struct IShader;
        struct VertexDeclaration;

        enum E_ShaderTypes
        {
            EST_VERTEX_SHADER,
            EST_FRAGMENT_SHADER,
            EST_GEOMETRY_SHADER,
            EST_HULL_SHADER,
            EST_DOMAIN_SHADER,
            EST_COMPUTE_SHADER,
            EST_TESSELLATION_SHADER,
            EST_HIGH_LEVEL_SHADER,
        };

        enum E_ShaderVersion
        {
            ESV_GLSL_ASM,
            ESV_GLSL_HIGH_LEVEL,
            ESV_HLSL_HIGH_LEVEL
        };

#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_OGL_VALUE 0xffffffff

        enum E_ShaderVariableType
        {
            ESVT_UNKNOWN,
            ESVT_ATTRIBUTE,
            ESVT_UNIFORM,
            ESVT_CONSTANT,
            ESVT_INPUT_STREAM
        };

        struct ShaderInitializerEntry
        {
            struct StageDesc
            {
                System::IO::IFileReader* DataStream;
                core::stringc EntryPoint;
                core::stringc ShaderModel;
                std::vector<IShaderDataBuffer*> Buffers;
                E_GPU_SHADING_LANGUAGE ShadingLang;
                E_ShaderTypes ShaderStageType;
                size_t DataStreamOffset = 0;
            };

            std::vector<StageDesc*> mStages;

            ~ShaderInitializerEntry()
            {
                for (auto stage : mStages)
                    delete stage;
            }

            StageDesc* AddShaderStage(
                System::IO::IFileReader* DataStream,
                E_ShaderTypes ShaderStageType,
                const c8* EntryPoint = nullptr,
                const c8* ShaderModel = nullptr,
                E_GPU_SHADING_LANGUAGE ShadingLang = E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT)
            {
                StageDesc* stage = new StageDesc;
                stage->DataStream = DataStream;
                stage->ShaderStageType = ShaderStageType;
                stage->EntryPoint = EntryPoint;
                stage->ShaderModel = ShaderModel;
                stage->ShadingLang = ShadingLang;
                mStages.push_back(stage);
                return stage;
            }
        };

        struct BufferVariableMemoryInfo
        {
            size_t Offset;
            size_t MemorySize;
            size_t ElementSize;
        };

        struct ShaderVariableDescriptor
        {
            ShaderVariableDescriptor()
            {
                m_location = 0;
                m_size = 0;
                m_length = 0;
                m_type = 0;
                m_class = 0;
                m_divisor = -1;
                m_shaderIndex = 0;
                m_variableType = 0;
            }

            s32 m_location;
            u16 m_class;
            u16 m_size;
            u16 m_length;
            u8 m_type;
            u8 m_shaderIndex;
            u32 m_variableType;
            s32 m_divisor;
            std::string m_name;
            std::string m_semantic;
        };

        struct IShaderDataBufferElement
        {
            struct ShaderDataBufferElementExepction : public std::exception
            {
                ShaderDataBufferElementExepction(const char* message) : std::exception(message)
                {

                }
            };

            ShaderVariableDescriptor const* m_description;

            virtual ~IShaderDataBufferElement()
            {

            }

            ShaderVariableDescriptor const* getDescription() const { return m_description; }
            virtual u16 getShaderLocation() const { return m_description->m_location; }
            virtual void* getShaderValues() const = 0;
            virtual u32 getShaderValueCount() const = 0;
            virtual size_t getValueSizeOf() const = 0;
            virtual bool isChanged() const = 0;
            virtual void setDirty() = 0;
            virtual void setUpdated() = 0;

            virtual void retarget(void* memoryBlock, size_t count) {}

        protected:

            IShaderDataBufferElement() : m_description(nullptr)
            {
            }

            explicit IShaderDataBufferElement(ShaderVariableDescriptor const* description) : m_description(description)
            {
            }

            void Initialize(ShaderVariableDescriptor const* description)
            {
                m_description = description;
            }
        };

        template<typename T>
        struct ShaderDataBufferElementObject : public IShaderDataBufferElement
        {
            T m_values;
            //u8* m_memoryBlock;

            bool m_changed : 1;

            explicit ShaderDataBufferElementObject(ShaderVariableDescriptor const* description) : IShaderDataBufferElement(description), m_changed(true), m_values(T())
            {
                //m_memoryBlock = &description->DataBuffer[constantDecl.offset];
            }

            virtual bool isChanged() const { return m_changed; }
            virtual void setDirty() { m_changed = true; }
            virtual void setUpdated() { m_changed = false; }

            void setShaderValues(T const& data)
            {
                if (m_values != data)
                {
                    m_values = data;
                    m_changed = true;
                }
            }

            ShaderDataBufferElementObject<T>* operator=(T const& data)
            {
                setShaderValues(data);
                return (ShaderDataBufferElementObject<T>*)this;
            }

            //T& GetValueRef() { return (T&)m_memoryBlock; }
            virtual void* getShaderValues() const { return (void*)&m_values; }
            virtual u32 getShaderValueCount() const { return 1; }
            virtual size_t getValueSizeOf() const { return sizeof(T); }
        };

        template<typename T>
        struct ShaderDataBufferElementArray : public IShaderDataBufferElement
        {
            std::vector<T> m_values;
            bool m_changed : 1;

            explicit ShaderDataBufferElementArray(ShaderVariableDescriptor const* description) : IShaderDataBufferElement(description), m_changed(true)
            {
            }

            // TODO: implement later or set manual
            virtual bool isChanged() const { return m_changed; }
            virtual void setDirty() { m_changed = true; }
            virtual void setUpdated() { m_changed = false; }

            void ReAllocBuffer(int size)
            {
                m_values.reserve(size);
            }

            void ResetShaderValues()
            {
                m_changed = true;
                m_values.resize(0);
            }

            void PushShaderValues(T const& data)
            {
                m_changed = true;
                m_values.push_back(data);
            }

            void MoveShaderValue(T const&& data)
            {
                m_changed = true;
                m_values.push_back(data);
            }

            T& Emplace()
            {
                m_changed = true;
                m_values.emplace_back();
                return m_values.back();
            }

            ShaderDataBufferElementObject<T>* operator+=(T const& data)
            {
                PushShaderValues(data);
                return (ShaderDataBufferElementObject<T>*)this;
            }

            virtual void* getShaderValues() const { return (void*)m_values.data(); }
            virtual u32 getShaderValueCount() const { return m_values.size(); }
            virtual size_t getValueSizeOf() const { return sizeof(T); }
        };

        struct IRRLICHT_API IShaderDataBuffer
        {
            enum E_UPDATE_TYPE
            {
                EUT_PER_FRAME,
                EUT_PER_FRAME_PER_MESH,
                EUT_PER_FRAME_PER_MATERIAL,
                EUT_MAX_VALUE,
            };

            enum E_UPDATE_FLAGS
            {
                EUF_COMMIT_VALUES = 0x0000001,
                EUF_CUSTOM_INPUT_VALUES = 0x0000002,
            };

            typedef core::array<IShaderDataBufferElement*> BufferDataArray;

            BufferDataArray m_bufferDataArray;
            u32 mChangedID;
            E_UPDATE_TYPE m_updateType : 8;

            explicit IShaderDataBuffer(IShaderDataBuffer::E_UPDATE_TYPE updateType) : mChangedID(1), m_updateType(updateType)
            {

            }

            virtual ~IShaderDataBuffer()
            {
                Invalidate();
            }

            void Invalidate()
            {
                for (u32 i = 0; i != m_bufferDataArray.size(); ++i)
                    delete m_bufferDataArray[i];
                m_bufferDataArray.clear();
            }

            void AddBufferElement(IShaderDataBufferElement* databuf)
            {
                m_bufferDataArray.push_back(databuf);
            }

            bool isChanged() const
            {
                for (u32 i = 0; i != m_bufferDataArray.size(); ++i)
                    if (m_bufferDataArray[i]->isChanged())
                        return true;
                return false;
            }

            virtual void Clear() {}
            virtual bool AddDataFromSceneNode(scene::ISceneNode*, irr::scene::IMesh*) { return true; }

            E_UPDATE_TYPE getUpdateType() const { return m_updateType; }

            virtual void setDirty();

            virtual void InitializeFormShader(IShader* gpuProgram, void* Descriptor) = 0;
            virtual void CommitBuffer(IShader* gpuProgram, IHardwareBuffer* buffer = nullptr);
            virtual void UpdateBuffer(IShader* gpuProgram, irr::scene::IMeshBuffer* meshBuffer, irr::scene::IMesh* mesh, irr::scene::ISceneNode* node, u32 updateFlags) = 0;
            virtual u32 getInstanceCount() { return 0; }

            //! Get the currently used ID for identification of changes.
            /** This shouldn't be used for anything outside the VideoDriver. */
            virtual u32 getChangedID() const { return mChangedID; }

            virtual void addSubBuffer() {}
            virtual void setActiveSubBuffer(irr::u16 idx) {}
        };


        // New Version of Shader Buffer Variables


        template<typename T>
        struct CShaderVariableObject : public IShaderDataBufferElement
        {
            void** m_memoryBlock = nullptr;
            u32 m_offset = 0;
            bool* m_changed = nullptr;

            CShaderVariableObject() : IShaderDataBufferElement()
            {
            }

            virtual ~CShaderVariableObject() {}

            void Initialize(bool* changedRef, ShaderVariableDescriptor const* description, void** memoryBlock, u32 offset)
            {
                m_changed = changedRef;

                IShaderDataBufferElement::Initialize(description);
                m_memoryBlock = memoryBlock;
                m_offset = offset;
                setShaderValues(T());
            }

            virtual bool isChanged() const { return *m_changed; }
            virtual void setDirty() { *m_changed = true; }
            virtual void setUpdated() { *m_changed = false; }

            void setShaderValues(T const& data)
            {
                if (GetValueRef() != data)
                {
                    GetValueRef() = data;
                    *m_changed = true;
                }
            }

            CShaderVariableObject<T>* operator=(T const& data)
            {
                if (m_description && m_memoryBlock)
                    setShaderValues(data);
                return this;
            }

            T& GetValueRef() { return *(T*)(((u8*)*m_memoryBlock) + m_offset); }
            virtual void* getShaderValues() const { return *(void**)(((u8*)*m_memoryBlock) + m_offset); }
            virtual u32 getShaderValueCount() const { return 1; }
            virtual size_t getValueSizeOf() const { return sizeof(T); }
        };

        template<typename T>
        struct CShaderVariableObjectArray : public IShaderDataBufferElement
        {
            core::array<T> m_values;
            bool* m_changed = nullptr;

            CShaderVariableObjectArray() : IShaderDataBufferElement()
            {
            }

            virtual ~CShaderVariableObjectArray() {}

            void Initialize(bool* changedRef, ShaderVariableDescriptor const* description, void** memoryBlock, size_t count)
            {
                m_changed = changedRef;

                IShaderDataBufferElement::Initialize(description);
                m_values.set_pointer((T*)*memoryBlock, count, false, false);
            }

            virtual void retarget(void* memoryBlock, size_t count) override
            {
                m_values.set_pointer((T*)memoryBlock, count, false, false);
            }

            // TODO: implement later or set manual
            virtual bool isChanged() const { return *m_changed; }
            virtual void setDirty() { *m_changed = true; }
            virtual void setUpdated() { *m_changed = false; }

            void ResetShaderValues()
            {
                *m_changed = true;
                m_values.set_used(0);
            }

            void PushShaderValues(T const& data)
            {
                *m_changed = true;
                m_values.push_back(data);
            }

            void MoveShaderValue(T const&& data)
            {
                *m_changed = true;
                m_values.push_back(data);
            }

            T& Emplace()
            {
                *m_changed = true;
                m_values.set_used(m_values.size() + 1);
                return m_values.getLast();
            }

            ShaderDataBufferElementObject<T>* operator+=(T const& data)
            {
                PushShaderValues(data);
                return (ShaderDataBufferElementObject<T>*)this;
            }

            virtual void* getShaderValues() const { return (void*)m_values.const_pointer(); }
            virtual u32 getShaderValueCount() const { return m_values.size(); }
            virtual size_t getValueSizeOf() const { return sizeof(T); }
        };

        struct  IRRLICHT_API  IShader
        {
            typedef core::array<ShaderVariableDescriptor> ShaderVariableDescArray;

            core::array<IShaderDataBuffer*> mBuffers[IShaderDataBuffer::EUT_MAX_VALUE];
            video::IVideoDriver* mContext;
            u8 mContextType;
            E_ShaderTypes mShaderType;
            bool mBinded : 1;

            ShaderVariableDescArray m_shaderVariableDescArray;
            //s16 m_shaderVariableIndexTable[EGVAT_MAX_VALUE];

            explicit IShader(video::IVideoDriver* context, E_ShaderTypes type);
            virtual ~IShader();

            static bool readFile(const char* pFileName, std::string& outFile);
            u8 getContextType() const;
            E_ShaderTypes getShaderType() const { return mShaderType; }

            video::IVideoDriver* getVideoDriver() { return mContext; }

            void addDataBuffer(IShaderDataBuffer* buffer, void* Descriptor)
            {
                buffer->InitializeFormShader(this, Descriptor);
                mBuffers[buffer->getUpdateType()].push_back(buffer);
            }

            //void LinkShaderVariable(const char* name, E_VERTEX_ELEMENT_SEMANTIC basicVariableLocation = EGVAT_NONE, u32* rid = nullptr);

            virtual void AddShaderVariable(ShaderVariableDescriptor* ui)
            {
                m_shaderVariableDescArray.push_back(*ui);
            }

            virtual E_ShaderVersion getShaderVersion() const = 0;
            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual void destroy() = 0;
            virtual void Init() = 0;

            virtual void CreateAttributeExtensions(video::IHardwareBuffer* hwBuff) {}
            void UpdateValues(IShaderDataBuffer::E_UPDATE_TYPE updateType, scene::IMeshBuffer* meshBuffer, scene::IMesh* mesh, scene::ISceneNode* node, u32 updateFlags)
            {
                if (mBuffers[updateType].empty())
                    return;

                for (u32 i = 0; i != mBuffers[updateType].size(); ++i)
                    mBuffers[updateType][i]->UpdateBuffer(this, meshBuffer, mesh, node, updateFlags);

                if (updateFlags & video::IShaderDataBuffer::EUF_COMMIT_VALUES)
                    CommitValues(updateType);
            }

            void CommitValues(IShaderDataBuffer::E_UPDATE_TYPE updateType)
            {
                if (mBuffers[updateType].empty())
                    return;

                for (u32 i = 0; i != mBuffers[updateType].size(); ++i)
                    mBuffers[updateType][i]->CommitBuffer(this);
            }

            void setDirty()
            {
                for (u32 ib = 0; ib != IShaderDataBuffer::EUT_MAX_VALUE; ++ib)
                {
                    if (mBuffers[ib].empty())
                        return;

                    for (u32 i = 0; i != mBuffers[ib].size(); ++i)
                        mBuffers[ib][i]->setDirty();
                }
            }

            virtual short GetUniformLocationByCacheIdx(short id) { return -1; }
            //virtual ShaderVariableDescriptor const* GetGPUVariableDesc(E_VERTEX_ELEMENT_SEMANTIC type) const;
            virtual s32 getGPUProgramAttribLocation(E_VERTEX_ELEMENT_SEMANTIC type) const;
            virtual ShaderVariableDescriptor const* GetGPUVariableDesc(const char* name, u32* rid = nullptr) const;
            virtual int getVariableLocation(const char* name);
            virtual int getProgramParam(int param) { return -1; }


            virtual u32 getBufferBinding(ShaderVariableDescriptor const*) const { return 0; }
            virtual size_t getBufferSize(ShaderVariableDescriptor const*) const { return 0; }
            virtual BufferVariableMemoryInfo getBufferVariableMemoryInfo(ShaderVariableDescriptor const*) const { return BufferVariableMemoryInfo(); }
        };

        struct IRRLICHT_API CShaderBuffer : irr::video::IShaderDataBuffer
        {
            irr::video::IVideoDriver* mDriver;
            std::vector<irr::video::IHardwareBuffer*> ConstantBuffers;
            std::vector<char*> DataBuffers;
            //std::vector<IShaderDataBufferElement*> ArrayObjects;
            
            char* DataBuffer;
            irr::u32 mShaderId;
            irr::u32 mBinding;
            irr::u32 mBufferSize;
            irr::u8 mBufferCount;
            irr::u16 mActiveSubBuffer;
            scene::E_HARDWARE_MAPPING mConstantMapping : 8;
            bool mChanged;
            bool mUnionBuffer;

            CShaderBuffer(irr::video::IVideoDriver* driver, irr::u32 shaderId)
                : IShaderDataBuffer(IShaderDataBuffer::E_UPDATE_TYPE::EUT_PER_FRAME)
                , mDriver(driver)
                , DataBuffer(nullptr)
                , mShaderId(shaderId)
                , mBufferSize(0)
                , mBufferCount(0)
                , mActiveSubBuffer(0)
                , mConstantMapping(scene::E_HARDWARE_MAPPING::EHM_STATIC)
                , mChanged(false)
                , mUnionBuffer(false)
            {

            }

            virtual ~CShaderBuffer();

            scene::E_HARDWARE_MAPPING getHardwareMappingHint() const { return mConstantMapping; }
            void setHardwareMappingHint(scene::E_HARDWARE_MAPPING value) { mConstantMapping = value; }

            irr::video::IHardwareBuffer* getHardwareBuffer()
            {
                return ConstantBuffers[mActiveSubBuffer];
            }

            irr::video::IHardwareBuffer* getHardwareBuffer(irr::u8 idx)
            {
                if (mBufferCount < idx)
                    return ConstantBuffers[idx];
                return nullptr;
            }

            void addSubBuffer() override
            {
                if (mUnionBuffer)
                    return;

                ConstantBuffers.emplace_back();
                DataBuffers.emplace_back();
                DataBuffers.back() = new char[mBufferSize];
                DataBuffer = DataBuffers[mActiveSubBuffer];
                ++mBufferCount;
            }

            void setActiveSubBuffer(irr::u16 idx) override
            {
                if (mUnionBuffer || mBufferCount <= idx || mActiveSubBuffer == idx)
                    return;

                mActiveSubBuffer = idx;
                DataBuffer = DataBuffers[mActiveSubBuffer];

                //ArrayObjects
            }

            void Initialize(ShaderVariableDescriptor const* description)
            {
                if (!description || mBufferSize)
                    return;

                irr::video::IShader* shader = mDriver->GetShaderModul(mShaderId);
                if (!shader)
                    return;

                ConstantBuffers.resize(1);
                DataBuffers.resize(1);
                mBufferCount = 1;

                mBufferSize = shader->getBufferSize(description);
                mBinding = shader->getBufferBinding(description);

                DataBuffers[0] = new char[mBufferSize];
                DataBuffer = DataBuffers[0];
            }

            template<typename T>
            void CreateVariableInterface(CShaderVariableObject<T>& variable, const char* VariableName)
            {
                irr::video::IShader* shader = mDriver->GetShaderModul(mShaderId);
                if (!shader)
                    return;

                ShaderVariableDescriptor const* description = shader->GetGPUVariableDesc(VariableName);
                if (!description || (mBufferSize && shader->getBufferBinding(description) != mBinding))
                    return;

                irr::video::BufferVariableMemoryInfo memoryInfo = shader->getBufferVariableMemoryInfo(description);
                if (!memoryInfo.MemorySize)
                    return;

                Initialize(description);

                assert(sizeof(T) <= memoryInfo.ElementSize);
                assert(mBufferSize >= (memoryInfo.Offset + memoryInfo.ElementSize));

                variable.Initialize(&mChanged, description, (void**)&DataBuffer, memoryInfo.Offset);
            }

            template<typename T>
            void CreateVariableArrayInterface(CShaderVariableObjectArray<T>& variable, const char* VariableName)
            {
                irr::video::IShader* shader = mDriver->GetShaderModul(mShaderId);
                if (!shader)
                    return;

                ShaderVariableDescriptor const* description = shader->GetGPUVariableDesc(VariableName);
                if (!description || (mBufferSize && shader->getBufferBinding(description) != mBinding))
                    return;

                irr::video::BufferVariableMemoryInfo memoryInfo = shader->getBufferVariableMemoryInfo(description);
                if (!memoryInfo.MemorySize)
                    return;

                Initialize(description);

                assert(sizeof(T) == memoryInfo.ElementSize);
                assert((memoryInfo.MemorySize % memoryInfo.ElementSize) == 0);
                assert(mBufferSize >= (memoryInfo.Offset + memoryInfo.ElementSize));

                //ArrayObjects.push_back(&variable);
                mUnionBuffer = true; // ToDo: not implement yet
                variable.Initialize(&mChanged, description, (void**)&DataBuffer, memoryInfo.MemorySize / memoryInfo.ElementSize);
            }
        };
    }
}

#endif // !__C_VIDEO_NULL_SHADER_H_INCLUDED__