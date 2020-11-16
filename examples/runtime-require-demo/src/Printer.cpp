#include <iostream>

#include "hscpp/module/SwapInfo.h"
#include "runtime-require-demo/Printer.h"
#include "runtime-require-demo/PrintVariant.h"
#include "runtime-require-demo/PrintHello.h"

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
hscpp_require_include_dir("../../hscpp-example-utils/include")

// Add a source files with hscpp_require_source. All hscpp_require macros support comma separated
// lists of dependencies.
#if 0 // Note that ifdefs have no effect on hscpp_require macros!
hscpp_require_source("./PrintVariant.cpp", "./PrintHello.cpp")
#endif

// Link in a library with hscpp_require_lib. Variables can be interpolated using ${VarName}.
hscpp_if (os == "Windows")
    hscpp_require_library("${libDirectory}/hscpp-example-utils.lib")
hscpp_elif (os == "Posix")
    hscpp_require_library("${libDirectory}/libhscpp-example-utils.a")
hscpp_else()
    hscpp_message("Unknown OS ${os}.")
hscpp_end()

// Add preprocessor definitions when this file is compiled. Definitions can be strings or identifiers.
hscpp_require_preprocessor_def("PREPROCESSOR_PRINTER_DEMO1", PREPROCESSOR_PRINTER_DEMO2);

// hscpp_require in comments and strings is ignored.
// hscpp_require_include("dummy")
/* hscpp_require_include("dummy") */
const char* DUMMY_STR = "hscpp_require_include(\"dummmy\")";

Printer::Printer()
{
    // Multiple hscpp_requires are allowed, anywhere in the source file.
    hscpp_require_source("./BaseState.cpp")

    Hscpp_SetSwapHandler([this](hscpp::SwapInfo& info) {
        BaseState::HandleSwap(info);
        });

    if (Hscpp_IsSwapping())
    {
        return;
    }

    // Set from base class.
    Set("DoubleVal", Variant(2.25));
    Set("IntVal", Variant(1));
}

void Printer::Update()
{
    // Remove "./PrintHello.cpp" from hscpp_require_source to see how compilation will fail.
    PrintHello();

    // Enumerate from base class.
    Enumerate([](Variant& v) {
        PrintVariant(v);
        });

    PrintBaseState();

#ifdef PREPROCESSOR_DEMO
    // This is defined in Main.cpp, with the AddPreprocessorDefinition function. This macro is
    // also defined in the project settings, so this will always print.
    std::cout << "PREPROCESSOR_DEMO is defined." << std::endl;
#endif

#ifdef PREPROCESSOR_PRINTER_DEMO1
    // This is defined using an hscpp_preprocessor_definition macro above. It will only be active
    // after this file has been recompiled, causing the macro to be parsed.
    std::cout << "PREPROCESSOR_PRINTER_DEMO1 is defined." << std::endl;
#endif

#ifdef PREPROCESSOR_PRINTER_DEMO2
    std::cout << "PREPROCESSOR_PRINTER_DEMO2 is defined." << std::endl;
#endif

    // If this Update function is called from within the hscpp::Hotswapper's DoProtectedCall method,
    // an exception will cause the Hotswapper to wait for changes, recompile, and then retry the call.
    // throw std::runtime_error("Uncomment me to demo DoProtectedCall");
}
