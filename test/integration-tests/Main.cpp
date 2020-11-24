#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Util.h"
#include "hscpp/Platform.h"
#include "IntegrationTest.h"

static std::string configuration;

int main(int argc, char** argv)
{
    Catch::Session session;

    // Configuration needed for Visual Studio builds.
    // For example, simple-demo-test will be located in:
    //      <build_directory>/simple-demo-test/Debug/simple-demo-test
    auto cli = session.cli()
        | Catch::clara::Opt(configuration, "Configuration")
            ["-c"]["--configuration"]
            ("Build configuration (ex. Debug). Can be left empty.");

    session.cli(cli);

    int ret = session.applyCommandLine(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    return session.run();
}

namespace hscpp { namespace test {

    const static fs::path TEST_CODE_PATH = util::GetHscppTestPath() / "integration-tests";
    const static fs::path TEST_BUILD_PATH = util::GetHscppBuildTestPath() / "integration-tests";

#ifndef HSCPP_DISABLE

    TEST_CASE("Integration test 'simple-printer-test' passes.")
    {
        IntegrationTest test;
        CALL(test.Init, "simple-printer-test", configuration, Milliseconds(30000));

        CALL(test.VerifyResult, "Printer update - original code. Count: 0");
        CALL(test.VerifyResult, "Printer update - original code. Count: 1");
        CALL(test.VerifyResult, "Printer update - original code. Count: 2");

        CALL(test.Modify, "src/Printer.cpp", {
            { "iteration", "if (m_Count < 4)" },
            { "log", "LOG_RESULT(\"Printer update - first modification. Count: \" << m_Count);"},
        });

        CALL(test.VerifyResult, "Printer update - first modification. Count: 3");

        CALL(test.Modify, "src/Printer.cpp", {
            { "iteration", "if (m_Count < 100)" },
            { "log", "LOG_RESULT(\"Printer update - second modification. Count: \" << m_Count);"},
            { "return", "return UpdateResult::Done;"},
        });

        CALL(test.VerifyResult, "Printer update - second modification. Count: 4");
        CALL(test.VerifyDone);
    }

#else

    TEST_CASE("hscpp is disabled, skipping integration tests.")
    {}

#endif

}}