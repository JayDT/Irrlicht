// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_NULL_SHADER_H_INCLUDED__
#define __C_VIDEO_NULL_SHADER_H_INCLUDED__

#include "IVideoDriver.h"
#include "IShader.h"
#include "IShaderConstantSetCallBack.h"
#include "IGPUProgrammingServices.h"
#include "standard/client/IDataSourceClient.h"

namespace irr
{
    namespace video
    {
        struct SConstantBuffer;

        struct ShaderInitializerEntry
        {
            struct ShaderMacro
            {
                core::stringc Name;
                core::stringc Value;
            };

            struct StageDesc
            {
                System::IO::IFileReader* DataStream = nullptr;
                core::stringc EntryPoint;
                core::stringc ShaderModel;
                E_GPU_SHADING_LANGUAGE ShadingLang = E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT;
                E_SHADER_TYPES ShaderStageType = E_SHADER_TYPES::EST_HIGH_LEVEL_SHADER;
                size_t DataStreamOffset = 0;
                bool IsByteCode = false;
                std::vector<ShaderMacro> Macros;
            };

            std::map<std::string/*buffer name*/, IShaderConstantSetCallBack*> Callback;
            std::vector<StageDesc*> mStages;
            s32 mShaderId = -1;

            ~ShaderInitializerEntry()
            {
                for (auto stage : mStages)
                    delete stage;
            }

            StageDesc* AddShaderStage(
                System::IO::IFileReader* DataStream,
                E_SHADER_TYPES ShaderStageType,
                const c8* EntryPoint = nullptr,
                const c8* ShaderModel = nullptr,
                E_GPU_SHADING_LANGUAGE ShadingLang = E_GPU_SHADING_LANGUAGE::EGSL_DEFAULT,
                bool IsByteCode = false,
                std::vector<ShaderMacro> Macros = {})
            {
                StageDesc* stage = new StageDesc;
                stage->DataStream = DataStream;
                stage->ShaderStageType = ShaderStageType;
                stage->EntryPoint = EntryPoint;
                stage->ShaderModel = ShaderModel;
                stage->ShadingLang = ShadingLang;
                stage->IsByteCode = IsByteCode;
                stage->Macros = Macros;
                mStages.push_back(stage);
                return stage;
            }
        };

        // New Version of Shader Buffer Variables

        struct SShaderVariableScalar : IShaderScalarVariable
        {
            SConstantBuffer * mBuffer;
            IShaderVariable * mParent;
            SShaderType*      mType;
            core::stringc     mName;
            core::stringc     mSemantic;
            u32               mSemanticIndex = 0;

            u32 mOffset;
            u32 mLayoutIndex;
            E_SHADER_TYPES mShaderStage;  // Use only uniforms

            bool mIsDirty;
            bool mIsValid;

            // Inherited via IShaderScalarVariable
            virtual bool isValid() override;
            virtual bool isDirty() override;
            virtual IConstantBuffer * getRootBuffer() override;
            virtual IShaderVariable * getParentVariable() override;
            virtual IShaderScalarVariable * AsScalar() override;
            virtual IShaderMatrixVariable * AsMatrix() override;
            virtual SShaderVariableStruct * _asStruct() override { return nullptr; }
            virtual void setRawValue(const u8 * values, u32 offset, u32 count) override;
            virtual u8 * getRawValue(u32 offset, u32 count) override;
            virtual void setFloat(const float value) override;
            virtual float getFloat() override;
            virtual void setFloatArray(const float * values, u32 offset, u32 count) override;
            virtual float * getFloatArray(u32 offset, u32 count) override;
            virtual void setInt(const s32 value) override;
            virtual s32 getInt() override;
            virtual void setIntArray(const s32 * values, u32 offset, u32 count) override;
            virtual s32 * getIntArray(u32 offset, u32 count) override;
            virtual void setBool(const bool value) override;
            virtual bool getBool() override;
            virtual void setBoolArray(const bool * values, u32 offset, u32 count) override;
            virtual bool * getBoolArray(u32 offset, u32 count) override;

            virtual const core::stringc& GetName() override { return mName; }
            virtual u32 _getOffset() override { return mOffset; }
            virtual SShaderType* getType() override { return mType; }
            virtual u32 _getIndex() override { return mLayoutIndex; }
            virtual const core::stringc& GetSemantic() override { return mSemantic; }

            virtual IShaderVariable * getVariableByIndex(u32 index) { return nullptr; }
            virtual IShaderVariable * getVariableByName(const char * name) { return nullptr; }
            virtual IShaderVariable * getVariableBySemantic(const char * semantic) { return nullptr; }
        };

        struct SShaderVariableMatrix : IShaderMatrixVariable
        {
            SConstantBuffer * mBuffer;
            IShaderVariable * mParent;
            SShaderType*      mType;
            core::stringc     mName;
            core::stringc     mSemantic;

            u32 mOffset;
            u32 mLayoutIndex;

            bool mIsRowMajor;
            bool mIsDirty;
            bool mIsValid;

            // Inherited via IShaderMatrixVariable
            virtual bool isValid() override;
            virtual bool isDirty() override;
            virtual IConstantBuffer * getRootBuffer() override;
            virtual IShaderVariable * getParentVariable() override;
            virtual IShaderScalarVariable * AsScalar() override;
            virtual IShaderMatrixVariable * AsMatrix() override;
            virtual SShaderVariableStruct * _asStruct() override { return nullptr; }
            virtual void setRawValue(const u8 * values, u32 offset, u32 count) override;
            virtual u8 * getRawValue(u32 offset, u32 count) override;
            virtual bool isRowMajor() override { return mIsRowMajor; }
            virtual void setMatrix(const core::matrix4 & value) override;
            virtual core::matrix4 & getMatrix() override;
            virtual void setMatrixArray(const core::matrix4 * values, u32 offset, u32 count) override;
            virtual core::matrix4 * getMatrixArray(u32 offset, u32 count) override;

            virtual const core::stringc& GetName() override { return mName; }
            virtual u32 _getOffset() override { return mOffset; }
            virtual SShaderType* getType() override { return mType; }
            virtual u32 _getIndex() override { return mLayoutIndex; }
            virtual const core::stringc& GetSemantic() override { return mSemantic; }

            virtual IShaderVariable * getVariableByIndex(u32 index) { return nullptr; }
            virtual IShaderVariable * getVariableByName(const char * name) { return nullptr; }
            virtual IShaderVariable * getVariableBySemantic(const char * semantic) { return nullptr; }
        };

        struct SShaderVariableStruct : IShaderVariable
        {
            SConstantBuffer * mBuffer;
            IShaderVariable * mParent;
            SShaderType*      mType;
            core::stringc     mName;
            core::stringc     mSemantic;
            void*             mLoaderData;

            u32 mOffset;
            u32 mLayoutIndex;

            bool mIsDirty;
            bool mIsValid;

            std::vector<IShaderVariable*> mVariables;

            // Inherited via IShaderVariable
            virtual bool isValid() override;
            virtual bool isDirty() override;
            virtual IConstantBuffer * getRootBuffer() override;
            virtual IShaderVariable * getParentVariable() override;
            virtual IShaderScalarVariable * AsScalar() override;
            virtual IShaderMatrixVariable * AsMatrix() override;
            virtual SShaderVariableStruct * _asStruct() override { return this; }
            virtual void setRawValue(const u8 * values, u32 offset, u32 count) override;
            virtual u8 * getRawValue(u32 offset, u32 count) override;

            virtual const core::stringc& GetName() override { return mName; }
            virtual u32 _getOffset() override { return mOffset; }
            virtual SShaderType* getType() override { return mType; }
            virtual u32 _getIndex() override { return mLayoutIndex; }
            virtual const core::stringc& GetSemantic() override { return mSemantic; }

            virtual IShaderVariable * getVariableByIndex(u32 index);
            virtual IShaderVariable * getVariableByName(const char * name);
            virtual IShaderVariable * getVariableBySemantic(const char * semantic);
        };

        struct SShaderSampler : IShaderVariable
        {
            SConstantBuffer* mBuffer;
            IShaderVariable* mParent;
            SShaderType*     mType;
            core::stringc    mName;
            static core::stringc mSemantic;

            E_SHADER_TYPES                  mShaderStage;       // Sets which shader stage to use

            u32 mBindPoint;             // Used when a CB has been explicitly(or generated) bound.
            u32 mLayoutIndex;

            bool mIsValid;

            // Inherited via IShaderVariable
            virtual bool isValid() override;
            virtual bool isDirty() override;
            virtual IConstantBuffer* getRootBuffer() override;
            virtual IShaderVariable* getParentVariable() override;
            virtual IShaderScalarVariable* AsScalar() override;
            virtual IShaderMatrixVariable* AsMatrix() override;
            virtual SShaderVariableStruct* _asStruct() override { return nullptr; }
            virtual void setRawValue(const u8* values, u32 offset, u32 count) override {}
            virtual u8* getRawValue(u32 offset, u32 count) override { return nullptr; }

            virtual const core::stringc& GetName() override { return mName; }
            virtual u32 _getOffset() override { return 0; }
            virtual SShaderType* getType() override { return mType; }
            virtual u32 _getIndex() override { return mBindPoint; }
            virtual const core::stringc& GetSemantic() override { return mSemantic; }

            virtual IShaderVariable* getVariableByIndex(u32 index) { return nullptr; }
            virtual IShaderVariable* getVariableByName(const char* name) { return nullptr; }
            virtual IShaderVariable* getVariableBySemantic(const char* semantic) { return nullptr; }
        };

        struct SConstantBuffer : IConstantBuffer
        {
            IShader*                        mShader;
            irr::video::IHardwareBuffer*    mHwObject;
            SShaderType*                    mType;
            irr::Ptr<IShaderConstantSetCallBack> mCallBack;
            uint8_t*                        mBackStore;
            uint32_t                        mSize;              // in bytes

            core::stringc                   mName;

            std::vector<u8>                 mHostMemory;
            std::vector<IShaderVariable*>   mVariables;

            E_SHADER_BUFFER_TYPE            mBufferType;
            E_SHADER_TYPES                  mShaderStage;       // Sets which shader stage to use
            s32                             mOffset;
            s32                             mBindPoint;         // Used when a CB has been explicitly(or generated) bound.
            u32                             mChangedID;         // Set when any member is updated; cleared on CB apply 
            scene::E_HARDWARE_MAPPING       mMapping : 8;

            bool                            mRowMajor;
            bool                            mLayout16ByteAlign;


            explicit SConstantBuffer(IShader*, E_SHADER_TYPES stage);
            virtual ~SConstantBuffer();

            void Invalidate();
            void AddVariable(IShaderVariable* variable);

            virtual void Clear() {}
            virtual bool AddDataFromSceneNode(scene::ISceneNode*, irr::scene::IMesh*) { return true; }

            bool isChanged() const;
            virtual void setDirty() { ++mChangedID; }

            //! Get the currently used ID for identification of changes.
            /** This shouldn't be used for anything outside the VideoDriver. */
            virtual u32 getChangedID() const { return mChangedID; }

            // Inherited via IConstantBuffer
            virtual void clear() override;
            virtual void setShaderDataCallback(const IShaderConstantSetCallBack *) override;
            virtual IShaderVariable * getVariableByIndex(u32 index) override;
            virtual IShaderVariable * getVariableByName(const char * name) override;
            virtual IShaderVariable * getVariableBySemantic(const char * semantic) override;
            virtual bool setHardwareMappingHint(scene::E_HARDWARE_MAPPING constantMapping) override;
            virtual scene::E_HARDWARE_MAPPING getHardwareMappingHint() override;
            virtual u32 getBindingIndex() const override;

            virtual bool isValid() override;
            virtual bool isDirty() override;

            virtual const core::stringc& GetName() override;
            virtual SShaderType* getType() override;

            virtual IConstantBuffer* getRootBuffer() override;
            virtual IShaderVariable* getParentVariable() override;

            virtual IShaderScalarVariable* AsScalar() override;
            virtual IShaderMatrixVariable* AsMatrix() override;
            virtual SShaderVariableStruct * _asStruct() override { return nullptr; }

            virtual void setRawValue(const u8* values, u32 offset, u32 count) override;
            virtual u8* getRawValue(u32 offset, u32 count) override;

            void write4bAlign(const u8* values, u32 offset /*In Byte*/, u32 count /*In Byte*/);
            void write8bAlign(const u8* values, u32 offset /*In Byte*/, u32 count /*In Byte*/);

            // Internal usage
            virtual u32 _getOffset() override;
            virtual u32 _getIndex() override { return 0; }
            virtual const core::stringc& GetSemantic() override { throw std::exception(); }

            virtual IShader* getShader() { return mShader; }
        };

        struct CNullShader : public IShader
        {
            typedef core::array<irr::Ptr<SConstantBuffer>> ConstantBufferArray;
            typedef core::array<irr::Ptr<IShaderScalarVariable>> VariableArray;
            typedef core::array<irr::Ptr<SShaderSampler>> SamplerArray;

            ConstantBufferArray mBuffers;
            SamplerArray mTextures;
            VariableArray mVertexInput;

            video::IVideoDriver* mContext;
            E_SHADER_LANG mShaderLang;
            bool mBinded : 1;

            explicit CNullShader(video::IVideoDriver* context, E_SHADER_LANG type);
            virtual ~CNullShader();

            E_SHADER_LANG getShaderType() const { return mShaderLang; }
            video::IVideoDriver* getVideoDriver() { return mContext; }

            virtual void Init() {}

            // Add byte array buffer
            virtual IConstantBuffer* AddUnknownBuffer(E_SHADER_TYPES shaderType, u32 size) = 0;
            void AddConstantBuffer(SConstantBuffer* buffer);
            void AddShaderResource(IShaderVariable* var);

            virtual IConstantBuffer* getConstantBufferByIndex(u32 index) override;
            virtual IConstantBuffer* getConstantBufferByName(const char* name) override;

            virtual IShaderVariable * getVariableByIndex(u32 index);
            virtual IShaderVariable * getVariableByName(const char * name);
            virtual IShaderVariable * getVariableBySemantic(const char * semantic);

            static SShaderType * GetShaderType(E_SHADER_VARIABLE_TYPE Type, u32 Elements, u32 Members, u32 Rows, u32 Columns, u32 PackedSize, u32 UnpackedSize, u32 ArrayStride, u32 Stride, bool UseCache = false);
        };

        class IrrDefaultShaderVertexCallBack : public video::IShaderConstantSetCallBack
        {
        public:

            virtual ~IrrDefaultShaderVertexCallBack() {}

            void OnPrepare(irr::video::IConstantBuffer* shader) override;

            void OnSetMaterial(video::IConstantBuffer* buffer, const SMaterial& material)
            {
                mMaterial = &material;
            }

            void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr);

        private:

            const SMaterial* mMaterial = nullptr;
            IShaderMatrixVariable* mWorldMatrix;
            IShaderMatrixVariable* mViewMatrix;
            IShaderMatrixVariable* mProjectionMatrix;
            IShaderMatrixVariable* mTextureMatrix1;
            IShaderMatrixVariable* mTextureMatrix2;
        };

        class IrrDefaultShaderFragmentCallBack : public video::IShaderConstantSetCallBack
        {
        public:

            virtual ~IrrDefaultShaderFragmentCallBack() {}

            void OnPrepare(irr::video::IConstantBuffer* shader) override;

            void OnSetMaterial(video::IConstantBuffer* buffer, const SMaterial& material)
            {
                mMaterial = &material;
            }

            void OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, scene::IMesh* mesh = nullptr, scene::ISceneNode* node = nullptr) override;

        private:

            struct ATTR_ALIGNED(16) Light
            {
                float Position[4];
                float Atten[4];
                irr::video::SColorf Diffuse;
                irr::video::SColorf Specular;
                irr::video::SColorf Ambient;
                float Range;
                float Falloff;
                float Theta;
                float Phi;
                int   Type;
            
                bool operator==(const Light& other) const
                {
                    return memcmp(this, &other, sizeof(Light)) == 0;
                }
            
                bool operator!=(const Light& other) const
                {
                    return memcmp(this, &other, sizeof(Light)) != 0;
                }
            };
            
            struct ATTR_ALIGNED(16) Material
            {
                irr::video::SColorf    Ambient;
                irr::video::SColorf    Diffuse;
                irr::video::SColorf    Specular;
                irr::video::SColorf    Emissive;
                float     Shininess;
                int       Type;
                int       Lighted;
                int       Fogged;
            
                bool operator==(const Material& other) const
                {
                    return memcmp(this, &other, sizeof(Material)) == 0;
                }
            
                bool operator!=(const Material& other) const
                {
                    return memcmp(this, &other, sizeof(Material)) != 0;
                }
            };

            core::vector3df viewMatrixTranslationCache;

            const SMaterial* mMaterial = nullptr;
            IShaderScalarVariable* mTexture;
            IShaderScalarVariable* mAlphaTest;
            IShaderScalarVariable* mLightCount;
            IShaderScalarVariable* mFAlphaRef;
            IShaderScalarVariable* mFogColor;
            IShaderScalarVariable* mFogParams;
            IShaderScalarVariable* mEyePositionVert;

            IShaderVariable* mMaterialVar;
            IShaderVariable* mLightVar;
        };

    }
}

#endif // !__C_VIDEO_NULL_SHADER_H_INCLUDED__