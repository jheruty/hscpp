#include <iostream>

#include "hscpp/module/SwapInfo.h"
#include "runtime-require-demo/Printer.h"
#include "runtime-require-demo/PrintVariant.h"
#include "runtime-require-demo/PrintHello.h"

// The hscpp_require macros allow you to specify dependencies without setting those dependencies
// through the hscpp::Hotswapper. These macros will be parsed by hscpp whenever the file is
// saved. Note that these macros will be evaluated if they are found anywhere in the file,
// including within "disabled" ifdefs.
//
// hscpp_require macros are good for quick testing, and can be added and removed at runtime.
//
// Note that macros must contain literal strings, because hscpp will parse this file directly,
// and is completely separate from the C++ preprocessor.

// Add an include directory with hscpp_require_include_dir. Relative paths are always relative
// to the directory in which the source file with the hscpp_require macro is placed. Absolute
// paths are also valid arguments.
hscpp_require_include_dir("../../hscpp-example-utils/include")

// Add source files with hscpp_require_source. All hscpp_require macros support comma separated
// lists of dependencies.
#if 0 // Note that ifdefs have no effect on hscpp_require macros!
hscpp_require_source("./PrintVariant.cpp", "./PrintHello.cpp")
#endif

// Link in a library with hscpp_require_library. Variables can be interpolated using ${VarName}.
// These variables were set in Main.cpp, by calling the hscpp::Hotswapper's SetVar methods.
//
// Variables can also be used within expressions inside of hscpp_if and hscpp_elif. This allows
// for dynamic selection of hscpp macros. Like C++ if statements, hscpp_if statements can be nested.
hscpp_if (os == "Windows")
    hscpp_require_library("${libDirectory}/hscpp-example-utils.lib")
hscpp_elif (os == "Posix")
    hscpp_require_library("${libDirectory}/libhscpp-example-utils.a")
hscpp_else()
    // Diagnostic messages can be printed to the build output with hscpp_message.
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
        // Passing the hscpp::SwapInfo to the parent class allows the parent to serialize its state.
        BaseState::HandleSwap(info);
    });

    if (Hscpp_IsSwapping())
    {
        return;
    }

    // Set is a function in the base class. These will not be reevaluated on runtime swaps, due to the
    // Hscpp_IsSwapping() check above.
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
    // after this file has been recompiled.
    std::cout << "PREPROCESSOR_PRINTER_DEMO1 is defined." << std::endl;
#endif

#ifdef PREPROCESSOR_PRINTER_DEMO2
    // This is also defined using an hscpp_preprocessor_definition macro.
    std::cout << "PREPROCESSOR_PRINTER_DEMO2 is defined." << std::endl;
#endif

    // If this Update function is called from within the hscpp::Hotswapper's DoProtectedCall method,
    // an exception will cause the Hotswapper to wait for changes, recompile, and then retry the call.
    //
    // Try uncommenting this line as a demonstration.
    // throw std::runtime_error("Uncomment me to demo DoProtectedCall");
}
