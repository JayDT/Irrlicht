// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_IREFERENCE_COUNTED_H_INCLUDED__
#define __I_IREFERENCE_COUNTED_H_INCLUDED__

#include "irrTypes.h"

#include <atomic>

namespace irr
{

    //! Base class of most objects of the Irrlicht Engine.
    /** This class provides reference counting through the methods grab() and drop().
    It also is able to store a debug string for every instance of an object.
    Most objects of the Irrlicht
    Engine are derived from IReferenceCounted, and so they are reference counted.

    When you create an object in the Irrlicht engine, calling a method
    which starts with 'create', an object is created, and you get a pointer
    to the new object. If you no longer need the object, you have
    to call drop(). This will destroy the object, if grab() was not called
    in another part of you program, because this part still needs the object.
    Note, that you only need to call drop() to the object, if you created it,
    and the method had a 'create' in it.

    A simple example:

    If you want to create a texture, you may want to call an imaginable method
    IDriver::createTexture. You call
    ITexture* texture = driver->createTexture(dimension2d<u32>(128, 128));
    If you no longer need the texture, call texture->drop().

    If you want to load a texture, you may want to call imaginable method
    IDriver::loadTexture. You do this like
    ITexture* texture = driver->loadTexture("example.jpg");
    You will not have to drop the pointer to the loaded texture, because
    the name of the method does not start with 'create'. The texture
    is stored somewhere by the driver.
    */
    class IRRLICHT_API IReferenceCounted //MTA
    {
        constexpr static int32_t _ObjectAlreadyDeleted = int32_t(0x8BADF00D);

        bool OnDestroy() const
        {
            ReferenceCounter = _ObjectAlreadyDeleted;
            delete this;
            return true;
        }

    public:

        //! Constructor.
        IReferenceCounted()
            : ReferenceCounter(1)
        {
        }

        //! Destructor.
        virtual ~IReferenceCounted()
        {
        }

        //! Grabs the object. Increments the reference counter by one.
        /** Someone who calls grab() to an object, should later also
        call drop() to it. If an object never gets as much drop() as
        grab() calls, it will never be destroyed. The
        IReferenceCounted class provides a basic reference counting
        mechanism with its methods grab() and drop(). Most objects of
        the Irrlicht Engine are derived from IReferenceCounted, and so
        they are reference counted.

        When you create an object in the Irrlicht engine, calling a
        method which starts with 'create', an object is created, and
        you get a pointer to the new object. If you no longer need the
        object, you have to call drop(). This will destroy the object,
        if grab() was not called in another part of you program,
        because this part still needs the object. Note, that you only
        need to call drop() to the object, if you created it, and the
        method had a 'create' in it.

        A simple example:

        If you want to create a texture, you may want to call an
        imaginable method IDriver::createTexture. You call
        ITexture* texture = driver->createTexture(dimension2d<u32>(128, 128));
        If you no longer need the texture, call texture->drop().
        If you want to load a texture, you may want to call imaginable
        method IDriver::loadTexture. You do this like
        ITexture* texture = driver->loadTexture("example.jpg");
        You will not have to drop the pointer to the loaded texture,
        because the name of the method does not start with 'create'.
        The texture is stored somewhere by the driver. */
        virtual void grab() const noexcept
        {
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            ++ReferenceCounter;
        }

        //! Drops the object. Decrements the reference counter by one.
        /** The IReferenceCounted class provides a basic reference
        counting mechanism with its methods grab() and drop(). Most
        objects of the Irrlicht Engine are derived from
        IReferenceCounted, and so they are reference counted.

        When you create an object in the Irrlicht engine, calling a
        method which starts with 'create', an object is created, and
        you get a pointer to the new object. If you no longer need the
        object, you have to call drop(). This will destroy the object,
        if grab() was not called in another part of you program,
        because this part still needs the object. Note, that you only
        need to call drop() to the object, if you created it, and the
        method had a 'create' in it.

        A simple example:

        If you want to create a texture, you may want to call an
        imaginable method IDriver::createTexture. You call
        ITexture* texture = driver->createTexture(dimension2d<u32>(128, 128));
        If you no longer need the texture, call texture->drop().
        If you want to load a texture, you may want to call imaginable
        method IDriver::loadTexture. You do this like
        ITexture* texture = driver->loadTexture("example.jpg");
        You will not have to drop the pointer to the loaded texture,
        because the name of the method does not start with 'create'.
        The texture is stored somewhere by the driver.
        \return True, if the object was deleted. */
        virtual bool drop() const noexcept
        {
            // someone is doing bad reference counting.
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            _IRR_DEBUG_BREAK_IF(ReferenceCounter <= 0);

            --ReferenceCounter;
            if (!ReferenceCounter)
                return OnDestroy();

            return false;
        }

        //! Get the reference count.
        /** \return Current value of the reference counter. */
        s32 getReferenceCount() const
        {
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            return ReferenceCounter;
        }

        //! Returns the debug name of the object.
        /** The Debugname may only be set and changed by the object
        itself. This method should only be used in Debug mode.
        \return Returns a string, previously set by setDebugName(); */
        const c8* getDebugName() const
        {
#ifdef _DEBUG
            return DebugName;
#endif
            return ""; // DebugName;
        }

    protected:

        //! Sets the debug name of the object.
        /** The Debugname may only be set and changed by the object
        itself. This method should only be used in Debug mode.
        \param newName: New debug name to set. */
        void setDebugName(const c8* newName)
        {
#ifdef _DEBUG
            DebugName = newName;
#endif
        }

    private:

#ifdef _DEBUG
        //! The debug name.
        const c8* DebugName;
#endif

        //! The reference counter. Mutable to do reference counting on const objects.
        mutable std::atomic_int ReferenceCounter;
    };


    class IRRLICHT_API IReferenceCountedSTA
    {
        constexpr static int32_t _ObjectAlreadyDeleted = int32_t(0x8BADF00D);

        bool OnDestroy() const
        {
            ReferenceCounter = _ObjectAlreadyDeleted;
            delete this;
            return true;
        }

    public:

        //! Constructor.
        IReferenceCountedSTA()
            : ReferenceCounter(1)
        {
        }

        IReferenceCountedSTA(const IReferenceCountedSTA&) = delete;
        IReferenceCountedSTA(IReferenceCountedSTA&&) = delete;
        void operator=(const IReferenceCountedSTA&) = delete;
        void operator=(IReferenceCountedSTA&&) = delete;

        //! Destructor.
        virtual ~IReferenceCountedSTA()
        {
        }

        //! Grabs the object. Increments the reference counter by one.
        /** Someone who calls grab() to an object, should later also
        call drop() to it. If an object never gets as much drop() as
        grab() calls, it will never be destroyed. The
        IReferenceCounted class provides a basic reference counting
        mechanism with its methods grab() and drop(). Most objects of
        the Irrlicht Engine are derived from IReferenceCounted, and so
        they are reference counted.

        When you create an object in the Irrlicht engine, calling a
        method which starts with 'create', an object is created, and
        you get a pointer to the new object. If you no longer need the
        object, you have to call drop(). This will destroy the object,
        if grab() was not called in another part of you program,
        because this part still needs the object. Note, that you only
        need to call drop() to the object, if you created it, and the
        method had a 'create' in it.

        A simple example:

        If you want to create a texture, you may want to call an
        imaginable method IDriver::createTexture. You call
        ITexture* texture = driver->createTexture(dimension2d<u32>(128, 128));
        If you no longer need the texture, call texture->drop().
        If you want to load a texture, you may want to call imaginable
        method IDriver::loadTexture. You do this like
        ITexture* texture = driver->loadTexture("example.jpg");
        You will not have to drop the pointer to the loaded texture,
        because the name of the method does not start with 'create'.
        The texture is stored somewhere by the driver. */
        virtual void grab() const noexcept
        {
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            ++ReferenceCounter;
        }

        //! Drops the object. Decrements the reference counter by one.
        /** The IReferenceCounted class provides a basic reference
        counting mechanism with its methods grab() and drop(). Most
        objects of the Irrlicht Engine are derived from
        IReferenceCounted, and so they are reference counted.

        When you create an object in the Irrlicht engine, calling a
        method which starts with 'create', an object is created, and
        you get a pointer to the new object. If you no longer need the
        object, you have to call drop(). This will destroy the object,
        if grab() was not called in another part of you program,
        because this part still needs the object. Note, that you only
        need to call drop() to the object, if you created it, and the
        method had a 'create' in it.

        A simple example:

        If you want to create a texture, you may want to call an
        imaginable method IDriver::createTexture. You call
        ITexture* texture = driver->createTexture(dimension2d<u32>(128, 128));
        If you no longer need the texture, call texture->drop().
        If you want to load a texture, you may want to call imaginable
        method IDriver::loadTexture. You do this like
        ITexture* texture = driver->loadTexture("example.jpg");
        You will not have to drop the pointer to the loaded texture,
        because the name of the method does not start with 'create'.
        The texture is stored somewhere by the driver.
        \return True, if the object was deleted. */
        virtual bool drop() const noexcept
        {
            // someone is doing bad reference counting.
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            _IRR_DEBUG_BREAK_IF(ReferenceCounter <= 0);

            --ReferenceCounter;
            if (!ReferenceCounter)
                return OnDestroy();

            return false;
        }

        //! Get the reference count.
        /** \return Current value of the reference counter. */
        s32 getReferenceCount() const
        {
            _IRR_DEBUG_BREAK_IF(ReferenceCounter == _ObjectAlreadyDeleted);
            return ReferenceCounter;
        }

        //! Returns the debug name of the object.
        /** The Debugname may only be set and changed by the object
        itself. This method should only be used in Debug mode.
        \return Returns a string, previously set by setDebugName(); */
        const c8* getDebugName() const
        {
#ifdef _DEBUG
            return DebugName;
#endif
            return ""; // DebugName;
        }

    protected:

        //! Sets the debug name of the object.
        /** The Debugname may only be set and changed by the object
        itself. This method should only be used in Debug mode.
        \param newName: New debug name to set. */
        void setDebugName(const c8* newName)
        {
#ifdef _DEBUG
            DebugName = newName;
#endif
        }

    private:

#ifdef _DEBUG
        //! The debug name.
        const c8* DebugName;
#endif

        //! The reference counter. Mutable to do reference counting on const objects.
        mutable s32 ReferenceCounter;
    };

    // Cloning Noesis implementation
    template<class T> class Ptr
    {
    public:
        typedef T Type;

        /// Constructs empty smart pointer
        inline Ptr();
        inline Ptr(std::nullptr_t);

        /// Constructor from pointer, increasing reference counter
        inline explicit Ptr(T* ptr);

        /// Constructor from dereferenced pointer, without increasing reference counter.
        /// Very useful for assigning from new operator: Ptr<Cube> cube = *new Cube(50.0f);
        /// MakePtr is a more flexible alternative to this pattern.
        inline Ptr(T& ptr);

        /// Copy constructors
        inline Ptr(const Ptr& ptr);
        template<class S> inline Ptr(const Ptr<S>& ptr);

        /// Move constructors
        inline Ptr(Ptr&& ptr);
        template<class S> inline Ptr(Ptr<S>&& ptr);

        /// Destructor
        inline ~Ptr();

        /// Copy operators
        inline Ptr& operator=(const Ptr& ptr);
        template<class S> inline Ptr& operator=(const Ptr<S>& ptr);

        /// Move operators
        inline Ptr& operator=(Ptr&& ptr);
        template<class S> inline Ptr& operator=(Ptr<S>&& ptr);

        /// Copy from from dereferenced pointer. without increasing reference counter.
        /// Very useful for assigning from new operator: cube = *new Cube(50.0f);
        inline Ptr& operator=(T& ptr);

        /// Resets to null pointer
        inline void Reset();

        /// Resets to null pointer
        inline bool Release();

        /// Resets to pointer, increasing reference counter
        inline void Reset(T* ptr);

        /// Clears the stored pointer without decrementing the reference counter
        inline T* GiveOwnership();

        /// Dereferences the stored pointer
        inline T* operator->() const;

        /// Returns the stored pointer 
        inline T* GetPtr() const;

        /// Automatic conversion to pointer
        inline operator T* () const;

    private:
        T* mPtr;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr() : mPtr(0) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr(std::nullptr_t) : mPtr(0) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr(T* ptr) : mPtr(ptr)
    {
        if (mPtr != 0)
        {
            mPtr->grab();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr(T& ptr) : mPtr(&ptr) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr(const Ptr& ptr) : mPtr(ptr.mPtr)
    {
        if (mPtr != 0)
        {
            mPtr->grab();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    template<class S> Ptr<T>::Ptr(const Ptr<S>& ptr) : mPtr(ptr.GetPtr())
    {
        if (mPtr != 0)
        {
            mPtr->grab();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::Ptr(Ptr&& ptr) : mPtr(ptr.mPtr)
    {
        ptr.GiveOwnership();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    template<class S> Ptr<T>::Ptr(Ptr<S>&& ptr) : mPtr(ptr.GetPtr())
    {
        ptr.GiveOwnership();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::~Ptr()
    {
        if (mPtr != 0)
        {
            mPtr->drop();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>& Ptr<T>::operator=(const Ptr& ptr)
    {
        Reset(ptr.mPtr);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    template<class S> Ptr<T>& Ptr<T>::operator=(const Ptr<S>& ptr)
    {
        Reset(ptr.GetPtr());
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>& Ptr<T>::operator=(Ptr&& ptr)
    {
        if (mPtr != ptr.mPtr)
        {
            if (mPtr != 0)
            {
                mPtr->drop();
            }

            mPtr = ptr.mPtr;
            ptr.GiveOwnership();
        }

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    template<class S> Ptr<T>& Ptr<T>::operator=(Ptr<S>&& ptr)
    {
        // GiveOwnership() done at the beginning to avoid a bug in MSVC2015 Update3 when compiling with 
        // Whole Program Optimization and Omit Frame Pointers enabled
        S* obj = ptr.GetPtr();
        ptr.GiveOwnership();

        if (mPtr != 0)
        {
            mPtr->drop();
        }

        mPtr = obj;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>& Ptr<T>::operator=(T& ptr)
    {
        if (mPtr != &ptr)
        {
            if (mPtr != 0)
            {
                mPtr->drop();
            }

            mPtr = &ptr;
        }

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void Ptr<T>::Reset()
    {
        if (mPtr != 0)
        {
            mPtr->drop();
        }

        mPtr = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    bool Ptr<T>::Release()
    {
        bool _deleted = false;
        if (mPtr != 0)
        {
            _deleted = mPtr->drop();
        }

        mPtr = 0;
        return _deleted;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void Ptr<T>::Reset(T* ptr)
    {
        if (mPtr != ptr)
        {
            if (ptr != 0)
            {
                ptr->grab();
            }

            if (mPtr != 0)
            {
                mPtr->drop();
            }

            mPtr = ptr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    T* Ptr<T>::GiveOwnership()
    {
        T* ptr = mPtr;
        mPtr = 0;
        return ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    T* Ptr<T>::operator->() const
    {
        _IRR_DEBUG_BREAK_IF(mPtr == 0);
        return mPtr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    T* Ptr<T>::GetPtr() const
    {
        return mPtr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T>::operator T* () const
    {
        return mPtr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T> MakePtr()
    {
        return *new T();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T, class... Args>
    Ptr<T> MakePtr(Args && ... args)
    {
        return *new T(std::forward<Args>(args)...);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    Ptr<T> ToSmartPtr(T* ptr)
    {
        if (ptr)
            return Ptr<T>(ptr);
        return {};
    }

} // end namespace irr

#endif

