#include <iostream>
#include <unordered_map>

#include "IntegrationTest.h"
#include "hscpp/Util.h"

namespace hscpp { namespace test
{

    const static fs::path SANDBOX_PATH = util::GetHscppTestPath() / "sandbox";
    const static fs::path TEST_CODE_PATH = util::GetHscppTestPath() / "integration-tests";
    const static fs::path TEST_BUILD_PATH = util::GetHscppBuildTestPath() / "integration-tests";

    void IntegrationTest::Init(const std::string& testName,
        const std::string& configuration, Milliseconds timeoutMs)
    {
        m_StartTime = std::chrono::steady_clock::now();
        m_TimeoutMs = timeoutMs;
        m_iOutput = 0;

        fs::path sandboxPath = CALL(InitializeSandbox, TEST_CODE_PATH / testName);

        m_pCmdShell = platform::CreateCmdShell();
        REQUIRE(m_pCmdShell->CreateCmdProcess());

        std::string task = fs::path(TEST_BUILD_PATH / testName / configuration / testName).u8string();
        m_pCmdShell->StartTask(task, 0);

        WaitForLog("[LOADED]:");
    }

    void IntegrationTest::Modify(const fs::path& relativeFilePath,
        std::unordered_map<std::string, std::string> replacements)
    {
        fs::path fullFilePath = SANDBOX_PATH / relativeFilePath;
        CALL(ModifyFile, fullFilePath, replacements);
    }

    void IntegrationTest::VerifyResult(const std::string& expectedResult)
    {
        INFO("Verifying result: '" + expectedResult + "'.");

        std::string result = WaitForLog("[RESULT]:");
        REQUIRE(result == expectedResult);
    }

    void IntegrationTest::VerifyDone()
    {
        WaitForLog("[DONE]:");
    }

    std::string IntegrationTest::WaitForLog(const std::string& logPrefix)
    {
        std::unordered_set<std::string> invalidLogPrefixes = {
            "[FAIL]:",
            "[LOADED]:",
            "[DONE]:",
            "[RESULT]:",
        };

        invalidLogPrefixes.erase(logPrefix);

        auto now = std::chrono::steady_clock::now();
        auto timeElapsedMs = now - m_StartTime;

        while (timeElapsedMs < m_TimeoutMs)
        {
            int taskId = 0;
            m_pCmdShell->Update(taskId);

            const std::vector<std::string>& taskOutput = m_pCmdShell->PeekTaskOutput();
            for (; m_iOutput < taskOutput.size(); ++m_iOutput)
            {
                std::string line = taskOutput.at(m_iOutput);
                std::cout << line << std::endl;

                for (const auto& invalidPrefix : invalidLogPrefixes)
                {
                    if (line.find(invalidPrefix) == 0)
                    {
                        FAIL("Unexpected log: '" << line << "'.");
                    }
                }

                if (line.find(logPrefix) == 0)
                {
                    ++m_iOutput;

                    line.erase(0, logPrefix.size());
                    return util::Trim(line);
                }
            }

            now = std::chrono::steady_clock::now();
            timeElapsedMs = now - m_StartTime;
        }

        FAIL("Timed out waiting for log '" + logPrefix + "'.");
        return "";
    }

}}