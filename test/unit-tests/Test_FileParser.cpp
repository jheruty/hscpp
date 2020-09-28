#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/FileParser.h"
#include "hscpp/Util.h"


namespace hscpp { namespace test
{

    const static fs::path TEST_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "test-file-parser";

    static void ValidateRequire(const FileParser::Require& require,
            FileParser::Require::Type type, const std::vector<fs::path>& paths)
    {
        REQUIRE(require.type == type);
        ValidateOrderedVector(require.paths, paths);
    }

    TEST_CASE("FileParser can parse simple file.", "[FileParser]")
    {
        FileParser parser;

        fs::path filePath= TEST_FILES_PATH / "SimpleFile.cpp";
        FileParser::ParseInfo info = parser.Parse(filePath);

        REQUIRE(info.filePath == filePath);
        REQUIRE(info.requires.size() == 3);
        REQUIRE(info.preprocessorDefinitions.size() == 2);
        REQUIRE(info.modules.size() == 2);
        REQUIRE(info.includePaths.size() == 2);

        CALL(ValidateRequire, info.requires.at(0), FileParser::Require::Type::Include, {
            fs::path("../../fake/path"),
            fs::path("."),
        });

        CALL(ValidateRequire, info.requires.at(1), FileParser::Require::Type::Source, {
            fs::path("../../another/fake/path"),
            fs::path("../"),
        });

        CALL(ValidateRequire, info.requires.at(2), FileParser::Require::Type::Library, {
            fs::path("fakelib.lib"),
            fs::path("../another/fake/lib.lib"),
        });

        CALL(ValidateOrderedVector, info.preprocessorDefinitions, {
            "HSCPP_SAMPLE_DEFINITION1",
            "HSCPP_SAMPLE_DEFINITION2",
        });

        CALL(ValidateOrderedVector, info.modules, {
            "abc",
            "123",
        });

        CALL(ValidateOrderedVector, info.includePaths, {
            fs::path("hscpp/module/PreprocessorMacros.h"),
            fs::path("FakeIncludePath.h"),
        });
    }

    TEST_CASE("FileParser can skip comments and strings.", "[FileParser]")
    {
        FileParser parser;

        fs::path filePath = TEST_FILES_PATH / "SkipCommentsAndStrings.cpp";
        FileParser::ParseInfo info = parser.Parse(filePath);

        REQUIRE(info.filePath == filePath);
        REQUIRE(info.requires.size() == 3);
        REQUIRE(info.preprocessorDefinitions.size() == 1);
        REQUIRE(info.modules.size() == 2);
        REQUIRE(info.includePaths.size() == 3);

        CALL(ValidateRequire, info.requires.at(0), FileParser::Require::Type::Include, {
            fs::path("valid-include"),
        });

        CALL(ValidateRequire, info.requires.at(1), FileParser::Require::Type::Library, {
            fs::path("valid-lib"),
        });

        CALL(ValidateRequire, info.requires.at(2), FileParser::Require::Type::Source, {
            fs::path("valid-source"),
        });
    }

}}