#include <assert.h>

#include "hscpp/Compiler.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    Compiler::Compiler()
    {
        m_CmdShell.CreateCmdProcess();
        StartVsPathTask();
    }

    void Compiler::Compile(const std::vector<std::filesystem::path>& files,
        const std::vector<std::filesystem::path>& includeDirectories)
    {
        if (!m_Initialized)
        {
            Log::Write(LogLevel::Debug, "%s: Compiler is not initialized, skipping compilation.\n", __func__);
            return;
        }
    }

    void Compiler::Update()
    {
        int taskId;
        CmdShell::TaskState taskState = m_CmdShell.Update(taskId);

        switch (taskState)
        {
        case CmdShell::TaskState::Running: // TODO: Timeout on visual studio path, print logs on compiling...
        case CmdShell::TaskState::Idle:
            // Do nothing.
            break;
        case CmdShell::TaskState::Done:
            HandleTaskComplete(static_cast<CompilerTask>(taskId));
            break;
        case CmdShell::TaskState::Error:
            // TODO: Handle error.
            break;
        default:
            assert(false);
            break;
        }
    }

    void Compiler::StartVsPathTask()
{
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
        // Find the matching compiler version. Versions supported: VS2017 (15.7+), VS2019
        std::string compilerVersion = "16.0";
        switch (_MSC_VER)
        {
        case 1914: // First version of VS2017 with std::filesystem support.
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
            Log::Write(LogLevel::Error, "%s: Unknown compiler version, using default version '%s'.\n",
                __func__, compilerVersion.c_str());
        }

        // VS2017 and up ships with vswhere.exe, which can be used to find the Visual Studio install path.
        std::string query = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere\""
            " -version " + compilerVersion +
            " -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
            " -property installationPath";

        m_CmdShell.StartTask(query, static_cast<int>(CompilerTask::GetVsPath));
    }

    bool Compiler::HandleTaskComplete(CompilerTask task)
    {
        const std::vector<std::string>& output = m_CmdShell.PeekTaskOutput();

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

        return false;
    }

    bool Compiler::HandleGetVsPathTaskComplete(const std::vector<std::string>& output)
    {
        if (output.empty())
        {
            Log::Write(LogLevel::Error, "%s: Failed to run vswhere.exe command.\n", __func__);
            return false;
        }

        // Find first non-empty line. Results should be sorted by newest VS version first.
        std::filesystem::path bestVsPath;
        for (const auto& line : output)
        {
            if (!IsWhitespace(line))
            {
                bestVsPath = Trim(line);
                break;
            }
        }

        if (bestVsPath.empty())
        {
            Log::Write(LogLevel::Error, "%s: vswhere.exe failed to find Visual Studio installation path.\n", __func__);
            return false;
        }

        std::filesystem::path vcVarsAllPath = bestVsPath / "VC\\Auxiliary\\Build\\vcvarsall.bat";
        if (!std::filesystem::exists(vcVarsAllPath))
        {
            Log::Write(LogLevel::Error, "%s: Could not find vcvarsall.bat in path %s.\n",
                __func__, vcVarsAllPath.string().c_str());
            return false;
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

        m_CmdShell.StartTask(command, static_cast<int>(CompilerTask::SetVcVarsAll));
        
        return true;
    }

    bool Compiler::HandleSetVcVarsAllTaskComplete(std::vector<std::string> output)
    {
        if (output.empty())
        {
            Log::Write(LogLevel::Error, "%s: Failed to run vcvarsall.bat command.\n", __func__);
            return false;
        }

        for (auto rIt = output.rbegin(); rIt != output.rend(); ++rIt)
        {
            if (rIt->find("[vcvarsall.bat] Environment initialized") != std::string::npos)
            {
                // Environmental variables set, we can now use 'cl' to compile.
                m_Initialized = true;
                return true;
            }
        }

        Log::Write(LogLevel::Error, "%s: Failed to initialize environment.\n", __func__);
        return false;
    }

}