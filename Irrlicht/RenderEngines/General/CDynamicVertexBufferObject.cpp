#include "CDynamicVertexBufferObject.h"

#include "CNullDriver.h"

using namespace irr;
using namespace irr::scene;

CDynamicVertexBufferObject::~CDynamicVertexBufferObject()
{
    assert(getReferenceCount() == 0);

    if (SubBuffer.empty())
    {
        if (Material)
            delete Material;
    }

    for (u32 i = 0; i != SubBuffer.size(); ++i)
        delete SubBuffer[i];

    for (u32 i = 0; i != mShaderConstantBuffers.size(); ++i)
        if (mShaderConstantBuffers[i])
            delete mShaderConstantBuffers[i];

    setVertexBuffer(nullptr);
    setIndexBuffer(nullptr);
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
    //if (m_gpuProgram)
    //    sShaderMgr->RemoveToReloadMeshBufferSet(this);
    if (m_gpuProgramId != -1)
        m_gpuProgramId = -1;
    m_gpuProgram = gpuProgram;

    //if (m_gpuProgram)
    //    sShaderMgr->AddToReloadMeshBufferSet(this, m_gpuProgram);
}

irr::video::IShader * irr::scene::CDynamicVertexBufferObject::GetGPUProgram() const
{
    if (m_gpuProgramId == -1)
        return m_gpuProgram;
    return Driver->GetShaderModul(m_gpuProgramId);
}

void irr::scene::CDynamicVertexBufferObject::AddConstantBuffer(video::IConstantBuffer* buffer)
{
    if (mShaderConstantBuffers.size() <= buffer->getBindingIndex())
        mShaderConstantBuffers.resize(buffer->getBindingIndex() + 1);

    mShaderConstantBuffers[buffer->getBindingIndex()] = buffer;
}

void irr::scene::CDynamicVertexBufferObject::AddSubBuffer(u16 istart, u16 icount, u16 vstart, u16 vcount)
{
    SubBuffer.push_back(new CDynamicVertexSubBuffer(istart, icount, vstart, vcount));
    Material = &SubBuffer[ActiveSubBuffer]->Material;

    //for (u32 i = 0; i != mShaderConstantBuffers.size(); ++i)
    //    if (mShaderConstantBuffers[i])
    //        mShaderConstantBuffers[i]->addSubBuffer();
}

void irr::scene::CDynamicVertexBufferObject::SetActiveSubBuffer(u16 sid)
{
    if (SubBuffer.size() <= sid || ActiveSubBuffer == sid)
        return;

    m_needBoundRecalulate = true;
    ActiveSubBuffer = sid;
    Material = &SubBuffer[sid]->Material;

    //for (u32 i = 0; i != mShaderConstantBuffers.size(); ++i)
    //    if (mShaderConstantBuffers[i])
    //        mShaderConstantBuffers[i]->setActiveSubBuffer(ActiveSubBuffer);
}