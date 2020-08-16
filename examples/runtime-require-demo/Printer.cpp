#include <iostream>
#include "Printer.h"

#include "hscpp/module/SwapInfo.h"
#include "PrintVariant.h"
#include "PrintHello.h"

// The hscpp_require macros allow you to specify dependencies without setting those dependencies
// through the hscpp::Hotswapper. These macros will be parsed by hscpp whenever the file is
// recompiled. Note that these macros will be evaluated if they are found anywhere in the file,
// including within "disabled" ifdefs.
//
// hscpp_require macros are good for quick testing, and can be added and removed at runtime.
//
// Note that macros must contain literal strings, because hscpp will parse this file directly
// and will not invoke the preprocessor.

// Add a include directory with hscpp_require_include. All paths are relative to the path of the
// file in which the macro is placed (absolute paths are also allowed).
hscpp_require_include("../hscpp-example-utils/include")

// Add a source files with hscpp_require_source. All hscpp_require macros support comma separated
// lists of dependencies.
#if 0 // Note that ifdefs have no effect on hscpp_require macros!
hscpp_require_source("./PrintVariant.cpp", "./PrintHello.cpp")
#endif

// Link in a library with hscpp_require_lib. %PROJECT_CONFIGURATION% is set to either Debug
// or Release, using hscpp::Hotswappers "AddHscppRequireVariable" function.
hscpp_require_lib("../x64/%PROJECT_CONFIGURATION%/hscpp-example-utils.lib")

// hscpp_require in comments and strings is ignored.
// hscpp_require_include("dummy")
/* hscpp_require_include("dummy") */
const char* DUMMY_STR = "hscpp_require_include(\"dummmy\")";

Printer::Printer()
{
    // Multiple hscpp_requires are allowed, anywhere in the source file.
    hscpp_require_source("./BaseState.cpp")

    HSCPP_SET_SWAP_HANDLER([this](hscpp::SwapInfo& info) {
        BaseState::HandleSwap(info);
        });

    if (HSCPP_IS_SWAPPING())
    {
        return;
    }

    // Set from base class.
    Set("DoubleVal", Variant(2.25));
    Set("IntVal", Variant(1));
}

void Printer::Update()
{
    // Remove "./PrinteHello.cpp" from hscpp_require_source to see how compilation will fail.
    PrintHello();

    // Enumerate from base class.
    Enumerate([](Variant& v) {
        Print(v);
        });
}