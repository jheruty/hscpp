#include "hscpp/Compiler.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler::Compiler()
    {
        m_CmdShell.CreateCmdProcess();

        std::string vsPath = GetVisualStudioPath();
    }

    void Compiler::Compile(const std::vector<std::filesystem::path>& files,
        const std::vector<std::filesystem::path>& includeDirectories)
    {
        for (const auto& file : files)
        {
            m_CmdShell.SendCommand("echo " + file.generic_string());
        }
    }

    void Compiler::Update()
    {
        std::string output;
        m_CmdShell.ReadOutputLine(output);
        Log::Write(LogLevel::Debug, "%s", output.c_str());
    }

    std::string Compiler::GetVisualStudioPath()
    {
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
        // Find the matching compiler version. Versions supported: VS2017, VS2019
        std::string compilerVersion = "16.0";
        switch (_MSC_VER)
        {
            // VS2017
        case 1910:
        case 1911:
        case 1912:
        case 1913:
        case 1914:
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
        std::string query = "%ProgramFiles(x86)/Microsoft Visual Studio/Installer/vswhere"
            " -version " + compilerVersion +
            " -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
            " -property installationPath";

        return ""; // TODO
    }

}