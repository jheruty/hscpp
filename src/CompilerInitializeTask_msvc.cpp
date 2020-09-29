#include <cassert>

#include "hscpp/CompilerInitializeTask_msvc.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    void CompilerInitializeTask_msvc::Start(ICmdShell* pCmdShell, std::chrono::milliseconds timeout)
    {
        m_pCmdShell = pCmdShell;

        m_StartTime = std::chrono::steady_clock::now();
        m_Timeout = timeout;

        StartVsPathTask();
    }

    ICmdShellTask::TaskState CompilerInitializeTask_msvc::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);

        switch (state)
        {
            case ICmdShell::TaskState::Running:
            case ICmdShell::TaskState::Idle:
                // Do nothing.
                break;
            case ICmdShell::TaskState::Done:
                return HandleTaskComplete(static_cast<CompilerTask>(taskId));
            case ICmdShell::TaskState::Error:
                log::Error() << HSCPP_LOG_PREFIX << "Compiler shell task '"
                             << taskId << "' resulted in error." << log::End();
                return ICmdShellTask::TaskState::Failure;
            default:
                assert(false);
                break;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - m_StartTime > m_Timeout)
        {
            return ICmdShellTask::TaskState::Timeout;
        }

        return ICmdShellTask::TaskState::Running;
    }

    void CompilerInitializeTask_msvc::StartVsPathTask()
    {
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
        // Find the matching compiler version. Versions supported: VS2017 (15.7+), VS2019
        std::string compilerVersion = "16.0";

        int mscVersion = -1;

#ifdef _MSC_VER
        mscVersion = _MSC_VER;
#else
        log::Error() << HSCPP_LOG_PREFIX << "_MSC_VER is not defined, cannot use MSVC compiler." << log::End();
        return;
#endif

        switch (mscVersion)
        {
            case 1914: // First version of VS2017 with std::filesystem support. TODO support VS2015
            case 1915:
            case 1916:
                compilerVersion = "15.0";
                break;
            case 1920:
            case 1921:
            case 1922:
            case 1923:
            case 1924:
            case 1925:
            case 1926:
            case 1927:
                compilerVersion = "16.0";
                break;
            default:
                log::Warning() << HSCPP_LOG_PREFIX << "Unknown compiler version, using default version '"
                               << compilerVersion << log::End("'.");
                break;
        }

        // VS2017 and up ships with vswhere.exe, which can be used to find the Visual Studio install path.
        std::string query = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere\""
                            " -version " + compilerVersion +
                            " -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
                            " -property installationPath";

        m_pCmdShell->StartTask(query, static_cast<int>(CompilerTask::GetVsPath));
    }

    ICmdShellTask::TaskState CompilerInitializeTask_msvc::HandleTaskComplete(
            CompilerInitializeTask_msvc::CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
            case CompilerTask::GetVsPath:
                return HandleGetVsPathTaskComplete(output);
            case CompilerTask::SetVcVarsAll:
                return HandleSetVcVarsAllTaskComplete(output);
            default:
                assert(false);
                break;
        }

        return ICmdShellTask::TaskState::Failure;
    }

    ICmdShellTask::TaskState CompilerInitializeTask_msvc::HandleGetVsPathTaskComplete(
            const std::vector<std::string> &output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vswhere.exe command." << log::End();
            return ICmdShellTask::TaskState::Failure;
        }

        // Find first non-empty line. Results should be sorted by newest VS version first.
        fs::path bestVsPath;
        for (const auto& line : output)
        {
            if (!util::IsWhitespace(line))
            {
                bestVsPath = util::Trim(line);
                break;
            }
        }

        if (bestVsPath.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX
                << "vswhere.exe failed to find Visual Studio installation path." << log::End();
            return ICmdShellTask::TaskState::Failure;
        }

        fs::path vcVarsAllPath = bestVsPath / "VC\\Auxiliary\\Build\\vcvarsall.bat";
        if (!fs::exists(vcVarsAllPath))
        {
            log::Error() << HSCPP_LOG_PREFIX
                << "Could not find vcvarsall.bat in path " << vcVarsAllPath << log::End(".");
            return ICmdShellTask::TaskState::Failure;
        }

        // Determine whether we are running in 32 or 64 bit.
        std::string command = "\"" + vcVarsAllPath.string() + "\"";
        switch (sizeof(void*))
        {
            case 4:
                command += " x86";
                break;
            case 8:
                command += " x86_amd64";
                break;
            default:
                assert(false); // It must be the future!
                break;
        }

        m_pCmdShell->StartTask(command, static_cast<int>(CompilerTask::SetVcVarsAll));

        return ICmdShellTask::TaskState::Running;
    }

    ICmdShellTask::TaskState CompilerInitializeTask_msvc::HandleSetVcVarsAllTaskComplete(
            std::vector<std::string> output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vcvarsall.bat command." << log::End();
            return ICmdShellTask::TaskState::Failure;
        }

        for (auto rIt = output.rbegin(); rIt != output.rend(); ++rIt)
        {
            if (rIt->find("[vcvarsall.bat] Environment initialized") != std::string::npos)
            {
                // Environmental variables set, we can now use 'cl' to compile.
                return ICmdShellTask::TaskState::Success;
            }
        }

        log::Error() << HSCPP_LOG_PREFIX << "Failed to initialize environment with vcvarsall.bat." << log::End();
        return ICmdShellTask::TaskState::Failure;
    }

}