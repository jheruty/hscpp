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
            assetsPath / "Source1.cpp", // Source files passed to Preprocess are included.
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

    TEST_CASE("Preprocessor can handle dependent compilation.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "dependent-compilation-test";

        Preprocessor preprocessor;
        preprocessor.UpdateDependencyGraph({
                assetsPath / "Math.cpp",
                assetsPath / "Math.h",
                assetsPath / "MathDependency.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "Vector.h",
                assetsPath / "VectorDependency.cpp",
            }, {}, { assetsPath });

        Preprocessor::Output output;

        std::vector<fs::path> mathPaths = {
                assetsPath / "Math.cpp",
                assetsPath / "MathDependency.cpp",
        };

        REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Math.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);

        std::vector<fs::path> vectorPaths = {
            assetsPath / "Vector.cpp",
            assetsPath / "VectorDependency.cpp",
        };

        REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorDependency.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Vector.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);

        // This "links" together Vector and Math libraries. In other words, compiling anything in
        // Math requires compilation of everything in Vector.
        preprocessor.UpdateDependencyGraph({
            assetsPath / "VectorAndMathDependency.cpp",
            assetsPath / "VectorMath.cpp",
            assetsPath / "VectorMath.h",
        }, {}, { assetsPath });

        std::vector<fs::path> vectorMathPaths = {
            assetsPath / "Math.cpp",
            assetsPath / "Vector.cpp",
            assetsPath / "VectorMath.cpp",
            assetsPath / "VectorAndMathDependency.cpp",
            assetsPath / "VectorDependency.cpp",
            assetsPath / "MathDependency.cpp",
        };

        REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorAndMathDependency.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorMathPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorMath.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorMathPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Math.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorMathPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Vector.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorMathPaths);

        // Remove VectorMath.
        preprocessor.UpdateDependencyGraph({}, {
            assetsPath / "VectorAndMathDependency.cpp",
            assetsPath / "VectorMath.cpp",
            assetsPath / "VectorMath.h",
        }, { assetsPath });

        REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Math.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);

        REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorDependency.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);
        REQUIRE(preprocessor.Preprocess({ assetsPath / "Vector.cpp" }, output));
        CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);
    }

}}