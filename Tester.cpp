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
// Tests

void Test_CorrectUsage()
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

void Test_Forget_SetDep()
{
    DependencyInjected<Cog> cog;
    DependencyInjected<Widget> widget;

    cog.SetDependencies({
        nullptr, // &widget,
        nullptr
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
    TEST_EXPECT_NOASSERT(Test_CorrectUsage);
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
Note: Must be run with debugger DETACHED so assertion handlers fire.

Cog::Initialize()
Widget::Initialize()
Cog::DoCogThing()
Widget::DoWidgetThing() - ParameterX = 15
Cog::DoCogThing2()
Widget::Shutdown()
Cog::Shutdown()
*** Test_CorrectUsage() succeeded

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