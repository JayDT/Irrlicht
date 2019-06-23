#ifndef __CDYNAMIC_VERTEX_BUFFER_OBJECT_H__
#define __CDYNAMIC_VERTEX_BUFFER_OBJECT_H__

#include "IVertexBuffer.h"
#include "SMaterial.h"
#include "IMeshBuffer.h"
#include "IDynamicMeshBuffer.h"

#include <vector>

namespace irr
{
    namespace video
    {
        struct IConstantBuffer;
    }

    namespace scene
    {
        struct CDynamicVertexBufferObject;

        template <class T>
        class CDynamicVerticies : public irr::scene::IVertexBuffer
        {
        public:
            irr::core::array<T> Vertices;
            irr::scene::E_HARDWARE_MAPPING MappingHint : 8;
            u8 ChangedID;

            explicit CDynamicVerticies() : ChangedID(1), MappingHint(irr::scene::EHM_NEVER)
            {
            }

            virtual ~CDynamicVerticies()
            {
            }

            virtual u32 stride() const { return sizeof(T); }
            virtual u32 size() const { return Vertices.size(); }

            virtual irr::video::S3DVertex* getBaseVertexPointer()
            {
                return Vertices.pointer();
            }

            virtual void push_back(const irr::video::S3DVertex &element)
            {
                Vertices.push_back((T&)element);
            }

            virtual void operator ()(const u32 index, irr::video::S3DVertex const& element) const
            {
                (T&)Vertices[index] = (T&)element;
            }

            virtual irr::video::S3DVertex& operator [](const u32 index) const
            {
                return (T&)Vertices[index];
            }

            virtual irr::video::S3DVertex& getLast()
            {
                return (irr::video::S3DVertex&)Vertices.getLast();
            }

            virtual void set_used(u32 usedNow)
            {
                Vertices.set_used(usedNow);
            }

            virtual void reallocate(u32 new_size)
            {
                Vertices.reallocate(new_size);
            }

            virtual u32 allocated_size() const
            {
                return Vertices.allocated_size();
            }

            virtual irr::video::S3DVertex* pointer() { return Vertices.pointer(); }
            virtual irr::video::E_VERTEX_TYPE getType() const 
            { 
                static T empty;
                return empty.getType();
            }

            //! get the current hardware mapping hint
            virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint() const
            {
                return MappingHint;
            }

            //! set the hardware mapping hint, for driver
            virtual void setHardwareMappingHint(irr::scene::E_HARDWARE_MAPPING NewMappingHint)
            {
                MappingHint = NewMappingHint;
            }

            //! flags the mesh as changed, reloads hardware buffers
            virtual void setDirty()
            {
                ++ChangedID;
            }

            //! Get the currently used ID for identification of changes.
            /** This shouldn't be used for anything outside the VideoDriver. */
            virtual u32 getChangedID() const { return ChangedID; }

            virtual void setType(irr::video::E_VERTEX_TYPE vertexType)
            {
                throw std::runtime_error("not implement yet!");
            }

            virtual void* getData() { return Vertices.pointer(); }
        };

        template <class T>
        class CDynamicStreamBuffer : public irr::scene::IStreamBuffer
        {
        public:
            irr::core::array<T> Elements;
            irr::scene::E_HARDWARE_MAPPING MappingHint : 8;
            u8 ChangedID;

            explicit CDynamicStreamBuffer() : ChangedID(1), MappingHint(irr::scene::EHM_NEVER)
            {
            }

            virtual ~CDynamicStreamBuffer()
            {
            }

            virtual void clear() override { Elements.clear(); }

            virtual u32 stride() const { return sizeof(T); }
            virtual u32 size() const { return Elements.size(); }

            virtual irr::video::I3DStreamElement* getBaseVertexPointer()
            {
                return Elements.pointer();
            }

            virtual void push_back(const irr::video::I3DStreamElement &element)
            {
                Elements.push_back((T&)element);
            }

            virtual void operator ()(const u32 index, irr::video::I3DStreamElement const& element) const
            {
                (T&)Elements[index] = (T&)element;
            }

            virtual irr::video::I3DStreamElement& operator [](const u32 index) const
            {
                return (T&)Elements[index];
            }

            virtual irr::video::I3DStreamElement& getLast()
            {
                return (irr::video::I3DStreamElement&)Elements.getLast();
            }

            virtual void set_used(u32 usedNow)
            {
                Elements.set_used(usedNow);
            }

            virtual void reallocate(u32 new_size)
            {
                Elements.reallocate(new_size);
            }

            virtual u32 allocated_size() const
            {
                return Elements.allocated_size();
            }

            virtual irr::video::I3DStreamElement* pointer() { return Elements.pointer(); }
            virtual irr::u8 getType() const
            {
                static T empty;
                return empty.getType();
            }

            //! get the current hardware mapping hint
            virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint() const
            {
                return MappingHint;
            }

            //! set the hardware mapping hint, for driver
            virtual void setHardwareMappingHint(irr::scene::E_HARDWARE_MAPPING NewMappingHint)
            {
                MappingHint = NewMappingHint;
            }

            //! flags the mesh as changed, reloads hardware buffers
            virtual void setDirty()
            {
                ++ChangedID;
            }

            //! Get the currently used ID for identification of changes.
            /** This shouldn't be used for anything outside the VideoDriver. */
            virtual u32 getChangedID() const { return ChangedID; }

            virtual void setType(irr::u8 vertexType)
            {
                throw std::runtime_error("not implement yet!");
            }

            virtual void* getData() { return Elements.pointer(); }
        };

        struct CDynamicVertexSubBuffer
        {
            CDynamicVertexSubBuffer()
            {
                m_indicesStart = 0;
                m_indicesCount = 0;
                m_vectiesStart = 0;
                m_vectiesEnd = 0;
                m_isEnabled = true;
            }

            CDynamicVertexSubBuffer(u32 istart, u32 icount, u32 vstart, u32 vend)
            {
                m_indicesStart = istart;
                m_indicesCount = icount;
                m_vectiesStart = vstart;
                m_vectiesEnd = vend;
                m_isEnabled = true;
            }

            ~CDynamicVertexSubBuffer()
            {
            }

            //! Recalculate bounding box
            virtual void recalculateBoundingBox(CDynamicVertexBufferObject*);

            // Draw Range Elements
            u32 m_indicesStart;
            u32 m_indicesCount;
            u32 m_vectiesStart;
            u32 m_vectiesEnd;
            bool m_isEnabled;

            irr::core::aabbox3d<float> BoundingBox;
            irr::video::SMaterial Material;
        };

        struct IRRLICHT_API CDynamicVertexBufferObject : public irr::scene::IDynamicMeshBuffer
        {
            CDynamicVertexBufferObject();
            virtual ~CDynamicVertexBufferObject();

            virtual irr::scene::IVertexBuffer& getVertexBuffer() const override { return *VertexBuffer; }
            virtual irr::scene::IIndexBuffer& getIndexBuffer() const override { return *IndexBuffer; }
            virtual irr::scene::IStreamBuffer* getStreamBuffer() const override { return StreamBuffer; }

            virtual void setVertexBuffer(irr::scene::IVertexBuffer* newVertexBuffer);
            virtual void setIndexBuffer(irr::scene::IIndexBuffer* newIndexBuffer);
            virtual void setStreamBuffer(irr::scene::IStreamBuffer* newStreamBuffer);

            CDynamicVertexSubBuffer const* GetSubBufferDesc(int sid)
            {
                return !SubBuffer.empty() ? SubBuffer[sid] : nullptr;
            }

            //! Get Material of this buffer.
            virtual const irr::video::SMaterial& getMaterial() const
            {
                return (!SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->Material : *Material);
            }

            //! Get Material of this buffer.
            virtual irr::video::SMaterial& getMaterial()
            {
                return (!SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->Material : *Material);
            }

            //! Get Material of this buffer.
            virtual irr::video::SMaterial& getMaterialBySubBufferId(int sid) _IRR_OVERRIDE_
            {
                return (!SubBuffer.empty() ? SubBuffer[sid]->Material : *Material);
            }

            //! Get bounding box
            virtual const irr::core::aabbox3df& getBoundingBox() const;

            irr::core::aabbox3df& getMutableBoundingBox();

            //! Set bounding box
            virtual void setBoundingBox(const irr::core::aabbox3df& box)
            {
                BoundingBox = box;
            }

            //! Get bounding box
            virtual const irr::core::aabbox3d<float>& getSubBoundingBox(int sid) const
            {
                return (!SubBuffer.empty() ? SubBuffer[sid]->BoundingBox : BoundingBox);
            }

            //! Recalculate bounding box
            void recalculateBoundingBox() override;

            virtual void CreateBuffers() {}
            virtual void UpdateBuffers() {}
            virtual void InitSubBuffers(u16 count);
            virtual void AddSubBuffer(u32 istart, u32 icount, u32 vstart, u32 vcount);
            virtual void SetActiveSubBuffer(u16 sid);
            virtual u16 GetActiveSubBuffer() const { return ActiveSubBuffer; }
            virtual u32 GetSubBufferCount() const { return SubBuffer.size(); }

            virtual s32 GetVertexRangeStart() const { return !SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->m_vectiesStart : 0; }
            virtual s32 GetVertexRangeEnd()   const { return !SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->m_vectiesEnd : getVertexCount(); }
            virtual s32 GetIndexRangeStart()  const { return !SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->m_indicesStart : 0; }
            virtual s32 GetIndexRangeCount()  const { return !SubBuffer.empty() ? SubBuffer[ActiveSubBuffer]->m_indicesCount : getIndexCount(); }
            virtual s32 GetVertexStride() { return VertexBuffer ? VertexBuffer->stride() : 0; }
            virtual void boundingBoxNeedsRecalculated(void) { m_needBoundRecalulate = true; }
            const std::vector<video::IConstantBuffer*>* GetShaderConstantBuffers() const override final;

            virtual irr::video::IShader* GetGPUProgram() const;
            virtual void SetGPUProgram(irr::video::IShader* gpuProgram) override;
            virtual void AddConstantBuffer(video::IConstantBuffer* buffer);

            virtual void EnableSubBuffer(bool on, u16 sid);
            virtual bool IsAvailableSubBuffer(u16 sid);

        private:
            irr::video::SMaterial* Material;
            //std::vector<video::IConstantBuffer*> mShaderConstantBuffers;
            std::vector<CDynamicVertexSubBuffer*> SubBuffer;
            irr::Ptr<irr::scene::IVertexBuffer> VertexBuffer;
            irr::Ptr<irr::scene::IStreamBuffer> StreamBuffer; // ToDo: Can be multiple (now support only instance stream)
            irr::Ptr<irr::scene::IIndexBuffer> IndexBuffer;
            mutable irr::video::IShader* m_gpuProgram;
            irr::core::aabbox3df BoundingBox;
            u16 ActiveSubBuffer;
            bool m_isEnabled : 1;
            bool m_needBoundRecalulate : 1;
        };
    }
}
#endif // __CDYNAMIC_VERTEX_BUFFER_OBJECT_H__