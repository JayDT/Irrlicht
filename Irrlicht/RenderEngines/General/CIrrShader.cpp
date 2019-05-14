#include "CIrrShader.h"
#include "CIrrVertexDeclaration.h"
#include "IHardwareBuffer.h"
#include "IShaderConstantSetCallBack.h"
#include "CNullDriver.h"

using namespace irr;
using namespace irr::video;

core::stringc SShaderSampler::mSemantic;

const char* gShaderTypeNames[E_SHADER_VARIABLE_TYPE::ESVT_MAX]
{
    "VOID",
    "BOOL",
    "INT",
    "UINT",
    "INT8",
    "UINT8",
    "INT16",
    "UINT16",
    "INT64",
    "UINT64",
    "FLOAT",
    "DOUBLE",
    "TEXTURE",
    "SAMPLER",
    "BUFFER",
    "STRUCT"
};

core::matrix4 irr_SphereMapMatrix = core::matrix4(0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f);

std::list<SShaderType> gShaderTypeSingletonArray;

SShaderType * irr::video::CNullShader::GetShaderType(E_SHADER_VARIABLE_TYPE Type, u32 Elements, u32 Members, u32 Rows, u32 Columns, u32 PackedSize, u32 UnpackedSize, u32 ArrayStride, u32 Stride, bool UseCache)
{
    if (UseCache)
    {
        for (SShaderType& type : gShaderTypeSingletonArray)
        {
            if (type.Type == Type
                && type.Elements == Elements
                && type.Members == Members
                && type.Rows == Rows
                && type.Columns == Columns
                && type.PackedSize == PackedSize
                && type.UnpackedSize == UnpackedSize
                && type.ArrayStride == ArrayStride
                && type.Stride == Stride)
                return &type;
        }
    }

    gShaderTypeSingletonArray.push_back({});
    SShaderType* type = &gShaderTypeSingletonArray.back();
    type->TypeName = gShaderTypeNames[Type];
    type->Type = Type;
    type->Elements = Elements;
    type->Members = Members;
    type->Rows = Rows;
    type->Columns = Columns;
    type->PackedSize = PackedSize;
    type->UnpackedSize = UnpackedSize;
    type->ArrayStride = ArrayStride;
    type->Stride = Stride;
    return type;
}

bool irr::video::SShaderVariableScalar::isValid()
{
    return mIsValid;
}

bool irr::video::SShaderVariableScalar::isDirty()
{
    return mIsDirty;
}

IConstantBuffer * irr::video::SShaderVariableScalar::getRootBuffer()
{
    return mBuffer;
}

IShaderVariable * irr::video::SShaderVariableScalar::getParentVariable()
{
    return mParent;
}

IShaderScalarVariable * irr::video::SShaderVariableScalar::AsScalar()
{
    return this;
}

IShaderMatrixVariable * irr::video::SShaderVariableScalar::AsMatrix()
{
    return nullptr;
}

void irr::video::SShaderVariableScalar::setRawValue(const u8 * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (offset + count));
    mBuffer->setRawValue(values, mOffset + offset, count);
}

u8 * irr::video::SShaderVariableScalar::getRawValue(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return mBuffer->getRawValue(mOffset + offset, count);
}

void irr::video::SShaderVariableScalar::setFloat(const float value)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= sizeof(float));
    //mBuffer->setRawValue((u8*)&value, mOffset, sizeof(float));
    mBuffer->write4bAlign((u8*)&value, mOffset, sizeof(float));
}

float irr::video::SShaderVariableScalar::getFloat()
{
    if (!mBuffer)
        return 0.0f;

    return *(float*)mBuffer->getRawValue(mOffset, sizeof(float));
}

void irr::video::SShaderVariableScalar::setFloatArray(const float * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (sizeof(float) * offset + sizeof(float) * count));
    //mBuffer->setRawValue((u8*)values, mOffset + (sizeof(float) * offset), sizeof(float) * count);
    mBuffer->write4bAlign((u8*)values, mOffset + (sizeof(float) * offset), sizeof(float) * count);
}

float * irr::video::SShaderVariableScalar::getFloatArray(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return (float*)mBuffer->getRawValue(mOffset + (sizeof(float) * offset), sizeof(float) * count);
}

void irr::video::SShaderVariableScalar::setInt(const s32 value)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= sizeof(float));
    //mBuffer->setRawValue((u8*)&value, mOffset, sizeof(s32));
    mBuffer->write4bAlign((u8*)&value, mOffset, sizeof(s32));
}

s32 irr::video::SShaderVariableScalar::getInt()
{
    if (!mBuffer)
        return 0;

    return *(float*)mBuffer->getRawValue(mOffset, sizeof(s32));
}

void irr::video::SShaderVariableScalar::setIntArray(const s32 * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (sizeof(s32) * offset + sizeof(s32) * count));
    //mBuffer->setRawValue((u8*)values, mOffset + (sizeof(s32) * offset), sizeof(s32) * count);
    mBuffer->write4bAlign((u8*)values, mOffset + (sizeof(s32) * offset), sizeof(s32) * count);
}

s32 * irr::video::SShaderVariableScalar::getIntArray(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return (s32*)mBuffer->getRawValue(mOffset + (sizeof(s32) * offset), sizeof(s32) * count);
}

void irr::video::SShaderVariableScalar::setBool(const bool value)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= sizeof(bool));
    mBuffer->setRawValue((u8*)&value, mOffset, sizeof(bool));
}

bool irr::video::SShaderVariableScalar::getBool()
{
    if (!mBuffer)
        return false;

    return *(bool*)mBuffer->getRawValue(mOffset, sizeof(bool));
}

void irr::video::SShaderVariableScalar::setBoolArray(const bool * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (sizeof(bool) * offset + sizeof(bool) * count));
    mBuffer->setRawValue((u8*)values, mOffset + (sizeof(bool) * offset), sizeof(bool) * count);
}

bool * irr::video::SShaderVariableScalar::getBoolArray(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return (bool*)mBuffer->getRawValue(mOffset + (sizeof(bool) * offset), sizeof(bool) * count);
}

bool irr::video::SShaderVariableMatrix::isValid()
{
    return mIsValid;
}

bool irr::video::SShaderVariableMatrix::isDirty()
{
    return mIsDirty;
}

IConstantBuffer * irr::video::SShaderVariableMatrix::getRootBuffer()
{
    return mBuffer;
}

IShaderVariable * irr::video::SShaderVariableMatrix::getParentVariable()
{
    return mParent;
}

IShaderScalarVariable * irr::video::SShaderVariableMatrix::AsScalar()
{
    return nullptr;
}

IShaderMatrixVariable * irr::video::SShaderVariableMatrix::AsMatrix()
{
    return this;
}

void irr::video::SShaderVariableMatrix::setRawValue(const u8 * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (offset + count));
    mBuffer->setRawValue(values, mOffset + offset, count);
}

u8 * irr::video::SShaderVariableMatrix::getRawValue(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return mBuffer->getRawValue(mOffset + offset, count);
}

void irr::video::SShaderVariableMatrix::setMatrix(const core::matrix4 & value)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= sizeof(core::matrix4));
    mBuffer->setRawValue((u8*)&value, mOffset, sizeof(core::matrix4));
    //mBuffer->write8bAlign((u8*)&value, mOffset, sizeof(core::matrix4));
}

core::matrix4 & irr::video::SShaderVariableMatrix::getMatrix()
{
    if (!mBuffer)
        return (core::matrix4 &)core::IdentityMatrix;

    return *(core::matrix4*)mBuffer->getRawValue(mOffset, sizeof(core::matrix4));
}

void irr::video::SShaderVariableMatrix::setMatrixArray(const core::matrix4 * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (sizeof(core::matrix4) * offset + sizeof(core::matrix4) * count));
    mBuffer->setRawValue((u8*)values, mOffset + (sizeof(core::matrix4) * offset), sizeof(core::matrix4) * count);
    //mBuffer->write8bAlign((u8*)values, mOffset + (sizeof(core::matrix4) * offset), sizeof(core::matrix4) * count);
}

core::matrix4 * irr::video::SShaderVariableMatrix::getMatrixArray(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return (core::matrix4*)mBuffer->getRawValue(mOffset + (sizeof(core::matrix4) * offset), sizeof(core::matrix4) * count);
}

bool irr::video::SShaderVariableStruct::isValid()
{
    return mIsValid;
}

bool irr::video::SShaderVariableStruct::isDirty()
{
    return mIsDirty;
}

IConstantBuffer * irr::video::SShaderVariableStruct::getRootBuffer()
{
    return mBuffer;
}

IShaderVariable * irr::video::SShaderVariableStruct::getParentVariable()
{
    return mParent;
}

IShaderScalarVariable * irr::video::SShaderVariableStruct::AsScalar()
{
    return nullptr;
}

IShaderMatrixVariable * irr::video::SShaderVariableStruct::AsMatrix()
{
    return nullptr;
}

IShaderVariable * irr::video::SShaderVariableStruct::getVariableByIndex(u32 index)
{
    for (IShaderVariable* var : mVariables)
        if (var->_getIndex() == index)
            return var;
    return nullptr;
}

IShaderVariable * irr::video::SShaderVariableStruct::getVariableByName(const char * name)
{
    for (IShaderVariable* var : mVariables)
        if (var->GetName() == name)
            return var;
    return nullptr;
}

IShaderVariable * irr::video::SShaderVariableStruct::getVariableBySemantic(const char * semantic)
{
    return nullptr;
}

void irr::video::SShaderVariableStruct::setRawValue(const u8 * values, u32 offset, u32 count)
{
    if (!mBuffer)
        return;

    assert(mType->Stride >= (offset + count));
    mBuffer->setRawValue(values, mOffset + offset, count);
}

u8 * irr::video::SShaderVariableStruct::getRawValue(u32 offset, u32 count)
{
    if (!mBuffer)
        return nullptr;

    return mBuffer->getRawValue(mOffset + offset, count);
}


bool irr::video::SShaderSampler::isValid()
{
    return mIsValid;
}

bool irr::video::SShaderSampler::isDirty()
{
    return false;
}

IConstantBuffer* irr::video::SShaderSampler::getRootBuffer()
{
    return mBuffer;
}

IShaderVariable* irr::video::SShaderSampler::getParentVariable()
{
    return mParent;
}

IShaderScalarVariable* irr::video::SShaderSampler::AsScalar()
{
    return nullptr;
}

IShaderMatrixVariable* irr::video::SShaderSampler::AsMatrix()
{
    return nullptr;
}

irr::video::CNullShader::CNullShader(video::IVideoDriver * context, E_SHADER_LANG type)
    : mContext(context)
    , mShaderLang(type)
{
}

irr::video::CNullShader::~CNullShader()
{
}

void irr::video::CNullShader::Reset()
{
    mBuffers.clear();
}

void irr::video::CNullShader::AddConstantBuffer(SConstantBuffer * buffer)
{
    assert(buffer->mShader == this);
    mBuffers.push_back(irr::Ptr<SConstantBuffer>(buffer));
}

void irr::video::CNullShader::AddShaderResource(IShaderVariable * var)
{
    //mTextures.push_back(var);
}

IConstantBuffer* irr::video::CNullShader::getConstantBufferByIndex(u32 index)
{
    for (int i = 0; i != mBuffers.size(); ++i)
    {
        IConstantBuffer* var = mBuffers[i];
        if (var->_getIndex() == index)
            return var;
    }

    return nullptr;
}

IConstantBuffer* irr::video::CNullShader::getConstantBufferByName(const char* name)
{
    for (int i = 0; i != mBuffers.size(); ++i)
    {
        IConstantBuffer* var = mBuffers[i];
        if (var->GetName() == name)
            return var;
    }

    return nullptr;
}

IShaderVariable * irr::video::CNullShader::getVariableByIndex(u32 index)
{
    for (int i = 0; i != mVertexInput.size(); ++i)
    {
        IShaderVariable* var = mVertexInput[i];
        if (var->_getIndex() == index)
            return var;
    }

    for (int i = 0; i != mTextures.size(); ++i)
    {
        IShaderVariable* var = mTextures[i];
        if (var->_getIndex() == index)
            return var;
    }

    return getConstantBufferByIndex(index);
}

IShaderVariable * irr::video::CNullShader::getVariableByName(const char * name)
{
    for (int i = 0; i != mVertexInput.size(); ++i)
    {
        IShaderVariable* var = mVertexInput[i];
        if (var->GetName() == name)
            return var;
    }

    for (int i = 0; i != mTextures.size(); ++i)
    {
        IShaderVariable* var = mTextures[i];
        if (var->GetName() == name)
            return var;
    }

    return getConstantBufferByName(name);
}

IShaderVariable * irr::video::CNullShader::getVariableBySemantic(const char * semantic)
{
    for (int i = 0; i != mVertexInput.size(); ++i)
    {
        IShaderVariable* var = mVertexInput[i];
        if (var->GetSemantic() == semantic)
            return var;
    }
    return nullptr;
}

irr::video::SConstantBuffer::SConstantBuffer(IShader* shader, E_SHADER_TYPES stage)
    : mShader(shader)
    , mHwObject(nullptr)
    , mType(nullptr)
    , mCallBack(nullptr)
    , mBackStore(nullptr)
    , mSize(0)
    , mBufferType()
    , mShaderStage(stage)
    , mBindPoint(-1)
    , mChangedID(1)
    , mMapping()
    , mRowMajor(false)
    , mLayout16ByteAlign(true)      // Except Legacy OpenGL
{
}

irr::video::SConstantBuffer::~SConstantBuffer()
{
    Invalidate();
}

void irr::video::SConstantBuffer::Invalidate()
{
    for (u32 i = 0; i != mVariables.size(); ++i)
        delete mVariables[i];
    mVariables.clear();
}

void irr::video::SConstantBuffer::AddVariable(IShaderVariable * variable)
{
    assert(variable->getRootBuffer() != this);
    mVariables.push_back(variable);
}

void irr::video::SConstantBuffer::clear()
{
}

void irr::video::SConstantBuffer::setShaderDataCallback(const IShaderConstantSetCallBack * cb)
{
    mCallBack = irr::Ptr<IShaderConstantSetCallBack>(const_cast<IShaderConstantSetCallBack *>(cb));
}

IShaderVariable * irr::video::SConstantBuffer::getVariableByIndex(u32 index)
{
    for (IShaderVariable* var : mVariables)
        if (var->_getIndex() == index)
            return var;
    return nullptr;
}

IShaderVariable * irr::video::SConstantBuffer::getVariableByName(const char * name)
{
    for (IShaderVariable* var : mVariables)
        if (var->GetName() == name)
            return var;
    return nullptr;
}

IShaderVariable * irr::video::SConstantBuffer::getVariableBySemantic(const char * semantic)
{
    return nullptr;
}

bool irr::video::SConstantBuffer::setHardwareMappingHint(scene::E_HARDWARE_MAPPING constantMapping)
{
    mMapping = constantMapping;
    return true;
}

scene::E_HARDWARE_MAPPING irr::video::SConstantBuffer::getHardwareMappingHint()
{
    return mMapping;
}

u32 irr::video::SConstantBuffer::getBindingIndex() const
{
    return mBindPoint;
}

SShaderType * irr::video::SConstantBuffer::getType()
{
    return mType;
}

bool irr::video::SConstantBuffer::isValid()
{
    return true;
}

bool irr::video::SConstantBuffer::isChanged() const
{
    return mChangedID != mHwObject->getChangeID(E_HARDWARE_BUFFER_TYPE::EHBT_CONSTANTS);
}

bool irr::video::SConstantBuffer::isDirty()
{
    return isChanged();
}

const core::stringc& irr::video::SConstantBuffer::GetName()
{
    return mName;
}

IConstantBuffer* irr::video::SConstantBuffer::getRootBuffer()
{
    return this;
}
IShaderVariable* irr::video::SConstantBuffer::getParentVariable()
{
    return nullptr;
}

IShaderScalarVariable* irr::video::SConstantBuffer::AsScalar()
{
    return nullptr;
}

IShaderMatrixVariable* irr::video::SConstantBuffer::AsMatrix()
{
    return nullptr;
}

void irr::video::SConstantBuffer::setRawValue(const u8* values, u32 offset, u32 count)
{
    assert(mType->Stride >= (offset + count));

    if (std::memcmp(mHostMemory.data() + offset, values, count))
    {
        std::memcpy(mHostMemory.data() + offset, values, count);
        setDirty();
    }
}

u8* irr::video::SConstantBuffer::getRawValue(u32 offset, u32 count)
{
    assert(mType->Stride >= (offset + count));

    return mHostMemory.data() + offset;
}

void irr::video::SConstantBuffer::write4bAlign(const u8 * values, u32 offset, u32 count)
{
    assert(mType->Stride >= (offset + count) && (count % sizeof(u32)) == 0);

    u32* hostMemory = (u32*)(mHostMemory.data() + offset);
    u32* endHostMemory = (u32*)(mHostMemory.data() + offset + count);
    u32* dataPtr = (u32*)values;

    if (count == 4)
    {
        if (*hostMemory != *dataPtr)
        {
            *hostMemory = *dataPtr;
            setDirty();
        }
        return;
    }

    bool changed = false;
    while (hostMemory != endHostMemory)
    {
        if (*hostMemory != *dataPtr)
        {
            *hostMemory = *dataPtr;
            if (!changed)
                changed = true;
        }

        ++hostMemory;
        ++dataPtr;
    }

    if (changed)
        setDirty();
}

void irr::video::SConstantBuffer::write8bAlign(const u8* values, u32 offset, u32 count)
{
    assert(mType->Stride >= (offset + count) && (count % sizeof(u64)) == 0);

    u64* hostMemory = (u64*)(mHostMemory.data() + offset);
    u64* endHostMemory = (u64*)(mHostMemory.data() + offset + count);
    u64* dataPtr = (u64*)values;

    if (count == 8)
    {
        if (*hostMemory != *dataPtr)
        {
            *hostMemory = *dataPtr;
            setDirty();
        }
        return;
    }

    bool changed = false;
    while (hostMemory != endHostMemory)
    {
        if (*hostMemory != *dataPtr)
        {
            *hostMemory = *dataPtr;
            if (!changed)
                changed = true;
        }

        ++hostMemory;
        ++dataPtr;
    }

    if (changed)
        setDirty();
}

u32 irr::video::SConstantBuffer::_getOffset()
{
    return mOffset;
}

void irr::video::IrrDefaultShaderVertexCallBack::OnPrepare(irr::video::IConstantBuffer* buffer)
{
    mWorldMatrix = buffer->getVariableByName("worldMatrix")->AsMatrix();
    mViewMatrix = buffer->getVariableByName("viewMatrix")->AsMatrix();
    mProjectionMatrix = buffer->getVariableByName("projectionMatrix")->AsMatrix();
    mTextureMatrix1 = buffer->getVariableByName("textureMatrix1")->AsMatrix();
    mTextureMatrix2 = buffer->getVariableByName("textureMatrix2")->AsMatrix();

    buffer->setHardwareMappingHint(scene::E_HARDWARE_MAPPING::EHM_DYNAMIC);
}

void irr::video::IrrDefaultShaderVertexCallBack::OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
    video::IVideoDriver* driver = buffer->getShader()->getVideoDriver();

    mWorldMatrix->setMatrix(driver->getTransform(ETS_WORLD));
    mViewMatrix->setMatrix(driver->getTransform(ETS_VIEW));
    mProjectionMatrix->setMatrix(driver->getTransform(ETS_PROJECTION));

    if (mMaterial)
    {
        mTextureMatrix1->setMatrix(mMaterial->TextureLayer[0].getTextureMatrixConst());

        if (mMaterial->MaterialType == E_MATERIAL_TYPE::EMT_REFLECTION_2_LAYER || mMaterial->MaterialType == E_MATERIAL_TYPE::EMT_TRANSPARENT_REFLECTION_2_LAYER || mMaterial->MaterialType == E_MATERIAL_TYPE::EMT_SPHERE_MAP)
            mTextureMatrix2->setMatrix(irr_SphereMapMatrix);
        else
            mTextureMatrix2->setMatrix(mMaterial->TextureLayer[1].getTextureMatrixConst());

        mMaterial = nullptr;
    }
}

void irr::video::IrrDefaultShaderFragmentCallBack::OnPrepare(irr::video::IConstantBuffer* buffer)
{
    mTexture = buffer->getVariableByName("g_nTexture")->AsScalar();
    mAlphaTest = buffer->getVariableByName("g_bAlphaTest")->AsScalar();
    mLightCount = buffer->getVariableByName("g_iLightCount")->AsScalar();
    mFAlphaRef = buffer->getVariableByName("g_fAlphaRef")->AsScalar();
    mFogColor = buffer->getVariableByName("g_fogColor")->AsScalar();
    mFogParams = buffer->getVariableByName("g_fogParams")->AsScalar();
    mEyePositionVert = buffer->getVariableByName("g_eyePositionVert")->AsScalar();

    mMaterialVar = buffer->getVariableByName("g_material");
    mLightVar = buffer->getVariableByName("g_lights");

    _IRR_DEBUG_BREAK_IF(mLightVar->getType()->ArrayStride != sizeof(IrrDefaultShaderFragmentCallBack::Light));
    _IRR_DEBUG_BREAK_IF(mMaterialVar->getType()->Stride != sizeof(IrrDefaultShaderFragmentCallBack::Material));

    buffer->setHardwareMappingHint(scene::E_HARDWARE_MAPPING::EHM_DYNAMIC);
}

void irr::video::IrrDefaultShaderFragmentCallBack::OnSetConstants(irr::video::IConstantBuffer* buffer, irr::scene::IMeshBuffer* meshBuffer, scene::IMesh* mesh/* = nullptr*/, scene::ISceneNode* node/* = nullptr*/)
{
    video::IVideoDriver* driver = buffer->getShader()->getVideoDriver();

    if (mMaterial)
    {
        IrrDefaultShaderFragmentCallBack::Material pMaterial;
        const SMaterial& CurrentMaterial = *mMaterial;

        mTexture->setInt(mMaterial->getTexture(0) != nullptr ? 1 : 0);
        mFAlphaRef->setFloat(CurrentMaterial.MaterialType != EMT_ONETEXTURE_BLEND ? CurrentMaterial.MaterialType == ::EMT_TRANSPARENT_ALPHA_CHANNEL_REF ? CurrentMaterial.MaterialTypeParam : 0.5f : 0.0f);
        mAlphaTest->setInt(CurrentMaterial.isTransparent() ? 1 : 0);
        
        int n = CurrentMaterial.Lighting ? driver->getDynamicLightCount() : 0;
        mLightCount->setInt(n);
        
        auto inv = driver->getTransform(ETS_VIEW);
        if (driver->GetCurrentRenderMode() == E_RENDER_MODE::ERM_3D)
        {
            if (viewMatrixTranslationCache != inv.getTranslation())
            {
                viewMatrixTranslationCache = inv.getTranslation();
                inv.makeInverse();
                core::vector3df _tmp = inv.getTranslation();
                mEyePositionVert->setFloatArray(&_tmp.X, 0, 3);
            }
        
            if (n)
            {
                for (int i = 0; i < n; i++)
                {
                    IrrDefaultShaderFragmentCallBack::Light l;
                    SLight dl = driver->getDynamicLight(i);
                    std::memcpy(l.Position, &dl.Position.X, sizeof(float) * 3);
                    std::memcpy(l.Atten, &dl.Attenuation.X, sizeof(float) * 3);
                    l.Ambient = dl.AmbientColor;
                    l.Diffuse = dl.DiffuseColor;
                    l.Specular = dl.SpecularColor;
                    l.Range = dl.Radius;
                    l.Falloff = dl.Falloff;
                    l.Theta = dl.InnerCone * 2.0f * core::DEGTORAD;
                    l.Phi = dl.OuterCone * 2.0f * core::DEGTORAD;
                    l.Type = dl.Type;
                    mLightVar->setRawValue((u8*)&l, 0, sizeof(IrrDefaultShaderFragmentCallBack::Light));
                }
            }
        
            irr::video::SColor color;
            irr::video::E_FOG_TYPE fogType;
            f32 start;
            f32 end;
            f32 density;
            bool pixelFog;
            bool rangeFog;
        
            driver->getFog(color, fogType, start, end, density, pixelFog, rangeFog);
        
            float* value = mFogParams->getFloatArray(0, 4);
            if (value[0] != start || value[1] != end || value[2] != density || value[3] != fogType)
            {
                value[0] = start;
                value[1] = end;
                value[2] = density;
                value[3] = fogType;
                buffer->setDirty();
            }
        
            video::SColorf fogColor(color);
            mFogColor->setFloatArray(&fogColor.r, 0, 4);
        
            pMaterial.Ambient.set(
                driver->GetAmbientLight().getAlpha() * (float)CurrentMaterial.AmbientColor.getAlpha() / 255.f
                , driver->GetAmbientLight().getRed() * (float)CurrentMaterial.AmbientColor.getRed() / 255.f
                , driver->GetAmbientLight().getGreen() * (float)CurrentMaterial.AmbientColor.getGreen() / 255.f
                , driver->GetAmbientLight().getBlue() * (float)CurrentMaterial.AmbientColor.getBlue() / 255.f);
        }
        else
        {
            pMaterial.Ambient.set(
                (float)CurrentMaterial.AmbientColor.getAlpha() / 255.f
                , (float)CurrentMaterial.AmbientColor.getRed() / 255.f
                , (float)CurrentMaterial.AmbientColor.getGreen() / 255.f
                , (float)CurrentMaterial.AmbientColor.getBlue() / 255.f);
        }
        
        pMaterial.Diffuse.set((float)CurrentMaterial.DiffuseColor.getAlpha() / 255.f
            , (float)CurrentMaterial.DiffuseColor.getRed() / 255.f
            , (float)CurrentMaterial.DiffuseColor.getGreen() / 255.f
            , (float)CurrentMaterial.DiffuseColor.getBlue() / 255.f);
        
        pMaterial.Emissive.set((float)CurrentMaterial.EmissiveColor.getAlpha() / 255.f
            , (float)CurrentMaterial.EmissiveColor.getRed() / 255.f
            , (float)CurrentMaterial.EmissiveColor.getGreen() / 255.f
            , (float)CurrentMaterial.EmissiveColor.getBlue() / 255.f);
        
        pMaterial.Shininess = CurrentMaterial.Shininess;
        pMaterial.Type = CurrentMaterial.MaterialType;
        pMaterial.Lighted = driver->GetCurrentRenderMode() == E_RENDER_MODE::ERM_3D && CurrentMaterial.Lighting;
        pMaterial.Fogged = driver->GetCurrentRenderMode() == E_RENDER_MODE::ERM_3D && CurrentMaterial.FogEnable;
        
        mMaterialVar->setRawValue((u8*)&pMaterial, 0, sizeof(IrrDefaultShaderFragmentCallBack::Material));

        mMaterial = nullptr;
    }
}