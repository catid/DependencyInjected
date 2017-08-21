/*
    Unit Tester for DependencyInjected Pattern
*/

#include "DependencyInjected.h"

#include <iostream>
using namespace std;


class Cog;
class Widget;


//------------------------------------------------------------------------------
// Widget

class Widget
{
public:
    struct Dependencies
    {
        // Using RequiredDependency<> class to enable extra debug checks
        RequiredDependency<Cog> cog;
    };

    bool Initialize(const Dependencies& deps, int parameterX);
    void Shutdown();

    void DoWidgetThing();

private:
    int ParameterX;
    Dependencies Deps;
};


//------------------------------------------------------------------------------
// Cog

class Cog
{
public:
    struct Dependencies
    {
        // Using RequiredDependency<> class to enable extra debug checks
        RequiredDependency<Widget> widget;

        // Optional dependency for demonstrating null checks
        OptionalDependency<Widget> optionalWidget;
    };

    bool Initialize(const Dependencies& deps);
    void Shutdown();

    void DoCogThing();
    void DoCogThing2();

private:
    Dependencies Deps;
};


//------------------------------------------------------------------------------
// Widget

bool Widget::Initialize(const Dependencies& deps, int parameterX)
{
    cout << "Widget::Initialize()" << endl;
    Deps = deps;
    ParameterX = parameterX;
    return true;
}

void Widget::Shutdown()
{
    cout << "Widget::Shutdown()" << endl;
}

void Widget::DoWidgetThing()
{
    cout << "Widget::DoWidgetThing() - ParameterX = " << ParameterX << endl;

    Deps.cog->DoCogThing2();
}


//------------------------------------------------------------------------------
// Cog

bool Cog::Initialize(const Dependencies& deps)
{
    cout << "Cog::Initialize()" << endl;
    Deps = deps;
    return true;
}

void Cog::Shutdown()
{
    cout << "Cog::Shutdown()" << endl;
}

void Cog::DoCogThing()
{
    cout << "Cog::DoCogThing()" << endl;

    Deps.widget->DoWidgetThing();
    if (Deps.optionalWidget)
        Deps.optionalWidget->DoWidgetThing();
}

void Cog::DoCogThing2()
{
    cout << "Cog::DoCogThing2()" << endl;
}


//------------------------------------------------------------------------------
// LeafObject

class LeafObject
{
public:
    void Initialize(NoDependencies, int parameterX)
    {
        ParameterX = parameterX;
        cout << "LeafObject::Initialize()" << endl;
    }
    void Shutdown()
    {
        cout << "LeafObject::Shutdown()" << endl;
    }

    int DoThing()
    {
        return ++ParameterX;
    }

private:
    int ParameterX;
};


//------------------------------------------------------------------------------
// BranchObject

class BranchObject
{
public:
    bool Initialize(NoDependencies, int parameterX)
    {
        cout << "BranchObject::Initialize()" << endl;

        Leaf.SetDependencies({
        });

        Leaf.Initialize(parameterX);
        return true;
    }
    void Shutdown()
    {
        cout << "BranchObject::Shutdown()" << endl;
        Leaf.Shutdown();
    }

    int DoThing()
    {
        return Leaf->DoThing();
    }

private:
    DependencyInjected<LeafObject> Leaf;
};


//------------------------------------------------------------------------------
// IMyInterface

class IMyInterface
{
public:
    virtual ~IMyInterface() {}

    virtual int DoThing() = 0;
};

class MyImplementation : public IMyInterface
{
public:
    bool Initialize(NoDependencies, int parameterX)
    {
        cout << "MyImplementation::Initialize()" << endl;

        Leaf.Initialize(parameterX);
        return true;
    }
    void Shutdown()
    {
        cout << "MyImplementation::Shutdown()" << endl;
        Leaf.Shutdown();
    }

    int DoThing()
    {
        return Leaf->DoThing();
    }

private:
    DependencyInjected<LeafObject> Leaf;
};


//------------------------------------------------------------------------------
// InterfaceUser

class InterfaceUser
{
public:
    struct Dependencies
    {
        RequiredDependency<IMyInterface> Branch;
    };

    bool Initialize(const Dependencies& deps)
    {
        cout << "InterfaceUser::Initialize()" << endl;
        Deps = deps;

        return true;
    }
    void Shutdown()
    {
        cout << "InterfaceUser::Shutdown()" << endl;
    }

    int DoThing()
    {
        return Deps.Branch->DoThing();
    }

private:
    Dependencies Deps;
};


//------------------------------------------------------------------------------
// Tests

void Test_PeerObjects()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        &widget,
        nullptr // optional dependency
    });
    widget.SetDependencies({
        cog
    });

    cog.Initialize();
    widget.Initialize(15);

    cog->DoCogThing();

    widget.Shutdown();
    cog.Shutdown();
}

void Test_NestedDeps()
{
    DependencyInjected<BranchObject> branch;

    branch.SetDependencies({
    });

    branch.Initialize(10);

    cout << "branch->DoThing() = " << branch->DoThing() << endl;
    cout << "branch->DoThing() = " << branch->DoThing() << endl;
    cout << "branch->DoThing() = " << branch->DoThing() << endl;

    branch.Shutdown();
}

void Test_DerivedClass()
{
    DependencyInjected<MyImplementation> branch;
    DependencyInjected<InterfaceUser> user;

    branch.SetDependencies({
    });
    user.SetDependencies({
        branch
    });

    branch.Initialize(10);
    user.Initialize();

    cout << "user->DoThing() = " << user->DoThing() << endl;
    cout << "user->DoThing() = " << user->DoThing() << endl;
    cout << "user->DoThing() = " << user->DoThing() << endl;

    branch.Shutdown();
    user.Shutdown();
}

void Test_Forget_SetDep()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    DependencyInjected<Widget>* nullWidget = nullptr;

    // Note: It will actually fail to compile if nullptr is passed directly here
    cog.SetDependencies({
        nullWidget, // &widget
        nullptr
    });
#if 0
    widget.SetDependencies({
        cog
    });

    cog.Initialize();
    widget.Initialize(15);

    cog->DoCogThing();

    widget.Shutdown();
    cog.Shutdown();
#endif
}

void Test_Forget_InitCog()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        &widget,
        nullptr
    });
    widget.SetDependencies({
        cog
    });

    //cog.Initialize();
    widget.Initialize(15);

    cog->DoCogThing();

    widget.Shutdown();
    cog.Shutdown();
}

void Test_Forget_InitWidget()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        &widget,
        nullptr
    });
    widget.SetDependencies({
        cog
    });

    cog.Initialize();
    //widget.Initialize(15);

    cog->DoCogThing();

    widget.Shutdown();
    cog.Shutdown();
}

void Test_Forget_ShutdownWidget()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        &widget,
        nullptr
    });
    widget.SetDependencies({
        cog
    });

    cog.Initialize();
    widget.Initialize(15);

    cog->DoCogThing();

    //widget.Shutdown();
    cog.Shutdown();
}

void Test_Forget_ShutdownCog()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        &widget,
        nullptr
    });
    widget.SetDependencies({
        cog
    });

    cog.Initialize();
    widget.Initialize(15);

    cog->DoCogThing();

    widget.Shutdown();
    //cog.Shutdown();
}


#define TEST_EXPECT_NOASSERT(function) \
    __try { \
        function(); \
        cout << "*** " #function "() succeeded" << endl << endl; \
    } \
    __except (true) { \
        cout << "!!! Unexpected assertion in " #function "()" << endl; \
        return false; \
    }

#define TEST_EXPECT_ASSERT(function) \
    __try { \
        function(); \
        cout << "!!! Assertion not fired in " #function "()" << endl; \
        return false; \
    } \
    __except (true) { \
        cout << "*** Expected assertion fired in " #function "()" << endl << endl; \
    }

bool RunTests()
{
    TEST_EXPECT_NOASSERT(Test_PeerObjects);
    TEST_EXPECT_NOASSERT(Test_NestedDeps);
    TEST_EXPECT_NOASSERT(Test_DerivedClass);
    TEST_EXPECT_ASSERT(Test_Forget_SetDep);
    TEST_EXPECT_ASSERT(Test_Forget_InitCog);
    TEST_EXPECT_ASSERT(Test_Forget_InitWidget);
    TEST_EXPECT_ASSERT(Test_Forget_ShutdownWidget);
    TEST_EXPECT_ASSERT(Test_Forget_ShutdownCog);

    return true;
}


//------------------------------------------------------------------------------
// Entrypoint

int main()
{
    if (RunTests())
    {
        cout << "Tests PASSED" << endl;
    }
    else
    {
        cout << "!!! Tests FAILED !!!" << endl;
    }

    int x;
    cin >> x;

    return 0;
}

/*
Note: Must be built in Debug mode.
Note: Must be run with debugger DETACHED so assertions can be handled.

Expected Output:

Cog::Initialize()
Widget::Initialize()
Cog::DoCogThing()
Widget::DoWidgetThing() - ParameterX = 15
Cog::DoCogThing2()
Widget::Shutdown()
Cog::Shutdown()
*** Test_PeerObjects() succeeded

BranchObject::Initialize()
LeafObject::Initialize()
branch->DoThing() = 11
branch->DoThing() = 12
branch->DoThing() = 13
BranchObject::Shutdown()
LeafObject::Shutdown()
*** Test_NestedDeps() succeeded

MyImplementation::Initialize()
LeafObject::Initialize()
InterfaceUser::Initialize()
user->DoThing() = 11
user->DoThing() = 12
user->DoThing() = 13
MyImplementation::Shutdown()
LeafObject::Shutdown()
InterfaceUser::Shutdown()
*** Test_DerivedClass() succeeded

*** Expected assertion fired in Test_Forget_SetDep()

Widget::Initialize()
*** Expected assertion fired in Test_Forget_InitCog()

Cog::Initialize()
Cog::DoCogThing()
*** Expected assertion fired in Test_Forget_InitWidget()

Cog::Initialize()
Widget::Initialize()
Cog::DoCogThing()
Widget::DoWidgetThing() - ParameterX = 15
Cog::DoCogThing2()
Cog::Shutdown()
*** Expected assertion fired in Test_Forget_ShutdownWidget()

Cog::Initialize()
Widget::Initialize()
Cog::DoCogThing()
Widget::DoWidgetThing() - ParameterX = 15
Cog::DoCogThing2()
Widget::Shutdown()
*** Expected assertion fired in Test_Forget_ShutdownCog()

Tests PASSED
*/
