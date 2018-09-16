#include "COpenGLDriver.h"
#include "COpenGLShader.h"
#include "os.h"
#include "COpenGLSupport.h"
#include "COpenGLExtensionHandler.h"
#include "COpenGLMaterialRenderer.h"
#include "IShaderConstantSetCallBack.h"

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
#include "MacOSX/CIrrDeviceMacOSX.h"
#endif

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
#include <SDL/SDL.h>
#endif

#include <algorithm>

// Workaround on <X11/X.h> defined Success
#undef Success

using namespace irr;
using namespace irr::video;

static int mBindedProgram = 0;

core::matrix4 OGL_UnitMatrix;
core::matrix4 OGL_SphereMapMatrix;

video::E_SHADER_VARIABLE_TYPE getShaderVariableTypeId(u32 glslangType)
{
    switch (glslangType)
    {
        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_BOOL;
        case GL_DOUBLE:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_DOUBLE;
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT4x2:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_FLOAT;
        //case GL_FLOAT16_NV:
        //    return video::E_SHADER_VARIABLE_TYPE::ESVT_FLOAT16;
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_INT;
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_UINT;
        //case GL_INT16_NV:
        //    return video::E_SHADER_VARIABLE_TYPE::ESVT_INT16;
        case GL_INT64_ARB:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_INT64;
        case GL_UNSIGNED_INT64_ARB:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_UINT64;
        //case GL_INT8_NV:
        //    return video::E_SHADER_VARIABLE_TYPE::ESVT_INT8;
        case GL_SAMPLER:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_SAMPLER;
        //case GL_STRUCT:
        //    return video::E_SHADER_VARIABLE_TYPE::ESVT_STRUCT;
        case GL_BUFFER:
            return video::E_SHADER_VARIABLE_TYPE::ESVT_BUFFER;
    }
    return video::E_SHADER_VARIABLE_TYPE::ESVT_MAX;
}

u32 getShaderVariableVectorSize(u32 glslangType)
{
    switch (glslangType)
    {
        case GL_BOOL:
        case GL_FLOAT:
        case GL_DOUBLE:
        case GL_INT:
        case GL_UNSIGNED_INT:
            return 1;
        case GL_BOOL_VEC2:
        case GL_FLOAT_VEC2:
        case GL_INT_VEC2:
        case GL_UNSIGNED_INT_VEC2:
            return 2;
        case GL_BOOL_VEC3:
        case GL_FLOAT_VEC3:
        case GL_INT_VEC3:
        case GL_UNSIGNED_INT_VEC3:
            return 3;
        case GL_BOOL_VEC4:
        case GL_FLOAT_VEC4:
        case GL_INT_VEC4:
        case GL_UNSIGNED_INT_VEC4:
            return 4;
        case GL_FLOAT_MAT2:
            return 4;
        case GL_FLOAT_MAT3:
            return 9;
        case GL_FLOAT_MAT4:
            return 16;
        case GL_FLOAT_MAT2x3:
            return 6;
        case GL_FLOAT_MAT3x2:
            return 6;
        case GL_FLOAT_MAT3x4:
            return 12;
        case GL_FLOAT_MAT4x3:
            return 12;
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT4x2:
        case GL_INT64_ARB:
        case GL_UNSIGNED_INT64_ARB:
            return 8;
        case GL_SAMPLER:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_BUFFER:
            return 1;
    }
    return 1;
}

GLArbShader::GLArbShader(video::IVideoDriver* context, u32 _programId)
    : GLShader(context, E_SHADER_LANG::ESV_GLSL_ASM, _programId)
{
}

GLArbShader::~GLArbShader()
{
    if (programId)
    {
        video::COpenGLDriver* driverOGL = getDriverOGL();
        if (driverOGL->GetActiveShader() == this)
            driverOGL->SetShader(nullptr);
        driverOGL->extGlDeleteProgram(programId);
    }
}

void GLArbShader::addShaderFile(u32 _glShaderType, const char *pFileName)
{
    //std::string source;
    //if (!readFile(pFileName, source))
    //    throw std::runtime_error("shader not found");
    //
    //video::COpenGLDriver* driverOGL = getDriverOGL();
    //
    //driverOGL->extGlGenPrograms(1, &programId);
    //driverOGL->extGlBindProgram(glShaderType, programId);
    //driverOGL->extGlProgramString(glShaderType, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(source.c_str()), source.c_str());
    //
    //auto error = glGetError();
    //if (error != GL_NO_ERROR)
    //{
    //    int errpos;
    //    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
    //    printf("Error loading shader: %s\nError position: %d\n", glGetString(GL_PROGRAM_ERROR_STRING_ARB), errpos);
    //    throw std::runtime_error("shader not found");
    //}
}

COpenGLDriver * irr::video::GLShader::getDriverOGL()
{
    return static_cast<COpenGLDriver*>(getVideoDriver());
}

GLSLGpuShader::GLSLGpuShader(video::IVideoDriver* context, u32 _programId) 
    : GLShader(context, E_SHADER_LANG::ESV_GLSL_HIGH_LEVEL, _programId)
{
}

GLSLGpuShader::~GLSLGpuShader()
{
    if (programId)
    {
        video::COpenGLDriver* driverOGL = getDriverOGL();
        if (driverOGL->GetActiveShader() == this)
            driverOGL->SetShader(nullptr);
        driverOGL->deleteShader(this);
    }
}

void GLSLGpuShader::addShaderFile(GLSLGpuShader* gpuProgram, u32 shaderType, System::IO::IFileReader *file, std::list<u32>& ShaderObjList)
{
    std::string source;
    source.resize(file->Size());
    file->Read((byte*)source.data(), file->Size());

    GLuint ShaderObj = getDriverOGL()->extGlCreateShader(shaderType);

    if (ShaderObj == 0)
    {
        printf("Error creating shader type %d\n", shaderType);
        throw std::runtime_error("shader not found");
    }

    ShaderObjList.push_back(ShaderObj);

    const GLchar* p[1];
    p[0] = source.c_str();
    GLint Lengths[1] = { (GLint)source.size() };

    getDriverOGL()->extGlShaderSource(ShaderObj, 1, p, Lengths);
    getDriverOGL()->extGlCompileShader(ShaderObj);

    GLint success;
    getDriverOGL()->extGlGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        GLchar InfoLog[2048];
        getDriverOGL()->extGlGetShaderInfoLog(ShaderObj, 2048, NULL, InfoLog);
        printf("Error loading shader: %s\nError position: %s\n", InfoLog, file->FileName.c_str());
        throw std::runtime_error("shader not found");
    }

    getDriverOGL()->extGlAttachShader(gpuProgram->getProgramId(), ShaderObj);
    static_cast<COpenGLDriver*>(getVideoDriver())->testGLError();
}

void GLSLGpuShader::compile(GLSLGpuShader* gpuProgram, std::list<u32>& ShaderObjList)
{
    GLint Success = 0;
    GLchar ErrorLog[5000] = { 0 };

    getDriverOGL()->extGlLinkProgram(gpuProgram->getProgramId());

    getDriverOGL()->extGlGetProgramiv(gpuProgram->getProgramId(), GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        GLint maxLength = 0;
        getDriverOGL()->extGlGetShaderiv(gpuProgram->getProgramId(), GL_INFO_LOG_LENGTH, &maxLength);

        getDriverOGL()->extGlGetProgramInfoLog(gpuProgram->getProgramId(), sizeof(ErrorLog), &maxLength, ErrorLog);
        printf("Error linking shader program: '%s'\n", ErrorLog);
        throw std::runtime_error("shader not found");
    }

    getDriverOGL()->extGLValidateProgram(gpuProgram->getProgramId());
    getDriverOGL()->extGlGetProgramiv(gpuProgram->getProgramId(), GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        getDriverOGL()->extGlGetProgramInfoLog(gpuProgram->getProgramId(), sizeof(ErrorLog), NULL, ErrorLog);
        printf("Error linking shader program: '%s'\n", ErrorLog);
        throw std::runtime_error("shader not found");
    }

    // Delete the intermediate shader objects that have been added to the program
    for (auto it : ShaderObjList)
        getDriverOGL()->extGlDeleteShader(it);
    
    ShaderObjList.clear();

    _IRR_DEBUG_BREAK_IF(glGetError() != GL_NO_ERROR);
}

void GLSLReflectUniform(GLSLGpuShader* shader)
{
    GLint num = 0;
    shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_UNIFORMS, &num);
    if (num == 0)
        return;

    GLint maxlen = 0;
    shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxlen);
    if (maxlen == 0)
    {
        os::Printer::log("GLSL: failed to retrieve uniform information", ELL_ERROR);
        return;
    }

    c8 buf[128];

    for (GLint i = 0; i < num; ++i)
    {
        GLint variableSize;
        GLint lenght;
        GLenum varType;
        shader->getDriverOGL()->extGlGetActiveUniform(shader->getProgramId(), i, 127, &lenght, &variableSize, &varType, reinterpret_cast<GLchar*>(buf));

        // ToDo: GL_REFERENCED_BY_* but in future use SPRIV
        //GL_UNIFORM                0           1             2           3               4               5                   6                 7 
        GLenum memberProps[] = { GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_LOCATION };

        GLint memberDescParams[150];
        GLint memberParamsDescLength = 0;
        glGetProgramResourceiv(shader->getProgramId(), GL_UNIFORM, i, 8, memberProps, sizeof(memberDescParams), &memberParamsDescLength, memberDescParams);

        E_SHADER_VARIABLE_TYPE eShaderType = getShaderVariableTypeId(varType);

        SShaderType * _shaderType = CNullShader::GetShaderType(
            eShaderType,
            std::max(1, memberDescParams[1]),
            0,
            memberDescParams[5] > 0 ? memberDescParams[6] ? memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]) / memberDescParams[5] : 0,
            memberDescParams[5] > 0 ? !memberDescParams[6] ? memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]) / memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]),
            0,
            0,
            memberDescParams[4],
            variableSize);

        if (eShaderType == E_SHADER_VARIABLE_TYPE::ESVT_SAMPLER)
        {
            SShaderVariableScalar* var = new SShaderVariableScalar();
            var->mName = buf;
            var->mParent = nullptr;
            var->mBuffer = nullptr;
            var->mOffset = memberDescParams[2];
            //var->mShaderStage = shaderType;
            var->mType = _shaderType;
            var->mLayoutIndex = memberDescParams[7];
            var->mIsValid = true;
            var->mIsDirty = false;
            shader->mTextures.push_back(var);
        }
        else // Uniforms
        {
            SConstantBuffer* irrCB = new SConstantBuffer(shader, E_SHADER_TYPES::EST_HIGH_LEVEL_SHADER); // ToDo: SPIRV
            shader->AddConstantBuffer(irrCB);

            // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
            irrCB->mHwObject = new COpenGLHardwareBuffer(shader->getDriverOGL(), E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT, 0, *irrCB);
            irrCB->mHostMemory.resize(_shaderType->Stride);

            irrCB->mOffset = memberDescParams[2];
            irrCB->mBindPoint = memberDescParams[7];
            irrCB->mName = buf;
            irrCB->mBufferType = ESVT_UNIFORM;

            irrCB->mType = _shaderType;
        }
    }
}

void GLSLReflectAttributes(GLSLGpuShader* shader)
{
    GLint num = 0;
    shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_ATTRIBUTES, &num);
    if (num == 0)
        return;

    GLint maxlen = 0;
    shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxlen);
    if (maxlen == 0)
    {
        os::Printer::log("GLSL: failed to retrieve uniform information", ELL_ERROR);
        return;
    }

    c8 buf[128];

    for (GLint i = 0; i < num; ++i)
    {
        memset(buf, 0, maxlen);

        GLint variableSize;
        GLint lenght;
        GLenum varType;
        shader->getDriverOGL()->pGLGetActiveAttrib(shader->getProgramId(), i, 127, &lenght, &variableSize, &varType, reinterpret_cast<GLchar*>(buf));
        //shader->getDriverOGL()->pGLGetAttribLocation(shader->getProgramId(), buf);

        //GL_PROGRAM_INPUT          0           1             2           3               4
        GLenum memberProps[] = { GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION, GL_IS_PER_PATCH, GL_LOCATION_COMPONENT };

        GLint memberDescParams[5];
        GLint memberParamsDescLength = 0;
        glGetProgramResourceiv(shader->getProgramId(), GL_PROGRAM_INPUT, i, 5, memberProps, sizeof(memberDescParams), &memberParamsDescLength, memberDescParams);

        SShaderVariableScalar* var = new SShaderVariableScalar();
        var->mName = buf;
        var->mParent = nullptr;
        var->mBuffer = nullptr;
        var->mLayoutIndex = memberDescParams[2];

        var->mType = CNullShader::GetShaderType(
            getShaderVariableTypeId(varType),
            std::max(1, memberDescParams[1]),
            0,
            0,
            getShaderVariableVectorSize(memberDescParams[0]),
            0,
            0,
            0,
            variableSize
        );

        var->mIsValid = true;
        var->mIsDirty = false;
        shader->mVertexInput.push_back(var);
    }
}

void GLSLReflectUniformBlocks(GLSLGpuShader* shader)
{
    //GLint num = 0;
    //shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_UNIFORM_BLOCKS, &num);
    //if (num == 0)
    //    return;
    //
    //GLint maxlen = 0;
    //shader->getDriverOGL()->extGlGetProgramiv(shader->getProgramId(), GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxlen);
    //if (maxlen == 0)
    //{
    //    os::Printer::log("GLSL: failed to retrieve uniform information", ELL_ERROR);
    //    return;
    //}
    //
    //c8 buf[128];
    //
    //for (GLint i = 0; i < num; ++i)
    //{
    //    buffers.emplace_back();
    //    CGlslBufferDesc& desc = buffers.back();
    //    memset(buf, 0, maxlen);
    //
    //    desc.varDesc.m_type = ESVT_UNIFORM;
    //    //ui.m_basicVariableLocation = EVertexSemanticType(0);
    //    desc.varDesc.m_length = 0;
    //
    //    GLint size;
    //    glGetActiveUniformBlockName(shader->getProgramId(), i, 127, &size, reinterpret_cast<GLchar*>(buf));
    //    desc.varDesc.m_name = buf;
    //    desc.varDesc.m_location = shader->getDriverOGL()->pGLGetAttribLocation(shader->getProgramId(), buf);
    //    desc.varDesc.m_shaderIndex = buffers.size() - 1;
    //
    //    shader->AddShaderVariable(&desc.varDesc);
    //}
}

irr::video::SShaderVariableStruct* GetParentVariableAndNormalizeName(irr::video::SConstantBuffer* buffdesc, std::string& Name, u32 Elements)
{
    SShaderVariableStruct* parent = nullptr;

    size_t arrayOpIndex = Name.find_last_of('[');
    while (arrayOpIndex != std::string::npos)
    {
        if (arrayOpIndex != std::string::npos)
        {
            Name = Name.substr(0, arrayOpIndex) + Name.substr(arrayOpIndex + 3);
        }

        arrayOpIndex = Name.find_last_of('[');
    };


    arrayOpIndex = Name.find('.');
    while (arrayOpIndex != std::string::npos)
    {
        if (arrayOpIndex != std::string::npos)
        {
            std::string structName = Name.substr(0, arrayOpIndex);

            irr::video::IShaderVariable* currentlevel = buffdesc->getVariableByName(structName.c_str());
            if (!currentlevel)
            {
                SShaderType * _shaderType = CNullShader::GetShaderType(
                    video::E_SHADER_VARIABLE_TYPE::ESVT_VOID,
                    parent ? 1 : Elements,  // ToDo: more inheritance depth
                    1, // Set later
                    0,
                    0,
                    0,
                    0,
                    0,
                    0);

                SShaderVariableStruct* structVar = new SShaderVariableStruct();
                structVar->mName = (parent ? parent->GetName() : "") + (structName).c_str();
                structVar->mParent = parent;
                structVar->mBuffer = buffdesc;
                structVar->mOffset = 0;
                structVar->mLayoutIndex = -1;
                structVar->mType = _shaderType;
                structVar->mIsValid = true;
                structVar->mIsDirty = true;

                if (parent)
                    parent->_asStruct()->mVariables.push_back(structVar);
                else
                    buffdesc->mVariables.push_back(structVar);

                parent = structVar;
            }
            else
            {
                parent = currentlevel->_asStruct();
                assert(parent != nullptr);
            }

            Name = Name.substr(arrayOpIndex + 1);
        }

        arrayOpIndex = Name.find_last_of('.');
    };

    return parent;
}

void InitializeBaseOffsetConstantStructOGL(irr::video::SConstantBuffer* buffdesc, SShaderVariableStruct* structVar)
{
    u32 baseOffset = buffdesc->mType->Stride;
    for (int m = 0; m < structVar->mVariables.size(); ++m)
    {
        auto pVariable = structVar->mVariables[m];

        if (pVariable->_asStruct() && pVariable->getType()->Members > 0)
        {
            InitializeBaseOffsetConstantStructOGL(buffdesc, pVariable->_asStruct());
        }

        if (pVariable->_getOffset() < baseOffset)
            baseOffset = pVariable->_getOffset();
    }

    structVar->mOffset = baseOffset;
    structVar->mType->Members = structVar->mVariables.size();
}

void InitializeBaseAlignConstantStructOGL(irr::video::SConstantBuffer* buffdesc, SShaderVariableStruct* structVar)
{
    std::sort(structVar->mVariables.begin(), structVar->mVariables.end(), [](irr::video::IShaderVariable* lhs, irr::video::IShaderVariable* rhs)
    {
        return lhs->_getOffset() < rhs->_getOffset();
    });

    for (int m = 0; m < structVar->mVariables.size(); ++m)
    {
        auto pVariable = structVar->mVariables[m];

        if (pVariable->_asStruct() && pVariable->getType()->Members > 0)
        {
            InitializeBaseOffsetConstantStructOGL(buffdesc, pVariable->_asStruct());
        }

        auto pVariableNext = structVar->mVariables.size() > m + 1 ? structVar->mVariables[m + 1] : nullptr;

        uint32 stride = (pVariableNext ? pVariableNext->_getOffset() : structVar->_getOffset() + structVar->getType()->ArrayStride) - pVariable->_getOffset();
        uint32 elementSize = stride / pVariable->getType()->Elements;

        pVariable->getType()->ArrayStride = elementSize;
        pVariable->getType()->Stride = stride;
    }
}

void GLSLGpuShader::ReflParseStruct(irr::video::SConstantBuffer* buffdesc, irr::video::IShaderVariable* parent, const u32* memberParams, std::vector<irr::video::IShaderVariable*>& Variables, std::string namePrefix)
{
    u32 memberCount = parent ? parent->getType()->Members : buffdesc->getType()->Members;
    if (memberCount == 0)
        return;

    GLint Bufsize;
    c8 buf[128];

    for (int m = 0; m < memberCount; ++m)
    {
        GLint memberId = memberParams[m];

        glGetProgramResourceName(getProgramId(), GL_BUFFER_VARIABLE, memberId, 127, &Bufsize, buf);

        //GL_BUFFER_VARIABLE        0           1             2           3               4               5                   6                 7                         8
        GLenum memberProps[] = { GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE };

        GLint memberDescParams[150];
        GLint memberParamsDescLength = 0;
        glGetProgramResourceiv(getProgramId(), GL_BUFFER_VARIABLE, memberId, 9, memberProps, sizeof(memberDescParams), &memberParamsDescLength, memberDescParams);

        std::string Name = buf;
        SShaderVariableStruct* structParent = GetParentVariableAndNormalizeName(buffdesc, Name, memberDescParams[7]);

        if (structParent)
            structParent->mVariables.push_back(nullptr);
        else
            Variables.push_back(nullptr);

        irr::video::IShaderVariable*& structDecl = structParent ? structParent->mVariables.back() : Variables.back();

        video::E_SHADER_VARIABLE_TYPE eShaderType = getShaderVariableTypeId(memberDescParams[0]);

        SShaderType * _shaderType = CNullShader::GetShaderType(
            eShaderType,
            structParent ? 1 : std::max(1, memberDescParams[7]),
            0, // Set later
            structParent ? 0 : memberDescParams[5] > 0 ? memberDescParams[6] ? memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]) / memberDescParams[5] : 0,
            structParent ? 0 : memberDescParams[5] > 0 ? !memberDescParams[6] ? memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]) / memberDescParams[5] : getShaderVariableVectorSize(memberDescParams[0]),
            0,
            0,
            structParent ? 0 : memberDescParams[4],
            structParent ? 0 : memberDescParams[7] > 1 ? memberDescParams[8] : memberDescParams[4]);

        if (memberDescParams[5] > 0)
        {
            SShaderVariableMatrix* matVar = new SShaderVariableMatrix();
            matVar->mName = (namePrefix + buf).c_str();
            matVar->mParent = parent;
            matVar->mBuffer = buffdesc;
            matVar->mOffset = memberDescParams[2];
            matVar->mLayoutIndex = memberDescParams[3];
            matVar->mType = _shaderType;
            matVar->mIsRowMajor = memberDescParams[5];
            matVar->mIsValid = true;
            matVar->mIsDirty = true;

            structDecl = matVar;
        }
        else
        {
            SShaderVariableScalar* var = new SShaderVariableScalar();
            var->mName = (namePrefix + buf).c_str();
            var->mParent = parent;
            var->mBuffer = buffdesc;
            var->mOffset = memberDescParams[2];
            var->mLayoutIndex = memberDescParams[3];
            var->mType = _shaderType;
            var->mIsValid = true;
            var->mIsDirty = true;

            structDecl = var;
        }
    }

    for (int m = 0; m < Variables.size(); ++m)
    {
        auto pVariable = Variables[m];

        if (pVariable->_asStruct() && pVariable->getType()->Members > 0)
        {
            InitializeBaseOffsetConstantStructOGL(buffdesc, pVariable->_asStruct());
        }
    }

    std::sort(Variables.begin(), Variables.end(), [](irr::video::IShaderVariable* lhs, irr::video::IShaderVariable* rhs)
    {
        return lhs->_getOffset() < rhs->_getOffset();
    });

    for (int m = 0; m < Variables.size() - 1; ++m)
    {
        auto pVariable = Variables[m];
        auto pVariableNext = Variables[m + 1];

        uint32 stride = pVariableNext->_getOffset() - pVariable->_getOffset();
        uint32 elementSize = stride / pVariable->getType()->Elements;

        pVariable->getType()->ArrayStride = elementSize;
        pVariable->getType()->Stride = stride;
    }

    auto& pVariable = Variables.back();
    uint32 stride = buffdesc->getType()->Stride - pVariable->_getOffset();
    uint32 elementSize = stride / pVariable->getType()->Elements;

    pVariable->getType()->ArrayStride = elementSize;
    pVariable->getType()->Stride = stride;

    // Initialize sub struct aligns
    for (int m = 0; m < Variables.size(); ++m)
    {
        auto pVariable = Variables[m];
    
        if (pVariable->_asStruct() && pVariable->getType()->Members > 0)
        {
            InitializeBaseAlignConstantStructOGL(buffdesc, pVariable->_asStruct());
        }
    }
}

void GLSLReflectStorageBlocks(GLSLGpuShader* shader)
{
    GLint bufferCount = 0;
    glGetProgramInterfaceiv(shader->getProgramId(), GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &bufferCount);
    if (bufferCount == 0)
        return;

    GLint maxlen = 0;
    glGetProgramInterfaceiv(shader->getProgramId(), GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxlen);
    if (maxlen == 0)
    {
        os::Printer::log("GLSL: failed to retrieve uniform information", ELL_ERROR);
        return;
    }

    // seems that some implementations use an extra null terminator
    GLint Bufsize;
    c8 buf[128];

    for (GLint i = 0; i < bufferCount; ++i)
    {
        memset(buf, 0, maxlen);
        glGetProgramResourceName(shader->getProgramId(), GL_SHADER_STORAGE_BLOCK, i, 127, &Bufsize, buf);

        SConstantBuffer* irrCB = new SConstantBuffer(shader, E_SHADER_TYPES::EST_HIGH_LEVEL_SHADER); // ToDo: SPIRV
        shader->AddConstantBuffer(irrCB);

        irrCB->mOffset = i;
        irrCB->mName = buf;
        irrCB->mBufferType = ESVT_CONSTANT;

        GLenum props[] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES, GL_ACTIVE_VARIABLES };

        GLint memberParams[150];
        GLint memberParamsLength = 0;
        glGetProgramResourceiv(shader->getProgramId(), GL_SHADER_STORAGE_BLOCK, i, 4, props, sizeof(memberParams), &memberParamsLength, memberParams);


        // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
        irrCB->mHwObject = new COpenGLHardwareBuffer(shader->getDriverOGL(), E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT, 0, *irrCB);
        irrCB->mHostMemory.resize(memberParams[1]);
        //irrCB->mRowMajor = false;
        //irrCB->mLayout16ByteAlign = ;

        irrCB->mBindPoint = memberParams[0];
        irrCB->mType = CNullShader::GetShaderType(
            E_SHADER_VARIABLE_TYPE::ESVT_BUFFER,
            1,
            memberParams[2],
            0,
            0,
            0,
            0,
            0,
            memberParams[1]);

        shader->ReflParseStruct(irrCB, nullptr, (u32*)(memberParams + 3), irrCB->mVariables, "");
    }
}

void GLSLGpuShader::buildShaderVariableDescriptor()
{
    GLint total = 0, num = 0;
    glGetProgramInterfaceiv(getProgramId(), GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num);
    total += num;
    getDriverOGL()->extGlGetProgramiv(getProgramId(), GL_ACTIVE_UNIFORM_BLOCKS, &num);
    total += num;
    getDriverOGL()->extGlGetProgramiv(getProgramId(), GL_ACTIVE_UNIFORMS, &num);
    total += num;

    GLSLReflectStorageBlocks(this);
    // Find other legacy binding
    GLSLReflectUniform(this);
    GLSLReflectUniformBlocks(this);

    // Find Attributes
    GLSLReflectAttributes(this);
}

//void GLSLGpuShader::CommitValues(IShaderDataBuffer::E_UPDATE_TYPE updateType)
//{
//    IShader::CommitValues(updateType);
//}

//void GLSLGpuShader::bind()
//{
//    _IRR_DEBUG_BREAK_IF(getDriverOGL()->GetBindedProgramId() && getDriverOGL()->GetBindedProgramId() != getProgramId());
//    //if (!mBinded)
//    {
//        mBinded = true;
//        getVideoDriver()->useShader(this);
//        for (auto unit : this->TextureUnits)
//            glUniform1i(unit.varDesc.m_location, unit.binding);
//    }
//}
//
//void GLSLGpuShader::unbind()
//{
//    //if (mBinded)
//    {
//        mBinded = false;
//        _IRR_DEBUG_BREAK_IF(getDriverOGL()->GetBindedProgramId() && getDriverOGL()->GetBindedProgramId() != getProgramId());
//        getVideoDriver()->useShader(nullptr);
//    }
//}

/// OpenGL Driver Shader Spec

//ToDo: use SPIR-V optimized instead legacy OpenGL shader compile
IShader * COpenGLDriver::createShader(ShaderInitializerEntry * shaderCreateInfo)
{
    std::list<u32> objectList;
    u32 programId = extGlCreateProgram();

    if (programId == 0)
    {
        printf("Error creating shader program\n");
        throw std::runtime_error("shader not found");
    }

    GLSLGpuShader* gpuProgram = new GLSLGpuShader(this, programId);

    for (auto stage : shaderCreateInfo->mStages)
    {
        if (!stage->DataStream)
            continue;

        const char* ShaderModel = stage->ShaderModel.c_str();
        const char* ShaderEntryPoint = stage->EntryPoint.c_str();

        stage->DataStream->Seek(stage->DataStreamOffset, false);
        switch (stage->ShaderStageType)
        {
            case E_SHADER_TYPES::EST_VERTEX_SHADER:
                gpuProgram->addShaderFile(gpuProgram, GL_VERTEX_SHADER, stage->DataStream, objectList);
                break;
            case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
                gpuProgram->addShaderFile(gpuProgram, GL_GEOMETRY_SHADER, stage->DataStream, objectList);
                break;
            case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
                gpuProgram->addShaderFile(gpuProgram, GL_FRAGMENT_SHADER, stage->DataStream, objectList);
                break;
        }
    }

    //addShaderFile(gpuProgram, GL_TESSELLATION_SHADER, tesselationShader, objectList);
    gpuProgram->compile(gpuProgram, objectList);

    gpuProgram->buildShaderVariableDescriptor();

    for (auto cbEntry : shaderCreateInfo->Callback)
    {
        auto buffer = gpuProgram->getConstantBufferByName(cbEntry.first.c_str());
        if (buffer)
            buffer->setShaderDataCallback(cbEntry.second);
        cbEntry.second->OnPrepare(buffer);
    }

    gpuProgram->Init();
    shaderCreateInfo->mShaderId = AddShaderModul(gpuProgram, shaderCreateInfo->mShaderId);
    return gpuProgram;
}


void COpenGLDriver::useShader(IShader* gpuProgram)
{
    if (gpuProgram)
    {
        if (gpuProgram->getShaderType() == ESV_GLSL_HIGH_LEVEL)
        {
            GLSLGpuShader* glsl = static_cast<GLSLGpuShader*>(gpuProgram);
            //if (mBindedProgram != glsl->getProgramId())
            {
                //_IRR_DEBUG_BREAK_IF(mBindedProgram);
                mBindedProgram = glsl->getProgramId();
                extGlUseProgram(glsl->getProgramId());
                for (u8 i = 0; i != glsl->mTextures.size(); ++i)
                {
                    glUniform1i(glsl->mTextures[i]->_getIndex(), glsl->mTextures[i]->_getIndex());
                }
                
            }
        }
        else // ASM Shader
        {
            //_IRR_DEBUG_BREAK_IF(mBindedProgram);
            //mBindedProgram = programId;
            //getDriverOGL()->extGlBindProgram(glShaderType, programId);
            //GetStateCache()->setEnabled(glShaderType, true);

            //_IRR_DEBUG_BREAK_IF(!mBindedProgram || mBindedProgram != programId);
            //mBindedProgram = 0;
            //getDriverOGL()->extGlBindProgram(glShaderType, 0);
            //GetStateCache()->setEnabled(glShaderType, false);
        }
    }
    else
    {
        mBindedProgram = 0;
        extGlUseProgram(0);

        for (auto& buffer : BindedBuffers)
        {
            if (!buffer)
                continue;

            buffer->Unbind();
            buffer = nullptr;
        }
    }
}

void COpenGLDriver::deleteShader(IShader* gpuProgram)
{
    if (gpuProgram->getShaderType() == ESV_GLSL_HIGH_LEVEL)
    {
        GLSLGpuShader* glsl = (GLSLGpuShader*)gpuProgram;
        if (glsl->getProgramId())
            if (mBindedProgram == glsl->getProgramId())
                useShader(nullptr);
        extGlDeleteProgram(glsl->getProgramId());
    }
}

//bool COpenGLDriver::setShaderConstant(ShaderVariableDescriptor const* desc, const void* values, int count, IHardwareBuffer* buffer /*= nullptr*/)
//{
//    if (!desc || desc->m_location < 0 || !values || !count)
//        return false;
//
//    irr::video::GLSLGpuShader* glsl = static_cast<irr::video::GLSLGpuShader*>(GetActiveShader());
//
//    // Attribute never can be modify here
//    if (desc->m_type == ESVT_INPUT_STREAM || desc->m_type == ESVT_ATTRIBUTE)
//        return false;
//
//    // Modern Buffers (Directx 11 like)
//    if (desc->m_type == ESVT_CONSTANT)
//    {
//        if (!glsl)
//            return false;
//
//        irr::video::CGlslBufferDesc& ubuffer = glsl->ShaderBuffers[desc->m_shaderIndex];
//        irr::video::CGlslVariableDesc const& constantDecl = ubuffer.members[desc->m_location & 0xFF];
//
//        UINT elementSize = constantDecl.elementSize * count;
//
//        _IRR_DEBUG_BREAK_IF(elementSize > constantDecl.dataSize || (ubuffer.dataSize < (constantDecl.offset + elementSize)));
//
//        if (memcmp(&ubuffer.DataBuffer[constantDecl.offset], values, elementSize))
//        {
//            if (ubuffer.ChangeStartOffset > constantDecl.offset)
//                ubuffer.ChangeStartOffset = constantDecl.offset;
//
//            if (ubuffer.ChangeEndOffset < (constantDecl.offset + elementSize))
//                ubuffer.ChangeEndOffset = (constantDecl.offset + elementSize);
//
//            memcpy(&ubuffer.DataBuffer[constantDecl.offset], values, elementSize);
//            ++ubuffer.ChangeId;
//        }
//        return true;
//    }
//
//    // Legacy uniform set
//
//    bool status = true;
//
//    switch (desc->m_variableType)
//    {
//        case GL_FLOAT:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniform1fv(desc->m_location, count, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_VEC2:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniform2fv(desc->m_location, count, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_VEC3:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniform3fv(desc->m_location, count, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_VEC4:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniform4fv(desc->m_location, count, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_MAT2:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniformMatrix2fv(desc->m_location, count, false, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_MAT3:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniformMatrix3fv(desc->m_location, count, false, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_FLOAT_MAT4:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniformMatrix4fv(desc->m_location, count, false, (float*)values);
//            else
//                status = false;
//            break;
//        case GL_INT:
//        case GL_SAMPLER_1D:
//        case GL_SAMPLER_2D:
//        case GL_SAMPLER_3D:
//        case GL_SAMPLER_CUBE:
//        case GL_SAMPLER_1D_SHADOW:
//        case GL_SAMPLER_2D_SHADOW:
//            if (desc->m_type == ESVT_UNIFORM)
//                extGlUniform1iv(desc->m_location, count, (s32*)values);
//            else
//                status = false;
//            break;
//        default:
//            status = false;
//            break;
//    }
//    _IRR_DEBUG_BREAK_IF(testGLError());
//    return status;
//}

bool COpenGLDriver::SyncShaderConstant()
{
    irr::video::GLSLGpuShader* glsl = static_cast<irr::video::GLSLGpuShader*>(GetActiveShader());

    for (u32 ib = 0; ib != glsl->mBuffers.size(); ++ib)
    {
        irr::video::SConstantBuffer* cbuffer = glsl->mBuffers[ib];

        if (cbuffer->mCallBack)
            cbuffer->mCallBack->OnSetConstants(cbuffer, cbuffer->mHwObject->GetBuffer());

        if (!cbuffer->mHwObject)
            continue;

        if (cbuffer->getHardwareMappingHint() == scene::EHM_NEVER)
            cbuffer->setHardwareMappingHint(scene::EHM_STATIC);

        {
            if (cbuffer->mHwObject->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS) != cbuffer->getChangedID())
            {
                E_HARDWARE_BUFFER_ACCESS MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DEFAULT;
                if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_DYNAMIC)
                    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC;
                else if (cbuffer->getHardwareMappingHint() == scene::E_HARDWARE_MAPPING::EHM_STREAM)
                    MemoryAccess = E_HARDWARE_BUFFER_ACCESS::EHBA_SYSTEM_MEMORY;


                if (static_cast<COpenGLHardwareBuffer*>(cbuffer->mHwObject)->UpdateBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, MemoryAccess, cbuffer->mHostMemory.data(),
                    cbuffer->mHostMemory.size()/*, cbuffer->ChangeStartOffset, cbuffer->ChangeEndOffset*/))
                {
                    //cbuffer->ChangeStartOffset = cbuffer->DataBuffer.size();
                    //cbuffer->ChangeEndOffset = 0;
                    cbuffer->mHwObject->setChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, cbuffer->getChangedID());
                }
            }
        }

        u32 minSize = std::max(u32(cbuffer->getBindingIndex()), 0u);
        if (BindedBuffers.size() <= (int)minSize)
            BindedBuffers.resize(cbuffer->getBindingIndex() + 1);

        if (BindedBuffers[cbuffer->getBindingIndex()] != cbuffer->mHwObject)
        {
            //BindedBuffers[cbuffer->binding]->Unbind();
            cbuffer->mHwObject->Bind();
            BindedBuffers[cbuffer->getBindingIndex()] = static_cast<COpenGLHardwareBuffer*>(cbuffer->mHwObject);
        }
    }

    return true;
}
