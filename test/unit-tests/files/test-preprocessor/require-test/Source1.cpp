#include <iostream>
#include "SomeCustomPath"

int SomeFunction()
{
    std::cout << "Hello";
}

hscpp_require_source("Source2.cpp")
hscpp_if (true)

    hscpp_if (false)
        hscpp_return()
    hscpp_end()

    hscpp_require_library("lib.lib")
    hscpp_require_library_dir("libdir")
    hscpp_if (num == 2)
        hscpp_require_preprocessor_def("PREPROCESSOR${num}", IF)
    hscpp_elif (num == 3)
        hscpp_require_preprocessor_def(ELIF)
    hscpp_end()
hscpp_end()

int SomeOtherFunction()
{
    hscpp_require_include_dir("includedir")
    hscpp_message("message")
    hscpp_module("module")
}

hscpp_if(2 == 2)
    hscpp_return()
hscpp_end()

hscpp_require_preprocessor_def("SHOULD_NOT_BE_ADDED")