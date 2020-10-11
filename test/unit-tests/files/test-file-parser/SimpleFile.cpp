#include "hscpp/module/PreprocessorMacros.h"
#include "FakeIncludePath.h"

hscpp_module("abc", "123");
hscpp_preprocessor_definitions(HSCPP_SAMPLE_DEFINITION1, "HSCPP_SAMPLE_DEFINITION2");
hscpp_require_include("../../fake/path", ".");
hscpp_require_source("../../another/fake/path", "../");
hscpp_require_lib_directory("../fake_lib_directory", "moredirs");
hscpp_require_lib("fakelib.lib", "../another/fake/lib.lib");
