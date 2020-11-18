#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Util.h"
#include "hscpp/preprocessor/Preprocessor.h"

namespace hscpp { namespace test
{

    const static fs::path TEST_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "test-preprocessor";

    TEST_CASE("Preprocessor can process a simple C++ file.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "simple-test";

        Preprocessor preprocessor;
        preprocessor.SetVar("num", Variant(2.0));

        Preprocessor::Output output;
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Source1.cpp" }, output));

        CALL(ValidateUnorderedVector, output.sourceFiles, {
            assetsPath / "Source2.cpp",
            assetsPath / "Source3.cpp",
        });

        CALL(ValidateUnorderedVector, output.includeDirectories, {
            assetsPath / "includedir",
        });

        CALL(ValidateUnorderedVector, output.libraries, {
            assetsPath / "lib.lib",
        });

        CALL(ValidateUnorderedVector, output.libraryDirectories, {
            assetsPath / "libdir",
        });

        CALL(ValidateUnorderedVector, output.preprocessorDefinitions, {
           "PREPROCESSOR2",
           "IF",
           "SOURCE2",
           "SOURCE3",
        });
    }

}}