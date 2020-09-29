#include <fstream>
#include <sstream>

#include "hscpp/CompilerCmdLine_gcc.h"
#include "hscpp/Log.h"

namespace hscpp
{

    CompilerCmdLine_gcc::CompilerCmdLine_gcc(const CompilerConfig& config)
        : m_Config(config)
    {}

    bool CompilerCmdLine_gcc::GenerateCommandFile(const fs::path &commandFilePath,
                                                  const fs::path& moduleFilePath,
                                                  const ICompiler::Input &input)
    {
        std::ofstream commandFile(commandFilePath.native().c_str());
        std::stringstream command;

        if (!commandFile.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open command file "
                         << commandFilePath << log::End(".");
            return false;
        }

        // Output module name.
        command << "-o " << moduleFilePath.u8string() << std::endl;

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
            command << "\"" << library.u8string() << "\"" << std::endl;
        }

        for (const auto& file : input.sourceFilePaths)
        {
            command << "\"" << file.u8string() << "\"" << std::endl;
        }

        // Print effective command line.
        log::Build() << m_Config.executable.u8string() << " " << command.str() << log::End();

        // Write command file.
        commandFile << command.str();

        return true;
    }

}