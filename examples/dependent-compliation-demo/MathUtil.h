#pragma once

#include "hscpp/module/PreprocessorMacros.h"

// Declare this file as an "hscpp_module". All files with the same module name ("math" in this case)
// are assumed to be codependent. That is, making a change in a single "math" module file will
// trigger a recompilation of all "math" module files.
//
// Compiling an hscpp_module will also cause all dependent files to be recompiled. For example, both
// Printer1.cpp and Printer2.cpp include MathUtil.h. Therefore, changing ANY file in the "math" module
// will trigger a compilation of "MathUtil.cpp", "MathUtilExtra.cpp", "Printer1.cpp" and "Printer2.cpp".
// This allows non-virtual methods to be swapped out at runtime, since all dependents of those methods
// will also be swapped.
//
// Finally, ANY file that depends UPON an hscpp_module will also have all those module files added to
// the build. For example, compiling "Printer1.cpp" will also compile "MathUtil.cpp" and
// "MathUtilExtra.cpp".
// 
// Note that compiling a file in the "math" module also triggers a compilation of "StringUtil.cpp".
// This is because "StringUtil.cpp" also belongs to a module (the "string" module), and compiling
// a "math" module file triggers compilation of the DEPENDENT "Printer2.cpp", and "Printer2.cpp"
// DEPENDS on "StringUtil.h". 
hscpp_module("math");

int Add(int a, int b);
int Subtract(int a, int b);


