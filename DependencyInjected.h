/*
    Copyright (c) 2017 Christopher A. Taylor.  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of DependencyInjected nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

/*
    DependencyInjected

    Light-weight and powerful Dependency Injection pattern for C++.
    https://en.wikipedia.org/wiki/Dependency_injection

    Dependency Injection allows developers to write code that is:

    (1) Testable-
        Each dependency can be mocked out so the objects can be
        tested in isolation.
    (2) Maintainable-
        Each object is reduced to an interface of public member functions
        that can be implemented better in future software updates.
    (3) Better for a team environment-
        A large software system can be decomposed into blackbox components
        with well defined interfaces, and the task to develop each component
        can be assigned to different engineers.  Since the interfaces can be
        defined ahead of having fully working software, developers can start
        on their tasks sooner with mocked interfaces.

    This implementation of DI has additional features:

    (1) Objects look like normal C++ objects - Easy to adapt existing code!
    (2) Required and Optional dependencies can be specified and checked
    (3) Object instance is always reset with constructor on Initialize(),
        which fixes the problem created by:
            Initialize(); Shutdown(); Initialize();
        where state from the previous instance leaks into the new one.
    (4) Compiler optimizations work well with the abstractions (tested in MSVC)
    (5) All the extra debugging checks listed below...

    Verifies in debug mode that:

    (1) Required object dependencies are set
    (2) Required/optional object dependencies are set before use
    (3) Objects are initialized before use
    (4) Objects are not initialized twice
    (5) Objects are explicitly shutdown before they go out of scope
*/

/*
    Example Widget object:


    class Widget
    {
    public:
        struct Dependencies
        {
            RequiredDependency<Cog> cog;

            OptionalDependency<OtherCog> optionalCog;
        };

        bool Initialize(const Dependencies& deps, int parameterX)
        {
            Deps = deps;
            ParameterX = parameterX;

            return true;
        }
        void Shutdown();

        void DoWidgetThing()
        {
            Deps.cog->DoCogThing();

            if (Deps.optionalCog)
                Deps.optionalCog->DoCogThing();
        }

    private:
        int ParameterX;
        Dependencies Deps;
    };
*/

/*
    Example Usage:


    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        widget
    });
    widget.SetDependencies({
        cog,
        nullptr // optional dependency
    });

    // Objects can be initialized and shutdown repeatedly, cleanly:
    for (int i = 0; i < 1000; ++i)
    {
        cog.Initialize();
        widget.Initialize(15);

        cog->DoCogThing();

        widget.Shutdown();
        cog.Shutdown();
    }
*/


//------------------------------------------------------------------------------
// Portability macros

// Compiler-specific debug break
#if defined(_DEBUG) || defined(DEBUG)
    #define DI_DEBUG
    #if defined(_WIN32)
        #define DI_DEBUG_BREAK() __debugbreak()
    #else // _WIN32
        #define DI_DEBUG_BREAK() __builtin_trap()
    #endif // _WIN32
    #define DI_DEBUG_ASSERT(cond) { if (!(cond)) { DI_DEBUG_BREAK(); } }
#else // _DEBUG
    #define DI_DEBUG_BREAK() do {} while (false);
    #define DI_DEBUG_ASSERT(cond) do {} while (false);
#endif // _DEBUG

// Compiler-specific force inline keyword
#if defined(_MSC_VER)
    #define DI_FORCE_INLINE inline __forceinline
#else // _MSC_VER
    #define DI_FORCE_INLINE inline __attribute__((always_inline))
#endif // _MSC_VER


//------------------------------------------------------------------------------
// DependencyInjected
//
// Smart pointer with dependency injection

class IDependencyInjected
{
public:
    virtual ~IDependencyInjected() {}

    DI_FORCE_INLINE operator bool() const
    {
        return IsInitialized();
    }
    DI_FORCE_INLINE bool IsInitialized() const
    {
        return Initialized;
    }

protected:
    bool Initialized = false;
};

template<class T>
class DependencyInjected : public IDependencyInjected
{
public:
    typedef typename T::Dependencies DepsT;

    // Set dependencies
    DI_FORCE_INLINE void SetDependencies(const DepsT& deps)
    {
        // Catch setting dependencies after Initialize() in debug mode
        DI_DEBUG_ASSERT(!IsInitialized());

        Deps = deps;
        SetDeps = true;
    }

    // Initialize the object
    template<typename... Args>
    auto Initialize(Args&&... args)
    {
        // Catch double-initialization in debug mode
        DI_DEBUG_ASSERT(SetDeps && !IsInitialized());

        Initialized = true;

        // Create the object instance (placement new)
        Instance = new (&Memory)T();

        // Initialize the object
        return Instance->Initialize(Deps, args...);
    }

    // Shutdown the object
    template<typename... Args>
    void Shutdown(Args&&... args)
    {
        if (Instance)
        {
            // Invoke the derived class OnShutdown() method
            Instance->Shutdown(args...);

            // Call the deallocator
            Instance->~T();

            // Clear class memory
            memset(Memory, 0, sizeof(Memory));

            // Clear the object instance pointer
            Instance = nullptr;

            Initialized = false;
        }
    }

    DI_FORCE_INLINE explicit DependencyInjected()
    {
        // Ensure that object memory state is the same in debug and release
        memset(Memory, 0, sizeof(Memory));
    }

    DI_FORCE_INLINE virtual ~DependencyInjected()
    {
        // Catch never calling Shutdown() before object goes out of scope
        DI_DEBUG_ASSERT(!IsInitialized());

        Shutdown();
    }

    DI_FORCE_INLINE T* operator->() const
    {
        // In debug mode verify that the object is initialized
        DI_DEBUG_ASSERT(IsInitialized());

        return Instance;
    }
    DI_FORCE_INLINE T* GetObjectPtr()
    {
        return reinterpret_cast<T*>(&Memory);
    }

protected:
    // Object instance
    char Memory[sizeof(T)];
    T* Instance = nullptr;

    // Dependencies for the object
    typename DepsT Deps;
    bool SetDeps = false;

    // Deleted methods
    DependencyInjected(const DependencyInjected<T>&) = delete;
    DependencyInjected<T>& operator=(DependencyInjected<T>&) = delete;
};


//------------------------------------------------------------------------------
// OptionalDependency
//
// Use this to specify an optional dependency in a Dependencies list.

/*
    Example:

    struct Dependencies
    {
        RequiredDependency<Widget> widget;

        OptionalDependency<Widget> optionalWidget;
    };
*/

template<class T>
class OptionalDependency
{
protected:
    T* Reference;
    IDependencyInjected* Wrapper;

public:
    virtual ~OptionalDependency() {}

    // Need this to handle nullptr
    OptionalDependency(void* ptr = nullptr)
    {
        Reference = nullptr;
        Wrapper = nullptr;
    }
    template<class S>
    OptionalDependency(DependencyInjected<S>& wrapper)
    {
        Wrapper = &wrapper;
        if (Wrapper)
        {
            // Types must be convertible e.g. implementation to interface
            Reference = wrapper.GetObjectPtr();
        }
        else
            Reference = nullptr;
    }
    template<class S>
    OptionalDependency(DependencyInjected<S>* wrapper)
    {
        Wrapper = wrapper;
        if (Wrapper)
        {
            // Types must be convertible e.g. implementation to interface
            Reference = wrapper->GetObjectPtr();
        }
        else
            Reference = nullptr;
    }

    DI_FORCE_INLINE bool IsInitialized() const
    {
        // Verifies that reference is valid and that object is initialized
        return Wrapper != nullptr && Wrapper->IsInitialized();
    }
    DI_FORCE_INLINE operator bool() const
    {
        return IsInitialized();
    }
    DI_FORCE_INLINE T* operator->() const
    {
        DI_DEBUG_ASSERT(IsInitialized());

        return Reference;
    }
};


//------------------------------------------------------------------------------
// RequiredDependency
//
// Use this to specify a required dependency in a Dependencies list.

/*
    Example:

    struct Dependencies
    {
        RequiredDependency<Widget> widget;

        OptionalDependency<Widget> optionalWidget;
    };
*/

template<class T>
class RequiredDependency : public OptionalDependency<T>
{
public:
    RequiredDependency()
        : OptionalDependency<T>()
    {
    }
    template<class S>
    RequiredDependency(DependencyInjected<S>& wrapper)
        : OptionalDependency<T>(wrapper)
    {
        // Dependency is required
        DI_DEBUG_ASSERT(Reference != nullptr);
    }
    template<class S>
    RequiredDependency(DependencyInjected<S>* wrapper)
        : OptionalDependency<T>(wrapper)
    {
        // Dependency is required
        DI_DEBUG_ASSERT(Reference != nullptr);
    }
};
