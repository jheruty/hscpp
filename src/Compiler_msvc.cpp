#include <assert.h>
#include <fstream>
#include <sstream>

#include "hscpp/Compiler_msvc.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{
    const static std::string COMMAND_FILENAME = "cmdfile";
    const static std::string MODULE_FILENAME = "module.dll";

    Compiler_msvc::Compiler_msvc()
    {
        m_pCmdShell = platform::CreateCmdShell();

        if (m_pCmdShell->CreateCmdProcess())
        {
            StartVsPathTask();
        }
        else
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create compiler cmd process." << log::End();
        }
    }

    bool Compiler_msvc::IsInitialized()
    {
        return m_bInitialized;
    }

    bool Compiler_msvc::StartBuild(const Input& info)
    {
        if (!m_bInitialized)
        {
            log::Info() << HSCPP_LOG_PREFIX << "Compiler is still initializing, skipping compilation." << log::End();
            return false;
        }

        // The command shell uses ANSI, and cl doesn't appear to support reading filenames from UTF-8.
        // To get around this, write filenames to a separate file that cl can read from. This allows
        // us to compile files whose names contain Unicode characters.
        if (!CreateClCommandFile(info))
        {
            return false;
        }

        // Execute compile command.
        m_iCompileOutput = 0;
        m_CompiledModulePath.clear();

        std::string cmd = "cl @\"" + info.buildDirectoryPath.string() + "\\" + COMMAND_FILENAME + "\"";
        m_pCmdShell->StartTask(cmd, static_cast<int>(CompilerTask::Build));

        return true;
    }

    void Compiler_msvc::Update()
    {
        int taskId = -1;
        ICmdShell::TaskState taskState = m_pCmdShell->Update(taskId);

        // If compiling, write out output in real time.
        if (static_cast<CompilerTask>(taskId) == CompilerTask::Build)
        {
            const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();
            for (; m_iCompileOutput < output.size(); ++m_iCompileOutput)
            {
                log::Build() << output.at(m_iCompileOutput) << log::End();
            }
        }

        switch (taskState)
        {
        case ICmdShell::TaskState::Running:
        case ICmdShell::TaskState::Idle:
            // Do nothing.
            break;
        case ICmdShell::TaskState::Done:
            HandleTaskComplete(static_cast<CompilerTask>(taskId));
            break;
        case ICmdShell::TaskState::Error:
            log::Error() << HSCPP_LOG_PREFIX << "Compiler shell task '"
                << taskId << "' resulted in error." << log::End();
            break;
        default:
            assert(false);
            break;
        }
    }

    bool Compiler_msvc::IsCompiling()
    {
        return !m_CompilingModulePath.empty();
    }

    bool Compiler_msvc::HasCompiledModule()
    {
        return !m_CompiledModulePath.empty();
    }

    fs::path Compiler_msvc::PopModule()
    {
        fs::path modulePath = m_CompiledModulePath;
        m_CompiledModulePath.clear();

        return modulePath;
    }

    bool Compiler_msvc::CreateClCommandFile(const Input& info)
    {
        fs::path commandFilePath = info.buildDirectoryPath / COMMAND_FILENAME;
        std::ofstream commandFile(commandFilePath.native().c_str(), std::ios_base::binary);
        std::stringstream command;

        if (!commandFile.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command file "
                << commandFilePath << log::End(".");
            return false;
        }

        // Add the UTF-8 BOM. This is required for cl to read the file correctly.
        commandFile << static_cast<uint8_t>(0xEF);
        commandFile << static_cast<uint8_t>(0xBB);
        commandFile << static_cast<uint8_t>(0xBF);
        commandFile.close();

        // Reopen file and write command.
        commandFile.open(commandFilePath.native().c_str(), std::ios::app);
        if (!commandFile.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open command file "
                << commandFilePath << log::End(".");
            return false;
        }

        for (const auto& option : info.compileOptions)
        {
            command << option << std::endl;
        }

        // Output dll name.
        m_CompilingModulePath = info.buildDirectoryPath / MODULE_FILENAME;
        command << "/Fe" << "\"" << m_CompilingModulePath.u8string() << "\"" << std::endl;

        // Object file output directory. Trailing slash is required.
        command << "/Fo" << "\"" << info.buildDirectoryPath.u8string() << "\"\\" << std::endl;

        for (const auto& includeDirectory : info.includeDirectoryPaths)
        {
            command << "/I " << "\"" << includeDirectory.u8string() << "\"" << std::endl;
        }

        for (const auto& file : info.sourceFilePaths)
        {
            command << "\"" << file.u8string() << "\"" << std::endl;
        }

        for (const auto& library : info.libraryPaths)
        {
            command << "\"" << library.u8string() << "\"" << std::endl;
        }

        for (const auto& preprocessorDefinition : info.preprocessorDefinitions)
        {
            command << "/D" << "\"" << preprocessorDefinition << "\"" << std::endl;
        }

        if (!info.linkOptions.empty())
        {
            command << "/link " << std::endl;
        }

        for (const auto& option : info.linkOptions)
        {
            command << option << std::endl;
        }

        // Print effective command line. The /MP flag causes the VS logo to print multiple times,
        // so the default compile options use /nologo to suppress it.
        log::Build() << "cl " << command.str() << log::End();

        // Write command file.
        commandFile << command.str();

        return true;
    }

    void Compiler_msvc::StartVsPathTask()
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

    bool Compiler_msvc::HandleTaskComplete(CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
        case CompilerTask::GetVsPath:
            return HandleGetVsPathTaskComplete(output);
        case CompilerTask::SetVcVarsAll:
            return HandleSetVcVarsAllTaskComplete(output);
        case CompilerTask::Build:
            return HandleBuildTaskComplete();
        default:
            assert(false);
            break;
        }

        return false;
    }

    bool Compiler_msvc::HandleGetVsPathTaskComplete(const std::vector<std::string>& output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vswhere.exe command." << log::End();
            return false;
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
            log::Error() << HSCPP_LOG_PREFIX << "vswhere.exe failed to find Visual Studio installation path." << log::End();
            return false;
        }

        fs::path vcVarsAllPath = bestVsPath / "VC\\Auxiliary\\Build\\vcvarsall.bat";
        if (!fs::exists(vcVarsAllPath))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Could not find vcvarsall.bat in path " << vcVarsAllPath << log::End(".");
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

        m_pCmdShell->StartTask(command, static_cast<int>(CompilerTask::SetVcVarsAll));
        
        return true;
    }

    bool Compiler_msvc::HandleSetVcVarsAllTaskComplete(std::vector<std::string> output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vcvarsall.bat command." << log::End();
            return false;
        }

        for (auto rIt = output.rbegin(); rIt != output.rend(); ++rIt)
        {
            if (rIt->find("[vcvarsall.bat] Environment initialized") != std::string::npos)
            {
                // Environmental variables set, we can now use 'cl' to compile.
                m_bInitialized = true;
                return true;
            }
        }

        log::Error() << HSCPP_LOG_PREFIX << "Failed to initialize environment with vcvarsall.bat." << log::End();
        return false;
    }

    bool Compiler_msvc::HandleBuildTaskComplete()
    {
        m_CompiledModulePath = m_CompilingModulePath;
        m_CompilingModulePath.clear();

        return true;
    }

}