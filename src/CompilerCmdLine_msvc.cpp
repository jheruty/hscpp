#include <fstream>
#include <sstream>

#include "hscpp/CompilerCmdLine_msvc.h"
#include "hscpp/Log.h"

namespace hscpp
{

    bool CompilerCmdLine_msvc::GenerateCommandFile(const fs::path &commandFilePath,
                                                   const fs::path& moduleFilePath,
                                                   const ICompiler::Input &input)
    {
        std::ofstream commandFile(commandFilePath.native().c_str(), std::ios_base::binary);
        std::stringstream command;

        if (!commandFile.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command file "
                         << commandFilePath << log::End(".");
            return false;
        }

        // Add the UTF-8 BOM (required for cl to deduce UTF-8).
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

        for (const auto& option : input.compileOptions)
        {
            command << option << std::endl;
        }

        // Output dll name.
        command << "/Fe" << "\"" << moduleFilePath.u8string() << "\"" << std::endl;

        // Object file output directory. Trailing slash is required.
        command << "/Fo" << "\"" << input.buildDirectoryPath.u8string() << "\"\\" << std::endl;

        for (const auto& includeDirectory : input.includeDirectoryPaths)
        {
            command << "/I " << "\"" << includeDirectory.u8string() << "\"" << std::endl;
        }

        for (const auto& file : input.sourceFilePaths)
        {
            command << "\"" << file.u8string() << "\"" << std::endl;
        }

        for (const auto& library : input.libraryPaths)
        {
            command << "\"" << library.u8string() << "\"" << std::endl;
        }

        for (const auto& preprocessorDefinition : input.preprocessorDefinitions)
        {
            command << "/D" << "\"" << preprocessorDefinition << "\"" << std::endl;
        }

        if (!input.linkOptions.empty())
        {
            command << "/link " << std::endl;
        }

        for (const auto& option : input.linkOptions)
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

}