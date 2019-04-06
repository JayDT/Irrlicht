#include "CVulkanShader.h"
#include "CVulkanDriver.h"
#include "CVulkanHardwareBuffer.h"
#include "CVulkanShaderCompiler.h"
#include "CVulkanDescriptorLayout.h"
#include "CVulkanGpuPipelineState.h"
#include "CVulkanGpuParams.h"
#include "CVulkanDevice.h"
#include "os.h"
#include "IMaterialRenderer.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/MachineIndependent/reflection.h"
#include "SPIRV/GlslangToSpv.h"
#include "glslang/Include/Common.h"

using namespace irr;
using namespace irr::video;

extern EShLanguage GetCompileLang(E_SHADER_TYPES type);

core::matrix4 VK_UnitMatrix;
core::matrix4 VK_SphereMapMatrix;

namespace
{
    std::map<size_t, core::array<uint8_t>> mSharedBinaryCache;

    void VKWriteShaderCache(System::IO::IFileWriter* file)
    {
        if (!file)
            return;

        *file << uint32_t('ISDC');
        *file << uint32_t('IVKD');
        *file << uint32_t(mSharedBinaryCache.size());

        for (const auto& kvp : mSharedBinaryCache)
        {
            *file << uint64_t(kvp.first);
            *file << uint32_t(kvp.second.size());
            file->Write((uint8_t*)kvp.second.const_pointer(), kvp.second.size());
        }
    }
}

extern "C" void VKLoadShaderCache(System::IO::IFileReader* file)
{
    if (!file)
        return;

    uint32_t shaderCount;

    *file >> shaderCount;

    for (uint32_t i = 0; i < shaderCount; ++i)
    {
        uint64_t seed;
        uint32_t size;
        *file >> seed;
        *file >> size;
        auto& cacheBuffer = mSharedBinaryCache[seed];
        cacheBuffer.set_used(size);
        file->Read((uint8_t*)cacheBuffer.pointer(), cacheBuffer.size());
    }
}

void CVulkanDriver::WriteShaderCache(System::IO::IFileWriter* file)
{
    VKWriteShaderCache(file);
}

irr::video::CVulkanGLSLProgram::CVulkanGLSLProgram(video::IVideoDriver * context, E_SHADER_LANG type)
    : CNullShader(context, type)
    , CVulkanDeviceResource(static_cast<CVulkanDriver*>(context))
{
}

irr::video::CVulkanGLSLProgram::~CVulkanGLSLProgram()
{
    if (mLayout)
        delete mLayout;

    if (mParams)
        mParams->drop();

    for (auto modul : mStages)
    {
        vkDestroyShaderModule(Driver->_getPrimaryDevice()->getLogical(), modul.second, VulkanDevice::gVulkanAllocator);
    }
}

void irr::video::CVulkanGLSLProgram::Init()
{
    getLayout();
    mParams = new VulkanGpuParams(Driver, this, GpuDeviceFlags::GDF_PRIMARY);
    mParams->initialize();
}

bool irr::video::CVulkanGLSLProgram::enumInputLayout(void *)
{
    return false;
}

void irr::video::CVulkanGLSLProgram::BuildBufferDesc(irr::video::CVulkanGLSLang & compiler, E_SHADER_TYPES type)
{
}

bool irr::video::CVulkanGLSLProgram::initializeUniforms(irr::video::CVulkanGLSLang& compiler, E_SHADER_TYPES shaderType)
{
    uint32 num = compiler.Reflection->getNumUniforms();
    for (uint32 i = 0; i < num; ++i)
    {
        const auto& bufferEntry = compiler.Reflection->getUniform(i);

        if (bufferEntry.getType()->getBasicType() == glslang::TBasicType::EbtSampler)
        {
            auto var = irr::MakePtr<SShaderSampler>();
            var->mName = bufferEntry.name.c_str();
            var->mParent = nullptr;
            var->mBuffer = nullptr;
            var->mBindPoint = bufferEntry.getBinding();
            var->mShaderStage = shaderType;
            var->mType = CNullShader::GetShaderType(
                VulkanUtility::getShaderVariableTypeId(bufferEntry.getType()->getBasicType()),
                bufferEntry.getType()->getArraySizes() ? bufferEntry.getType()->getArraySizes()->getDimSize(0) : 1,
                bufferEntry.getType()->getStruct() ? bufferEntry.getType()->getStruct()->size() : 0,
                bufferEntry.getType()->getMatrixRows(),
                bufferEntry.getType()->isMatrix() ? bufferEntry.getType()->getMatrixCols() : bufferEntry.getType()->getVectorSize(),
                0,
                0,
                0,
                bufferEntry.size
            );
            if (bufferEntry.getType()->getQualifier().hasIndex())
                var->mLayoutIndex = bufferEntry.getType()->getQualifier().layoutIndex;
            else
                var->mLayoutIndex = -1;
            var->mIsValid = true;
            mTextures.push_back(var);
        }
        //else // Uniforms
        //{
        //    SConstantBuffer* irrCB = new SConstantBuffer(this, shaderType);
        //    AddConstantBuffer(irrCB);
        //
        //    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
        //    irrCB->mHwObject = static_cast<CVulkanHardwareBuffer*>(Driver->createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC, bufferEntry.size));
        //    irrCB->mHostMemory.resize(bufferEntry.size);
        //
        //    irrCB->mOffset = bufferEntry.offset;
        //    irrCB->mBindPoint = bufferEntry.getBinding();
        //    irrCB->mName = bufferEntry.name.c_str();
        //    irrCB->mBufferType = ESVT_UNIFORM;
        //
        //    irrCB->mType = CNullShader::GetShaderType(
        //        VulkanUtility::getShaderVariableTypeId(bufferEntry.getType()->getBasicType()),
        //        bufferEntry.getType()->getArraySizes() ? bufferEntry.getType()->getArraySizes()->getDimSize(0) : 1,
        //        bufferEntry.getType()->getStruct() ? bufferEntry.getType()->getStruct()->size() : 0,
        //        bufferEntry.getType()->getMatrixRows(),
        //        bufferEntry.getType()->isMatrix() ? bufferEntry.getType()->getMatrixCols() : bufferEntry.getType()->getVectorSize(),
        //        0,
        //        0,
        //        0,
        //        bufferEntry.size);
        //
        //    ReflParseStruct(irrCB, nullptr, bufferEntry.getType(), irrCB->mVariables, "", bufferEntry.size);
        //}
    }

    return true;
}

void irr::video::CVulkanGLSLProgram::ReflParseStruct(glslang::TIntermediate* shaderIntermediate, SConstantBuffer* buffdesc, irr::video::IShaderVariable* parent, const glslang::TType* type,
    std::vector<irr::video::IShaderVariable*>& Variables, std::string namePrefix, u32 pParentSize)
{
    uint32 elementNum = type->getStruct()->size();
    Variables.resize(elementNum);
    for (int m = 0; m < elementNum; ++m)
    {
        const glslang::TTypeLoc& pVariable = (*type->getStruct())[m];

        irr::video::IShaderVariable*& structDecl = Variables[m];

        int memberSize;
        int dummyStride;
        u32 Offset;
        glslang::TLayoutMatrix matrixLayout = pVariable.type->getQualifier().layoutMatrix;
        int memberAlignment = shaderIntermediate->getBaseAlignment(*pVariable.type, memberSize, dummyStride, buffdesc->mLayout16ByteAlign,
            matrixLayout != glslang::ElmNone ? matrixLayout == glslang::ElmRowMajor : buffdesc->mRowMajor);

        if (!pVariable.type->getQualifier().hasOffset())
        {
            if (m > 0)
            {
                int memberSizePrev;
                int dummyStridePrev;
                const glslang::TTypeLoc& pVariablePrev = (*type->getStruct())[m - 1];
                glslang::TLayoutMatrix subMatrixLayout = pVariablePrev.type->getQualifier().layoutMatrix;
                int memberAlignment = shaderIntermediate->getBaseAlignment(*pVariablePrev.type, memberSizePrev, dummyStridePrev, buffdesc->mLayout16ByteAlign,
                    subMatrixLayout != glslang::ElmNone ? subMatrixLayout == glslang::ElmRowMajor : buffdesc->mRowMajor);

                Offset = Variables[m - 1]->_getOffset() + memberSizePrev;
                // "The actual alignment of a member will be the greater of the specified align alignment and the standard
                // (e.g., std140) base alignment for the member's type."
                if (pVariable.type->getQualifier().hasAlign())
                    memberAlignment = std::max(memberAlignment, pVariable.type->getQualifier().layoutAlign);

                // "If the resulting offset is not a multiple of the actual alignment,
                // increase it to the first offset that is a multiple of
                // the actual alignment."
                glslang::RoundToPow2(Offset, memberAlignment);
            }
            else
            {
                Offset = parent->_getOffset();
            }
        }
        else
        {
            Offset = pVariable.type->getQualifier().layoutOffset;
        }

        //_IRR_DEBUG_BREAK_IF(!pVariable.type->getQualifier().hasOffset());
        //if (pVariable.type->getQualifier().hasAlign())
        //    structDecl.varDesc.m_size = pVariable.type->getQualifier().layoutAlign;

        video::E_SHADER_VARIABLE_TYPE eShaderType = VulkanUtility::getShaderVariableTypeId(pVariable.type->getBasicType());
        
        SShaderType * _shaderType = CNullShader::GetShaderType(
            eShaderType,
            pVariable.type->getArraySizes() ? pVariable.type->getArraySizes()->getDimSize(0) : 1,
            pVariable.type->getStruct() ? pVariable.type->getStruct()->size() : 0,
            pVariable.type->getMatrixRows(),
            pVariable.type->isMatrix() ? pVariable.type->getMatrixCols() : pVariable.type->getVectorSize(),
            memberSize,
            memberAlignment,
            0,
            0
        );

        if (pVariable.type->getStruct())
        {
            SShaderVariableStruct* structVar = new SShaderVariableStruct();
            structVar->mName = pVariable.type->getFieldName().c_str(); // (namePrefix + pVariable.type->getFieldName().c_str()).c_str();
            structVar->mParent = parent;
            structVar->mBuffer = buffdesc;
            structVar->mOffset = Offset;
            if (pVariable.type->getQualifier().hasIndex())
                structVar->mLayoutIndex = pVariable.type->getQualifier().layoutIndex;
            else
                structVar->mLayoutIndex = -1;
            structVar->mType = _shaderType;
            structVar->mIsValid = true;
            structVar->mIsDirty = true;
            structVar->mLoaderData = pVariable.type;

            structDecl = structVar;
        }
        else if (pVariable.type->isMatrix())
        {
            SShaderVariableMatrix* matVar = new SShaderVariableMatrix();
            matVar->mName = pVariable.type->getFieldName().c_str(); //(namePrefix + pVariable.type->getFieldName().c_str()).c_str();
            matVar->mParent = parent;
            matVar->mBuffer = buffdesc;
            matVar->mOffset = Offset;
            if (pVariable.type->getQualifier().hasIndex())
                matVar->mLayoutIndex = pVariable.type->getQualifier().layoutIndex;
            else
                matVar->mLayoutIndex = -1;
            matVar->mType = _shaderType;
            matVar->mIsRowMajor = pVariable.type->getQualifier().layoutMatrix == glslang::TLayoutMatrix::ElmRowMajor;
            matVar->mIsValid = true;
            matVar->mIsDirty = true;

            structDecl = matVar;
        }
        else if (pVariable.type->isScalar() || pVariable.type->isVector())
        {
            SShaderVariableScalar* var = new SShaderVariableScalar();
            var->mName = pVariable.type->getFieldName().c_str(); //(namePrefix + pVariable.type->getFieldName().c_str()).c_str();
            var->mParent = parent;
            var->mBuffer = buffdesc;
            var->mOffset = Offset;
            if (pVariable.type->getQualifier().hasIndex())
                var->mLayoutIndex = pVariable.type->getQualifier().layoutIndex;
            else
                var->mLayoutIndex = -1;
            var->mType = _shaderType;
            var->mIsValid = true;
            var->mIsDirty = true;

            structDecl = var;
        }
    }

    std::sort(Variables.begin(), Variables.end(), [](irr::video::IShaderVariable* lhs, irr::video::IShaderVariable* rhs)
    {
        return lhs->_getOffset() < rhs->_getOffset();
    });

    for (int m = 0; m < elementNum - 1; ++m)
    {
        auto pVariable = Variables[m];
        auto pVariableNext = Variables[m + 1];

        uint32 stride = pVariable->getType()->Stride == 0 ? pVariableNext->_getOffset() - pVariable->_getOffset() : pVariable->getType()->Stride;
        assert((pVariableNext->_getOffset() - pVariable->_getOffset()) == stride);
        uint32 elementSize = stride / pVariable->getType()->Elements;

        pVariable->getType()->ArrayStride = elementSize;
        pVariable->getType()->Stride = stride;
    }

    auto& pVariable = Variables.back();
    uint32 stride = pVariable->getType()->Stride == 0 ? ((parent ? (parent->_getOffset() + pParentSize) : pParentSize) - pVariable->_getOffset()) : pVariable->getType()->Stride; // ToDo
    assert(((parent ? (parent->_getOffset() + pParentSize) : pParentSize) - pVariable->_getOffset()) == stride);
    uint32 elementSize = stride / pVariable->getType()->Elements;

    pVariable->getType()->ArrayStride = elementSize;
    pVariable->getType()->Stride = stride;

    for (int m = 0; m < elementNum; ++m)
    {
        auto pVariable = Variables[m];

        if (pVariable->_asStruct() && pVariable->getType()->Members > 0)
        {
            std::string _namePrefix = (namePrefix + pVariable->GetName().c_str()).c_str();
            _namePrefix += '.';

            ReflParseStruct(shaderIntermediate, buffdesc, pVariable, static_cast<glslang::TType*>(pVariable->_asStruct()->mLoaderData), pVariable->_asStruct()->mVariables, _namePrefix, pVariable->getType()->Stride);
        }
    }
}

bool irr::video::CVulkanGLSLProgram::initializeConstantBuffers(irr::video::CVulkanGLSLang& compiler, E_SHADER_TYPES shaderType)
{
    glslang::TIntermediate* shaderIntermediate = compiler.Program->getIntermediate(GetCompileLang(shaderType));

    uint32 num = compiler.Reflection->getNumUniformBlocks();
    for (uint32 i = 0; i < num; ++i)
    {
        const auto& bufferEntry = compiler.Reflection->getUniformBlock(i);

        SConstantBuffer* irrCB = new SConstantBuffer(this, shaderType);
        AddConstantBuffer(irrCB);

        // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
        irrCB->mHwObject = static_cast<CVulkanHardwareBuffer*>(Driver->createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC, bufferEntry.size));
        irrCB->mHostMemory.resize(bufferEntry.size);

        irrCB->mOffset = bufferEntry.offset;
        irrCB->mBindPoint = bufferEntry.getBinding();
        irrCB->mName = bufferEntry.name.c_str();
        irrCB->mBufferType = ESVT_CONSTANT;
        irrCB->mRowMajor = bufferEntry.getType()->getQualifier().layoutMatrix == glslang::TLayoutMatrix::ElmRowMajor;
        irrCB->mLayout16ByteAlign = bufferEntry.getType()->getQualifier().layoutAlign != glslang::TLayoutPacking::ElpStd140;

        irrCB->mType = CNullShader::GetShaderType(
            VulkanUtility::getShaderVariableTypeId(bufferEntry.getType()->getBasicType()),
            1,
            bufferEntry.getType()->getStruct()->size(),
            0,
            1,
            0,
            0,
            0,
            bufferEntry.size);

        ReflParseStruct(shaderIntermediate, irrCB, nullptr, bufferEntry.getType(), irrCB->mVariables, "", bufferEntry.size);
    }
    return true;
}

bool irr::video::CVulkanGLSLProgram::CreateShaderModul(E_SHADER_TYPES type, CVulkanDriver * device, System::IO::IFileReader * file, const char * entryPoint, const char* shaderModel)
{
    irr::video::CVulkanGLSLang compiler(Driver);
    bool result = compiler.CompileShader(GetShaderModule(type), type, file, entryPoint, shaderModel);
    if (result)
    {
        mStages[type].first = entryPoint;

        if (type == E_SHADER_TYPES::EST_VERTEX_SHADER)
        {
            uint32 num = compiler.Reflection->getNumAttributes();
            for (uint32 i = 0; i < num; ++i)
            {
                const auto& attrEntry = compiler.Reflection->getAttribute(i);

                auto var = irr::MakePtr<SShaderVariableScalar>();
                var->mName = attrEntry.name.c_str();
                var->mParent = nullptr;
                var->mBuffer = nullptr;
                var->mOffset = attrEntry.offset;
                if (attrEntry.getType()->getQualifier().hasIndex())
                    var->mLayoutIndex = attrEntry.getType()->getQualifier().layoutIndex; // attrEntry.index;
                else
                    var->mLayoutIndex = i;

                if (attrEntry.getType()->getQualifier().semanticName)
                    var->mSemantic = attrEntry.getType()->getQualifier().semanticName;

                var->mType = CNullShader::GetShaderType(
                    VulkanUtility::getShaderVariableTypeId(attrEntry.getType()->getBasicType()),
                    attrEntry.getType()->getArraySizes() ? attrEntry.getType()->getArraySizes()->getDimSize(0) : 1,
                    0,
                    attrEntry.getType()->getMatrixRows(),
                    attrEntry.getType()->isMatrix() ? attrEntry.getType()->getMatrixCols() : attrEntry.getType()->getVectorSize(),
                    0,
                    0,
                    attrEntry.getType()->getArraySizes() ? attrEntry.size / attrEntry.getType()->getArraySizes()->getDimSize(0) : 0,
                    attrEntry.size
                );

                var->mIsValid = true;
                var->mIsDirty = false;
                mVertexInput.push_back(var);
            }
        }

        initializeConstantBuffers(compiler, type);
        initializeUniforms(compiler, type);
    }
    return result;
}

IConstantBuffer* irr::video::CVulkanGLSLProgram::AddUnknownBuffer(E_SHADER_TYPES shaderType, u32 size)
{
    auto irrCB = irr::MakePtr<SConstantBuffer>(this, shaderType);
    AddConstantBuffer(irrCB);

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    irrCB->mHwObject = static_cast<CVulkanHardwareBuffer*>(Driver->createHardwareBuffer(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS, E_HARDWARE_BUFFER_ACCESS::EHBA_DYNAMIC, size));
    irrCB->mHostMemory.resize(size);

    irrCB->mOffset = 0;
    irrCB->mBindPoint = 0;
    irrCB->mName = "data";
    irrCB->mBufferType = ESVT_CONSTANT;

    irrCB->mType = CNullShader::GetShaderType(
        E_SHADER_VARIABLE_TYPE::ESVT_UINT8,
        size,
        0,
        0,
        1,
        0,
        0,
        0,
        size);

    return irrCB.GetPtr();
}

VulkanDescriptorLayout* irr::video::CVulkanGLSLProgram::getLayout()
{
    if (!mLayout)
    {
        mBindings.reserve(u32(mBuffers.size()) + u32(mTextures.size()));

        for (int i = 0; i < mBuffers.size(); ++i)
        {
            const auto& buf = mBuffers[i];
            mBindings.push_back({});
            VkDescriptorSetLayoutBinding& bind = mBindings.back();
            bind.binding = buf->getBindingIndex();
            bind.stageFlags = GetShaderStageBit(static_cast<SConstantBuffer*>(buf->getRootBuffer())->mShaderStage);
            bind.descriptorType = buf->mBufferType == ESVT_CONSTANT ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bind.descriptorCount = 1;
            bind.pImmutableSamplers = nullptr;
        }

        for (int i = 0; i < mTextures.size(); ++i)
        {
            SShaderSampler* buf = static_cast<SShaderSampler*>(mTextures[i].GetPtr());
            if (buf->getType()->Type != E_SHADER_VARIABLE_TYPE::ESVT_SAMPLER)
                continue;

            mBindings.push_back({});
            VkDescriptorSetLayoutBinding& bind = mBindings.back();
            bind.binding = buf->mBindPoint;
            bind.stageFlags = GetShaderStageBit(buf->mShaderStage);
            bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bind.descriptorCount = 1;
            bind.pImmutableSamplers = nullptr;
        }

        mLayout = new VulkanDescriptorLayout(*Driver->_getPrimaryDevice(), mBindings.data(), mBindings.size());
    }

    return mLayout;
}

VulkanGpuParams * irr::video::CVulkanGLSLProgram::GetDefaultGpuParams()
{
    return mParams;
}

void irr::video::CVulkanGLSLProgram::OnDeviceLost(CVulkanDriver * device)
{
}

void irr::video::CVulkanGLSLProgram::OnDeviceRestored(CVulkanDriver * device)
{
}

//VulkanShaderGenericValuesBuffer::VulkanShaderGenericValuesBuffer(video::IShaderDataBuffer::E_UPDATE_TYPE updateType) : video::IShaderDataBuffer(updateType)
//{
//    world = nullptr;
//    view = nullptr;
//    projection = nullptr;
//    nTexture = nullptr;
//    AlphaTest = nullptr;
//    LightCount = nullptr;
//    AlphaRef = nullptr;
//    ShaderMaterial = nullptr;
//    ShaderLight = nullptr;
//}
//
//VulkanShaderGenericValuesBuffer::~VulkanShaderGenericValuesBuffer()
//{
//}
//
//void VulkanShaderGenericValuesBuffer::InitializeFormShader(video::IShader * gpuProgram, void * Descriptor)
//{
//#define UNSAFE_ADD_DATABUFFER_ELEMENT(var, type, name) \
//    var = new video::ShaderDataBufferElementObject<type>(gpuProgram->GetGPUVariableDesc(name)); \
//    AddBufferElement(var);
//    UNSAFE_ADD_DATABUFFER_ELEMENT(world, core::matrix4, "worldMatrix");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(view, core::matrix4, "viewMatrix");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(projection, core::matrix4, "projectionMatrix");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(textureMatrix1, core::matrix4, "textureMatrix1");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(textureMatrix2, core::matrix4, "textureMatrix2");
//
//    UNSAFE_ADD_DATABUFFER_ELEMENT(nTexture, int, "g_nTexture");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(AlphaTest, int, "g_bAlphaTest");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(LightCount, int, "g_iLightCount");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(AlphaRef, float, "g_fAlphaRef");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(fogParams, video::SColorf, "g_fogParams");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(fogColor, video::SColorf, "g_fogColor");
//    UNSAFE_ADD_DATABUFFER_ELEMENT(eyePositionVert, core::vector3df, "g_eyePositionVert");
//
//    UNSAFE_ADD_DATABUFFER_ELEMENT(ShaderMaterial, VulkanShaderGenericValuesBuffer::Material, "g_material");
//
//    gpuProgram->getVideoDriver()->setShaderMapping(projection->getDescription(), gpuProgram, irr::scene::E_HARDWARE_MAPPING::EHM_DYNAMIC);
//    gpuProgram->getVideoDriver()->setShaderMapping(ShaderMaterial->getDescription(), gpuProgram, irr::scene::E_HARDWARE_MAPPING::EHM_DYNAMIC);
//
//    ShaderLight = new video::ShaderDataBufferElementArray<VulkanShaderGenericValuesBuffer::Light>(gpuProgram->GetGPUVariableDesc("g_lights"));
//    AddBufferElement(ShaderLight);
//#undef UNSAFE_ADD_DATABUFFER_ELEMENT
//}
//
//void VulkanShaderGenericValuesBuffer::UpdateBuffer(video::IShader * gpuProgram, scene::IMeshBuffer * meshBuffer, scene::IMesh * mesh, scene::ISceneNode * node, u32 updateFlags)
//{
//    CVulkanDriver* Driver = static_cast<CVulkanDriver*>(gpuProgram->getVideoDriver());
//    SMaterial& CurrentMaterial = Driver->GetMaterial();// !meshBuffer ? Driver->GetMaterial() : meshBuffer->getMaterial();
//    //video::IMaterialRenderer* rnd = Driver->getMaterialRenderer(CurrentMaterial.MaterialType);
//
//    VulkanShaderGenericValuesBuffer::Material material;
//    memset(&material, 0, sizeof(VulkanShaderGenericValuesBuffer::Material));
//
//    *world = Driver->getTransform(ETS_WORLD);
//    *view = Driver->getTransform(ETS_VIEW);
//    *projection = Driver->getTransform(ETS_PROJECTION);
//
//    *AlphaRef = CurrentMaterial.MaterialType != EMT_ONETEXTURE_BLEND ? CurrentMaterial.MaterialType == ::EMT_TRANSPARENT_ALPHA_CHANNEL_REF ? CurrentMaterial.MaterialTypeParam : 0.5f : 0.0f;
//    *AlphaTest = CurrentMaterial.isTransparent(); // (rnd && rnd->isTransparent());
//    int _nTexture = 0;
//
//    *textureMatrix1 = CurrentMaterial.TextureLayer[0].getTextureMatrixConst();
//    if (CurrentMaterial.MaterialType == E_MATERIAL_TYPE::EMT_REFLECTION_2_LAYER || CurrentMaterial.MaterialType == E_MATERIAL_TYPE::EMT_TRANSPARENT_REFLECTION_2_LAYER || CurrentMaterial.MaterialType == E_MATERIAL_TYPE::EMT_SPHERE_MAP)
//        *textureMatrix2 = *(irr::core::matrix4*)&VK_SphereMapMatrix;
//    else
//        *textureMatrix2 = CurrentMaterial.TextureLayer[1].getTextureMatrixConst();
//
//    for (u8 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
//        if (Driver->getCurrentTexture(i) != nullptr)
//            ++_nTexture;
//
//    *nTexture = _nTexture;
//
//    int n = CurrentMaterial.Lighting ? Driver->getDynamicLightCount() : 0;
//    *LightCount = n;
//
//    auto inv = Driver->getTransform(ETS_VIEW);
//    if (Driver->GetCurrentRenderMode() == CVulkanDriver::E_RENDER_MODE::ERM_3D)
//    {
//        if (viewMatrixTranslationCache != inv.getTranslation())
//        {
//            viewMatrixTranslationCache = inv.getTranslation();
//            inv.makeInverse();
//            eyePositionVert->setShaderValues(inv.getTranslation());
//        }
//
//        if (n)
//        {
//            ShaderLight->ResetShaderValues();
//            for (int i = 0; i < n; i++)
//            {
//                VulkanShaderGenericValuesBuffer::Light l;
//                memset(&l, 0, sizeof(VulkanShaderGenericValuesBuffer::Light));
//                SLight dl = Driver->getDynamicLight(i);
//                memcpy(l.Position, &dl.Position.X, sizeof(float) * 3);
//                memcpy(l.Atten, &dl.Attenuation.X, sizeof(float) * 3);
//                l.Ambient = dl.AmbientColor;
//                l.Diffuse = dl.DiffuseColor;
//                l.Specular = dl.SpecularColor;
//                l.Range = dl.Radius;
//                l.Falloff = dl.Falloff;
//                l.Theta = dl.InnerCone * 2.0f * core::DEGTORAD;
//                l.Phi = dl.OuterCone * 2.0f * core::DEGTORAD;
//                l.Type = dl.Type;
//                *ShaderLight += l;
//            }
//        }
//
//        if (fogParams)
//        {
//            irr::video::SColor color;
//            irr::video::E_FOG_TYPE fogType;
//            f32 start;
//            f32 end;
//            f32 density;
//            bool pixelFog;
//            bool rangeFog;
//
//            gpuProgram->getVideoDriver()->getFog(color, fogType, start, end, density, pixelFog, rangeFog);
//
//            auto const& value = fogParams->m_values;
//            if (value.r != start || value.g != end || value.b != density || value.a != fogType)
//                *fogParams = video::SColorf(start, end, density, fogType);
//
//            if (fogColor)
//                *fogColor = video::SColorf(color);
//        }
//
//        material.Ambient.set(
//            Driver->GetAmbientLight().getAlpha() * (float)CurrentMaterial.AmbientColor.getAlpha() / 255.f
//            , Driver->GetAmbientLight().getRed() * (float)CurrentMaterial.AmbientColor.getRed() / 255.f
//            , Driver->GetAmbientLight().getGreen() * (float)CurrentMaterial.AmbientColor.getGreen() / 255.f
//            , Driver->GetAmbientLight().getBlue() * (float)CurrentMaterial.AmbientColor.getBlue() / 255.f);
//    }
//    else
//    {
//        material.Ambient.set(
//            (float)CurrentMaterial.AmbientColor.getAlpha() / 255.f
//            , (float)CurrentMaterial.AmbientColor.getRed() / 255.f
//            , (float)CurrentMaterial.AmbientColor.getGreen() / 255.f
//            , (float)CurrentMaterial.AmbientColor.getBlue() / 255.f);
//    }
//
//    material.Diffuse.set((float)CurrentMaterial.DiffuseColor.getAlpha() / 255.f
//        , (float)CurrentMaterial.DiffuseColor.getRed() / 255.f
//        , (float)CurrentMaterial.DiffuseColor.getGreen() / 255.f
//        , (float)CurrentMaterial.DiffuseColor.getBlue() / 255.f);
//
//    material.Emissive.set((float)CurrentMaterial.EmissiveColor.getAlpha() / 255.f
//        , (float)CurrentMaterial.EmissiveColor.getRed() / 255.f
//        , (float)CurrentMaterial.EmissiveColor.getGreen() / 255.f
//        , (float)CurrentMaterial.EmissiveColor.getBlue() / 255.f);
//
//    material.Shininess = CurrentMaterial.Shininess;
//    material.Type = CurrentMaterial.MaterialType;
//    material.Lighted = Driver->GetCurrentRenderMode() == CVulkanDriver::E_RENDER_MODE::ERM_3D && CurrentMaterial.Lighting;
//    material.Fogged = Driver->GetCurrentRenderMode() == CVulkanDriver::E_RENDER_MODE::ERM_3D && CurrentMaterial.FogEnable;
//
//    *ShaderMaterial = material;
//}
