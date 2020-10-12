#include <string>

#include "hscpp/module/PreprocessorMacros.h"
#include "FakePath.h"

//hscpp_preprocessor_definitions("PREPROCESSOR_DEFINITION_IN_COMMENT")
/*hscpp_preprocessor_definitions("PREPROCESSOR_DEFINITION_IN_COMMENT")*/

void FakeFunction()
{
    std::string str = "hscpp_require_include(PREPROCESSOR_DEFINITION_IN_STRING)";
    hscpp_preprocessor_definitions(VALID_PREPROCESSOR_DEFINITION);

    "hscpp_module(\"skip-me\")";
    //hscpp_module("skip-me");
    hscpp_module("valid-module"); // hscpp_require_module("skip-me")

    "hscpp_require_include(\"skip-me\")";
    //hscpp_require_include("skip-me");
    hscpp_require_include("valid-include"); // hscpp_require_include("skip-me")

    "hscpp_require_lib(\"skip-me\")";
    //hscpp_require_lib("skip-me");
    hscpp_require_lib("valid-lib"); // hscpp_require_lib("skip-me")

    "hscpp_require_source(\"skip-me\")";
    //hscpp_require_source("skip-me");
    hscpp_require_source("valid-source"); // hscpp_require_source("skip-me")

    /*
     * hscpp_module("skip-block-comment")
     * hscpp_require_include("skip-block-comment")
     * hscpp_require_lib("skip-block-comment")
     * hscpp_require_source("skip-block-comment")
     */
}

hscpp_module("another-valid-module") // hscpp_module("invalid-module")
