#include "CDynamicVertexBufferObject.h"

#include "CNullDriver.h"

using namespace irr;
using namespace irr::scene;

CDynamicVertexBufferObject::CDynamicVertexBufferObject()
    : m_isEnabled(true)
    , VertexBuffer(nullptr)
    , IndexBuffer(nullptr)
    , StreamBuffer(nullptr)
    , m_gpuProgram(nullptr)
    , Material(new irr::video::SMaterial)
    , ActiveSubBuffer(0)
    , m_needBoundRecalulate(true)
{
}

CDynamicVertexBufferObject::~CDynamicVertexBufferObject()
{
    if (SubBuffer.empty())
    {
        if (Material)
            delete Material;
    }

    for (u32 i = 0; i != SubBuffer.size(); ++i)
        delete SubBuffer[i];

    //for (u32 i = 0; i != mShaderConstantBuffers.size(); ++i)
    //    if (mShaderConstantBuffers[i])
    //        delete mShaderConstantBuffers[i];
}

void irr::scene::CDynamicVertexBufferObject::setVertexBuffer(irr::scene::IVertexBuffer* newVertexBuffer)
{
    VertexBuffer.Reset(newVertexBuffer);
}

void irr::scene::CDynamicVertexBufferObject::setIndexBuffer(irr::scene::IIndexBuffer* newIndexBuffer)
{
    IndexBuffer.Reset(newIndexBuffer);
}

void irr::scene::CDynamicVertexBufferObject::setStreamBuffer(irr::scene::IStreamBuffer* newStreamBuffer)
{
    StreamBuffer.Reset(newStreamBuffer);
}

void CDynamicVertexSubBuffer::recalculateBoundingBox(CDynamicVertexBufferObject* buffer)
{
    //! Recalculate bounding box
    if (!buffer->getVertexBuffer().size())
        BoundingBox.reset(0, 0, 0);
    else
    {
        uint16 const* indicies = (uint16 const*)buffer->getIndices();
        BoundingBox.reset(buffer->getVertexBuffer()[indicies[m_indicesStart]].Pos);

        for (uint32 i = m_indicesStart + 1; i < (m_indicesStart + m_indicesCount); ++i)
            BoundingBox.addInternalPoint(buffer->getVertexBuffer()[indicies[i]].Pos);
    }
}

void CDynamicVertexBufferObject::SetGPUProgram(video::IShader* gpuProgram)
{
    m_gpuProgram = gpuProgram;
}

irr::video::IShader * irr::scene::CDynamicVertexBufferObject::GetGPUProgram() const
{
    return m_gpuProgram;
}

void irr::scene::CDynamicVertexBufferObject::AddConstantBuffer(video::IConstantBuffer* buffer)
{
    //if (mShaderConstantBuffers.size() <= buffer->getBindingIndex())
    //    mShaderConstantBuffers.resize(buffer->getBindingIndex() + 1);
    //
    //mShaderConstantBuffers[buffer->getBindingIndex()] = buffer;
}

void irr::scene::CDynamicVertexBufferObject::AddSubBuffer(u32 istart, u32 icount, u32 vstart, u32 vcount)
{
    SubBuffer.emplace_back(new CDynamicVertexSubBuffer(istart, icount, vstart, vcount));
    Material = &SubBuffer[ActiveSubBuffer]->Material;
}

void irr::scene::CDynamicVertexBufferObject::SetActiveSubBuffer(u16 sid)
{
    if (SubBuffer.size() <= sid || ActiveSubBuffer == sid)
        return;

    m_needBoundRecalulate = true;
    ActiveSubBuffer = sid;
    Material = &SubBuffer[sid]->Material;
}

//! Recalculate bounding box
void irr::scene::CDynamicVertexBufferObject::recalculateBoundingBox()
{
    if (m_needBoundRecalulate)
    {
        if (!getVertexBuffer().size())
            BoundingBox.reset(0, 0, 0);
        else
        {
            BoundingBox.reset(getVertexBuffer()[0].Pos);
            for (u32 i = 1; i < getVertexBuffer().size(); ++i)
                BoundingBox.addInternalPoint(getVertexBuffer()[i].Pos);

            for (u32 i = 0; i != SubBuffer.size(); ++i)
                SubBuffer[i]->recalculateBoundingBox(this);
        }
    }
}

void irr::scene::CDynamicVertexBufferObject::InitSubBuffers(u16 count)
{
    if (!count)
        return;

    if (Material)
        delete Material;
    Material = nullptr;

    SubBuffer.reserve(count);
}

void irr::scene::CDynamicVertexBufferObject::EnableSubBuffer(bool on, u16 sid)
{
    if (SubBuffer.size() <= sid)
    {
        m_isEnabled = on;
        return;
    }

    SubBuffer[sid]->m_isEnabled = on;
}

bool irr::scene::CDynamicVertexBufferObject::IsAvailableSubBuffer(u16 sid)
{
    if (SubBuffer.size() <= sid)
        return m_isEnabled;

    return SubBuffer[sid]->m_isEnabled;
}

const std::vector<video::IConstantBuffer*>* irr::scene::CDynamicVertexBufferObject::GetShaderConstantBuffers() const
{
    //return &mShaderConstantBuffers;
    return nullptr;
}

const irr::core::aabbox3df& irr::scene::CDynamicVertexBufferObject::getBoundingBox() const
{
    return BoundingBox;
}

irr::core::aabbox3df& irr::scene::CDynamicVertexBufferObject::getMutableBoundingBox()
{
    return BoundingBox;
}