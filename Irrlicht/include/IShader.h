#ifndef __I_SHADER_H_INCLUDED__
#define __I_SHADER_H_INCLUDED__

#include "irrArray.h"
#include "fast_atof.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "coreutil.h"

namespace irr
{
    namespace video
    {
        class IHardwareBuffer;
        class IShaderConstantSetCallBack;
        struct VertexDeclaration;
        struct SShaderType;
        struct IShaderScalarVariable;
        struct IShaderMatrixVariable;
        struct IConstantBuffer;
        struct SShaderVariableStruct;
        struct SShaderSampler;

        enum E_SHADER_TYPES : u8
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

        enum E_SHADER_LANG : u8
        {
            ESV_GLSL_ASM,
            ESV_GLSL_HIGH_LEVEL,
            ESV_HLSL_HIGH_LEVEL
        };

        enum E_SHADER_BUFFER_TYPE : u8
        {
            ESVT_UNKNOWN,
            ESVT_ATTRIBUTE,
            ESVT_UNIFORM,
            ESVT_CONSTANT,
            ESVT_INPUT_STREAM
        };

        enum E_SHADER_VARIABLE_TYPE : u8
        {
            ESVT_VOID = 0,
            ESVT_BOOL,
            ESVT_INT,
            ESVT_UINT,
            ESVT_INT8,
            ESVT_UINT8,
            ESVT_INT16,
            ESVT_UINT16,
            ESVT_INT64,
            ESVT_UINT64,
            ESVT_FLOAT,
            ESVT_FLOAT16,
            ESVT_DOUBLE,
            ESVT_TEXTURE,
            ESVT_SAMPLER,
            ESVT_BUFFER,
            ESVT_STRUCT,
            ESVT_MAX
        };

        struct IShaderVariable : irr::IReferenceCounted
        {
            virtual bool isValid() = 0;
            virtual bool isDirty() = 0;

            virtual const core::stringc& GetName() = 0;
            virtual const core::stringc& GetSemantic() = 0;
            virtual SShaderType* getType() = 0;

            virtual IConstantBuffer* getRootBuffer() = 0;
            virtual IShaderVariable* getParentVariable() = 0;

            // ToDo: Vector if need
            virtual IShaderScalarVariable* AsScalar() = 0;
            virtual IShaderMatrixVariable* AsMatrix() = 0;

            virtual void setRawValue(const u8* values, u32 offset, u32 count) = 0;
            virtual u8* getRawValue(u32 offset, u32 count) = 0;

            virtual IShaderVariable* getVariableByIndex(u32 index) = 0;
            virtual IShaderVariable* getVariableByName(const char* name) = 0;
            virtual IShaderVariable* getVariableBySemantic(const char* semantic) = 0;

            // Internal usage
            virtual u32 _getOffset() = 0;
            virtual u32 _getIndex() = 0;
            virtual SShaderVariableStruct* _asStruct() = 0;
        };

        struct IConstantBuffer : IShaderVariable
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

            virtual ~IConstantBuffer() { }

            virtual void clear() = 0;
            virtual bool isChanged() const = 0;

            // Callback functions when
            virtual void setShaderDataCallback(const IShaderConstantSetCallBack*) = 0;

            virtual void setDirty() = 0;

            //! Get the currently used ID for identification of changes.
            /** This shouldn't be used for anything outside the VideoDriver. */
            virtual u32 getChangedID() const = 0;

            virtual IShaderVariable* getVariableByIndex(u32 index) = 0;
            virtual IShaderVariable* getVariableByName(const char* name) = 0;
            virtual IShaderVariable* getVariableBySemantic(const char* semantic) = 0;

            virtual bool setHardwareMappingHint(scene::E_HARDWARE_MAPPING constantMapping) = 0;
            virtual scene::E_HARDWARE_MAPPING getHardwareMappingHint() = 0;
            virtual u32 getBindingIndex() const = 0;

            virtual IShader* getShader() = 0;
        };

        struct IShader : virtual irr::IReferenceCounted
        {
            virtual u32 getId() const = 0;
            virtual E_SHADER_LANG getShaderType() const = 0;
            virtual video::IVideoDriver* getVideoDriver() = 0;

            virtual IConstantBuffer* getConstantBufferByIndex(u32 index) = 0;
            virtual IConstantBuffer* getConstantBufferByName(const char* name) = 0;

            virtual IShaderVariable* getVariableByIndex(u32 index) = 0;
            virtual IShaderVariable* getVariableByName(const char* name) = 0;
            virtual IShaderVariable* getVariableBySemantic(const char* semantic) = 0;
        };

        // Shader reflection variable

        struct IShaderScalarVariable : IShaderVariable
        {
            virtual void setFloat(const float value) = 0;
            virtual float getFloat() = 0;

            virtual void setFloatArray(const float* values, u32 offset, u32 count) = 0;
            virtual float* getFloatArray(u32 offset, u32 count) = 0;

            virtual void setInt(const s32 value) = 0;
            virtual s32 getInt() = 0;

            virtual void setIntArray(const s32* values, u32 offset, u32 count) = 0;
            virtual s32* getIntArray(u32 offset, u32 count) = 0;

            virtual void setBool(const bool value) = 0;
            virtual bool getBool() = 0;

            virtual void setBoolArray(const bool* values, u32 offset, u32 count) = 0;
            virtual bool* getBoolArray(u32 offset, u32 count) = 0;
        };

        struct IShaderMatrixVariable : IShaderVariable
        {
            virtual bool isRowMajor() = 0;

            virtual void setMatrix(const core::matrix4& value) = 0;
            virtual core::matrix4& getMatrix() = 0;

            virtual void setMatrixArray(const core::matrix4* values, u32 offset, u32 count) = 0;
            virtual core::matrix4* getMatrixArray(u32 offset, u32 count) = 0;
        };

        struct SShaderType
        {
            const char* TypeName;

            E_SHADER_VARIABLE_TYPE  Type;

            uint32_t    Elements;           // Number of elements in this type
                                            // (0 if not an array) 
            uint32_t    Members;            // Number of members
                                            // (0 if not a structure)
            uint32_t    Rows;               // Number of rows in this type
                                            // (0 if not a numeric primitive, for matrices)
            uint32_t    Columns;            // Number of columns in this type
                                            // (0 if not a numeric primitive, for vectors & matrices)

            uint32_t    PackedSize;         // Number of bytes required to represent
                                            // this data type, when tightly packed
            uint32_t    UnpackedSize;       // Number of bytes occupied by this data
                                            // type, when laid out in a constant buffer
            uint32_t    ArrayStride;        // Number of bytes to seek between array elements,
                                            // when laid out in a constant buffer
            uint32_t    Stride;             // Number of bytes to seek between elements,
                                            // when laid out in a constant buffer
        };
    }
}

#endif //! __I_SHADER_H_INCLUDED__