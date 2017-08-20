# DependencyInjected
## Light-weight and powerful Dependency Injection pattern for C++.

[https://en.wikipedia.org/wiki/Dependency_injection](https://en.wikipedia.org/wiki/Dependency_injection)

### Dependency Injection allows developers to write code that is:

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

### This implementation of DI has additional features:

(1) Objects look like normal C++ objects - Easy to adapt existing code!

(2) Required and Optional dependencies can be specified and checked

(3) Object instance is always reset with constructor on Initialize(),
	which fixes the problem created by:
		Initialize(); Shutdown(); Initialize();
	where state from the previous instance leaks into the new one.

(4) Compiler optimizations work well with the abstractions (tested in MSVC)

(5) All the extra debugging checks listed below...

### Verifies in debug mode that:

(1) Required object dependencies are set

(2) Required/optional object dependencies are set before use

(3) Objects are initialized before use

(4) Objects are not initialized twice

(5) Objects are explicitly shutdown before they go out of scope

### Example Widget object:

~~~
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
~~~

### Example Usage:

~~~
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
~~~

### Authors

Software by Christopher A. Taylor <mrcatid@gmail.com>

Please reach out if you need support or would like to collaborate on a project.
