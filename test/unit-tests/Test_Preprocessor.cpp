#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Util.h"
#include "hscpp/preprocessor/Preprocessor.h"

namespace hscpp { namespace test
{

    const static fs::path TEST_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "test-preprocessor";

    TEST_CASE("Preprocessor can process all hscpp_require types.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "require-test";

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
        Preprocessor::Output output;
        Preprocessor preprocessor;

        SECTION("Dependent compilation works with separated modules.")
        {
            preprocessor.UpdateDependencyGraph({
                assetsPath / "Math.cpp",
                assetsPath / "Math.h",
                assetsPath / "MathDependency.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "Vector.h",
                assetsPath / "VectorDependency.cpp",
            }, {}, { assetsPath });

            std::vector<fs::path> mathPaths = {
                assetsPath / "Math.cpp",
                assetsPath / "MathDependency.cpp",
            };

            std::vector<fs::path> vectorPaths = {
                assetsPath / "Vector.cpp",
                assetsPath / "VectorDependency.cpp",
            };

            REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);
            REQUIRE(preprocessor.Preprocess({ assetsPath / "Math.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);
            REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);
            REQUIRE(preprocessor.Preprocess({ assetsPath / "Vector.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);

            // Add a dependency of both Math.h and Vector.h.
            preprocessor.UpdateDependencyGraph({
                assetsPath / "VectorAndMathDependency.cpp",
            }, {}, { assetsPath });

            std::vector<fs::path> vectorAndMathPaths = {
                assetsPath / "Vector.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "VectorAndMathDependency.cpp",
            };

            REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorAndMathDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, vectorAndMathPaths);
            REQUIRE(preprocessor.Preprocess({ assetsPath / "VectorDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, vectorPaths);
            REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, mathPaths);

            REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp", assetsPath / "VectorDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "Vector.cpp",
                assetsPath / "VectorDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "MathDependency.cpp",
            });

            REQUIRE(preprocessor.Preprocess({ assetsPath / "MathDependency.cpp", assetsPath / "VectorAndMathDependency.cpp" }, output));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "Vector.cpp",
                assetsPath / "VectorAndMathDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "MathDependency.cpp",
            });
        }

        SECTION("Dependent compilation works with interleaved modules.")
        {
            preprocessor.UpdateDependencyGraph({
                assetsPath / "Math.cpp",
                assetsPath / "Math.h",
                assetsPath / "MathDependency.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "Vector.h",
                assetsPath / "VectorDependency.cpp",
                assetsPath / "VectorMath.cpp",
                assetsPath / "VectorMath.h",
                assetsPath / "VectorAndMathDependency.cpp",
                assetsPath / "VectorMathDependency.cpp",
            }, {}, { assetsPath });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "Math.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "Math.cpp",
                assetsPath / "MathDependency.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorMath.cpp",
                assetsPath / "VectorAndMathDependency.cpp",
                assetsPath / "VectorMathDependency.cpp",
            });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "Vector.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "Math.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorDependency.cpp",
                assetsPath / "VectorMath.cpp",
                assetsPath / "VectorAndMathDependency.cpp",
                assetsPath / "VectorMathDependency.cpp",
            });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "MathDependency.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "MathDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorMath.cpp",
            });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "VectorDependency.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "VectorDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorMath.cpp",
            });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "VectorMathDependency.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "VectorMathDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorMath.cpp",
            });

            REQUIRE(preprocessor.Preprocess( { assetsPath / "VectorAndMathDependency.cpp" }, output ));
            CALL(ValidateUnorderedVector, output.sourceFiles, {
                assetsPath / "VectorAndMathDependency.cpp",
                assetsPath / "Math.cpp",
                assetsPath / "Vector.cpp",
                assetsPath / "VectorMath.cpp",
            });
        }
    }

    TEST_CASE("Preprocessor can handle infinite recursion.")
    {
        // These files all add each other as hscpp_require_sources, validate that this does not
        // cause an infinite loop.
        fs::path assetsPath = TEST_FILES_PATH / "infinite-recursion-test";

        Preprocessor preprocessor;

        Preprocessor::Output output;
        REQUIRE(preprocessor.Preprocess({ assetsPath / "File-a.cpp" }, output));

        CALL(ValidateUnorderedVector, output.sourceFiles, {
            assetsPath / "File-a.cpp",
            assetsPath / "File-b.cpp",
            assetsPath / "File-c.cpp",
        });
    }
}}