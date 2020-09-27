#include <cassert>
#include <locale>
#include <fstream>

#include "hscpp/Compiler_gcclike.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    const static std::string COMMAND_FILENAME = "cmdfile";

#if defined(HSCPP_PLATFORM_WIN32)
    const static std::string MODULE_FILENAME = "module.dll";
#elif defined(HSCPP_PLATFORM_APPLE)
    const static std::string MODULE_FILENAME = "module.dynlib";
#elif defined(HSCPP_PLATFORM_UNIX)
    const static std::string MODULE_FILENAME = "module.so";
#endif

    Compiler_gcclike::Compiler_gcclike(Compiler_gcclike::Type type)
        : m_CompilerType(type)
    {
        switch (type)
        {
            case Type::Clang:
                m_ExecutableName = "clang";
                break;
            case Type::GCC:
                m_ExecutableName = "g++";
                break;
            default:
                assert(false);
        }

        m_pCmdShell = platform::CreateCmdShell();
        if (!m_pCmdShell->CreateCmdProcess())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command process." << log::End();
        }

        std::string versionCmd = m_ExecutableName + " --version";
        m_pCmdShell->StartTask(versionCmd, static_cast<int>(CompilerTask::GetVersion));
    }

    bool Compiler_gcclike::IsInitialized()
    {
        return m_bInitialized;
    }

    bool Compiler_gcclike::StartBuild(const Input& input)
    {
        if (!m_bInitialized)
        {
            log::Info() << HSCPP_LOG_PREFIX << "Compiler is still initializing, skipping compilation." << log::End();
            return false;
        }

        // The command shell may be using ANSI. Write command arguments to a file, such that we can
        // support UTF-8 filenames.
        if (!CreateCommandFile(input))
        {
            return false;
        }

        // Execute compile command.
        m_iCompileOutput = 0;
        m_CompiledModulePath.clear();

        std::string cmd = m_ExecutableName + " @\"" + input.buildDirectoryPath.string()
                + "/" + COMMAND_FILENAME + "\"";

        m_pCmdShell->StartTask(cmd, static_cast<int>(CompilerTask::Build));

        return true;
    }

    void Compiler_gcclike::Update()
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

    bool Compiler_gcclike::IsCompiling()
    {
        return !m_CompilingModulePath.empty();
    }

    bool Compiler_gcclike::HasCompiledModule()
    {
        return !m_CompiledModulePath.empty();
    }

    fs::path Compiler_gcclike::PopModule()
    {
        fs::path modulePath = m_CompiledModulePath;
        m_CompiledModulePath.clear();

        return modulePath;
    }

    bool Compiler_gcclike::CreateCommandFile(const ICompiler::Input &input)
    {
        fs::path commandFilePath = input.buildDirectoryPath / COMMAND_FILENAME;
        std::ofstream commandFile(commandFilePath.native().c_str());
        std::stringstream command;

        // Reopen file and write command.
        if (!commandFile.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open command file "
                         << commandFilePath << log::End(".");
            return false;
        }

        // Output module name.
        m_CompilingModulePath = input.buildDirectoryPath / MODULE_FILENAME;
        command << "-o " << m_CompilingModulePath << std::endl;

        for (const auto& option : input.compileOptions)
        {
            command << option << std::endl;
        }

        for (const auto& option : input.linkOptions)
        {
            command << option << std::endl;
        }

        for (const auto& preprocessorDefinition : input.preprocessorDefinitions)
        {
            command << "-D " << "\"" << preprocessorDefinition << "\"" << std::endl;
        }

        for (const auto& includeDirectory : input.includeDirectoryPaths)
        {
            command << "-I " << "\"" << includeDirectory.u8string() << "\"" << std::endl;
        }

        for (const auto& library : input.libraryPaths)
        {
            command << "-l " << "\"" << library.u8string() << "\"" << std::endl;
        }

        for (const auto& file : input.sourceFilePaths)
        {
            command << "\"" << file.u8string() << "\"" << std::endl;
        }

        // Print effective command line.
        log::Build() << m_ExecutableName << " " << command.str() << log::End();

        // Write command file.
        commandFile << command.str();

        return true;
    }

    bool Compiler_gcclike::HandleTaskComplete(Compiler_gcclike::CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
            case CompilerTask::GetVersion:
                return HandleGetVersionTaskComplete(output);
            case CompilerTask::Build:
                return HandleBuildTaskComplete();
            default:
                assert(false);
                break;
        }

        return false;
    }

    bool Compiler_gcclike::HandleGetVersionTaskComplete(const std::vector<std::string>& output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get " << m_ExecutableName << " version." << log::End();
            return false;
        }

        // Expect to find m_ExecutableName (in lowercase) somewhere in the first line of the version string.
        std::string version;
        for (const auto& c : output.at(0))
        {
            version += std::tolower(c);
        }

        if (version.find(m_ExecutableName) == std::string::npos)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Looking for compiler " << m_ExecutableName
                 << " but found non-matching version '" << output.at(0) << log::End("'.");
            return false;
        }

        m_bInitialized = true;

        return true;
    }

    bool Compiler_gcclike::HandleBuildTaskComplete()
    {
        m_CompiledModulePath = m_CompilingModulePath;
        m_CompilingModulePath.clear();

        return true;
    }

}