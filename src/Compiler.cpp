#include <assert.h>
#include <fstream>

#include "hscpp/Compiler.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{
    const static std::string COMMAND_FILENAME = "cmdfile";

    Compiler::Compiler()
    {
        m_CmdShell.CreateCmdProcess();
        StartVsPathTask();
    }

    bool Compiler::Compile(const CompileInfo& info)
    {
        if (!m_Initialized)
        {
            Log::Write(LogLevel::Debug, "%s: Compiler is not initialized, skipping compilation.\n", __func__);
            return false;
        }

        m_CompileInfo = info;

        // Create a new, temporary build directory.
        if (!CreateBuildDirectory())
        {
            return false;
        }

        // The command shell uses ANSI, and cl doesn't appear to support reading filenames from UTF-8.
        // To get around this, write filenames to a separate file that cl can read from. This allows
        // us to compile files whose names contain Unicode characters.
        if (!CreateClCommandFile())
        {
            return false;
        }

        std::string cmd = "cl /std:c++17 /D WIN32 /EHa /LD /I C:\\Users\\jheru\\Documents\\Projects\\hotswap-cpp\\include "
            "/I C:\\Users\\jheru\\Documents\\Projects\\hotswap-cpp\\examples\\simple-demo\\include @" + m_BuildDirectory.string() + "\\cmdfile";
        m_CmdShell.StartTask(cmd, static_cast<int>(CompilerTask::Compile));

        m_iCompileOutput = 0;
    }

    void Compiler::Update()
    {
        int taskId;
        CmdShell::TaskState taskState = m_CmdShell.Update(taskId);

        // If compiling, write out output in real time.
        if (static_cast<CompilerTask>(taskId) == CompilerTask::Compile)
        {
            const std::vector<std::string>& output = m_CmdShell.PeekTaskOutput();
            for (m_iCompileOutput; m_iCompileOutput < output.size(); ++m_iCompileOutput)
            {
                Log::Write(LogLevel::Info, "%s", output.at(m_iCompileOutput).c_str());
            }
        }

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

    bool Compiler::CreateBuildDirectory()
    {
        std::string guid = CreateGuid();
        std::filesystem::path temp = std::filesystem::temp_directory_path();

        m_BuildDirectory = temp / guid;

        if (!CreateDirectory(m_BuildDirectory.native().c_str(), NULL))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create directory '%s'. [%s]\n",
                __func__, m_BuildDirectory.string().c_str(), GetLastErrorString().c_str());
            return false;
        }

        std::error_code error;
        m_BuildDirectory = std::filesystem::canonical(m_BuildDirectory, error);

        if (error.value() != ERROR_SUCCESS)
        {
            Log::Write(LogLevel::Error, "%s: Failed to create canonical path for temp build directory.\n", __func__);
            return false;
        }

        return true;
    }

    bool Compiler::CreateClCommandFile()
    {
        std::filesystem::path commandFilepath = m_BuildDirectory / COMMAND_FILENAME;
        std::ofstream commandFile(commandFilepath.native().c_str(), std::ios_base::binary);

        if (!commandFile.is_open())
        {
            Log::Write(LogLevel::Error, "%s: Failed to create command file.\n", __func__);
            return false;
        }

        // Add the UTF-8 BOM. This is required for cl to read the file correctly.
        commandFile << static_cast<uint8_t>(0xEF);
        commandFile << static_cast<uint8_t>(0xBB);
        commandFile << static_cast<uint8_t>(0xBF);
        commandFile.close();

        // Reopen file to write filenames.
        commandFile.open(commandFilepath.native().c_str(), std::ios::app);
        if (!commandFile.is_open())
        {
            Log::Write(LogLevel::Error, "%s: Failed to open command file.\n", __func__);
            return false;
        }

        for (const auto& file : m_CompileInfo.files)
        {
            commandFile << file.u8string() << std::endl;
        }

        return true;
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
        case CompilerTask::Compile:
            break;
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