#pragma once

#include <memory>
#include <chrono>
#include <unordered_map>

#include "common/Common.h"
#include "hscpp/Platform.h"

namespace hscpp { namespace test
{

    class IntegrationTest
    {
    public:
        void Init(const std::string& testName,
            const std::string& configuration, Milliseconds timeoutMs);

        void Modify(const fs::path& relativeFilePath,
            std::unordered_map<std::string, std::string> replacements);

        void VerifyResult(const std::string& expectedResult);
        void VerifyDone();

    private:
        std::unique_ptr<ICmdShell> m_pCmdShell;

        std::chrono::steady_clock::time_point m_StartTime;
        Milliseconds m_TimeoutMs;

        size_t m_iOutput = 0;

        std::string WaitForLog(const std::string& logPrefix);
    };

}}