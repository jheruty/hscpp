#include <cassert>

#include "hscpp/Compiler.h"
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

    Compiler::Compiler(const fs::path& executablePath,
                       const fs::path& startupScriptPath,
                       std::unique_ptr<ICmdShellTask> pInitializeTask,
                       std::unique_ptr<ICompilerCmdLine> pCompilerCmdLine)
       : m_ExecutablePath(executablePath)
       , m_StartupScriptPath(startupScriptPath)
       , m_pInitializeTask(std::move(pInitializeTask))
       , m_pCompilerCmdLine(std::move(pCompilerCmdLine))
    {
        m_pCmdShell = platform::CreateCmdShell();
        if (!m_pCmdShell->CreateCmdProcess())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create compiler cmd process." << log::End();
            return;
        }

        m_pInitializeTask->Start(m_pCmdShell.get(), std::chrono::milliseconds(5000));
    }

    bool Compiler::IsInitialized()
    {
        return m_bInitialized;
    }

    bool Compiler::StartBuild(const ICompiler::Input& input)
    {
        if (!m_bInitialized)
        {
            log::Info() << HSCPP_LOG_PREFIX << "Compiler is still initializing, skipping compilation." << log::End();
            return false;
        }

        fs::path commandFilePath = input.buildDirectoryPath / COMMAND_FILENAME;
        fs::path moduleFilePath = input.buildDirectoryPath / MODULE_FILENAME;
        if (!m_pCompilerCmdLine->GenerateCommandFile(commandFilePath, moduleFilePath, input))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to generate command file." << log::End();
            return false;
        }

        // Execute compile command.
        m_iCompileOutput = 0;
        m_CompiledModulePath.clear();
        m_CompilingModulePath = moduleFilePath;

        std::string cmd = "\"" + m_ExecutablePath.u8string() + "\" @\"" + input.buildDirectoryPath.u8string()
                + "/" + COMMAND_FILENAME + "\"";

        m_pCmdShell->StartTask(cmd, static_cast<int>(CompilerTask::Build));

        return true;
    }

    void Compiler::Update()
    {
        if (!m_bInitialized)
        {
            UpdateInitialization();
            return;
        }

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
                log::Error() << HSCPP_LOG_PREFIX << "Compiler shell task '" << taskId
                    << "' resulted in error." << log::End();
                break;
            default:
                assert(false);
                break;
        }
    }

    bool Compiler::IsCompiling()
    {
        return !m_CompilingModulePath.empty();
    }

    bool Compiler::HasCompiledModule()
    {
        return !m_CompiledModulePath.empty();
    }

    fs::path Compiler::PopModule()
    {
        fs::path modulePath = m_CompiledModulePath;
        m_CompiledModulePath.clear();

        return modulePath;
    }

    void Compiler::UpdateInitialization()
    {
        ICmdShellTask::TaskState taskState = m_pInitializeTask->Update();
        switch (taskState)
        {
            case ICmdShellTask::TaskState::Running:
                // Do nothing.
                break;
            case ICmdShellTask::TaskState::Success:
                m_bInitialized = true;
                break;
            case ICmdShellTask::TaskState::Failure:
                log::Error() << HSCPP_LOG_PREFIX << "Failed to initialize compiler." << log::End();
                break;
            case ICmdShellTask::TaskState::Timeout:
                log::Error() << HSCPP_LOG_PREFIX << "Timed out initializing compiler." << log::End();
                break;
            default:
                assert(false);
                break;
        }
    }

    bool Compiler::HandleTaskComplete(Compiler::CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
            case CompilerTask::Build:
                return HandleBuildTaskComplete();
            default:
                assert(false);
                break;
        }

        return false;
    }

    bool Compiler::HandleBuildTaskComplete()
    {
        m_CompiledModulePath = m_CompilingModulePath;
        m_CompilingModulePath.clear();

        return true;
    }

}