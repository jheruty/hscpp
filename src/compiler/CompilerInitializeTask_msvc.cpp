#include <cassert>

#include "hscpp/compiler/CompilerInitializeTask_msvc.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    void CompilerInitializeTask_msvc::Start(ICmdShell* pCmdShell,
                                            std::chrono::milliseconds timeout,
                                            const std::function<void(Result)>& doneCb)
    {
        m_pCmdShell = pCmdShell;
        m_DoneCb = doneCb;

        m_StartTime = std::chrono::steady_clock::now();
        m_Timeout = timeout;

        StartVsPathTask();
    }

    void CompilerInitializeTask_msvc::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);

        switch (state)
        {
            case ICmdShell::TaskState::Running:
            {
                auto now = std::chrono::steady_clock::now();
                if (now - m_StartTime > m_Timeout)
                {
                    m_pCmdShell->CancelTask();
                    TriggerDoneCb(Result::Failure);
                    return;
                }

                break;
            }
            case ICmdShell::TaskState::Idle:
            case ICmdShell::TaskState::Cancelled:
                // Do nothing.
                break;
            case ICmdShell::TaskState::Done:
                HandleTaskComplete(static_cast<CompilerTask>(taskId));
                break;
            case ICmdShell::TaskState::Error:
                log::Error() << HSCPP_LOG_PREFIX << "CmdShell task '"
                             << taskId << "' resulted in error." << log::End();
                TriggerDoneCb(Result::Failure);
                return;
            default:
                assert(false);
                break;
        }
    }

    void CompilerInitializeTask_msvc::TriggerDoneCb(Result result)
    {
        if (m_DoneCb != nullptr)
        {
            m_DoneCb(result);
            m_pCmdShell->Clear();
        }

        m_DoneCb = nullptr;
    }

    void CompilerInitializeTask_msvc::StartVsPathTask()
    {
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
        // Find the matching compiler version. Versions supported: VS2015, VS2017, VS2019
        std::string compilerVersion = "16.0";

        switch (_MSC_VER)
        {
			case 1900:
				compilerVersion = "14.0";
				break;
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
			case 1928:
                compilerVersion = "16.0";
                break;
            default:
                log::Warning() << HSCPP_LOG_PREFIX << "Unknown compiler version, using default version '"
                               << compilerVersion << log::End("'.");
                break;
        }

		if (compilerVersion == "14.0")
		{
			// VS2015 stores installation path in registry key.
			HKEY registryKey;
			std::string registryName = "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7";
			
			LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				registryName.c_str(), 0, KEY_READ | KEY_WOW64_32KEY, &registryKey);

			if (result != HSCPP_ERROR_SUCCESS)
			{
				log::Error() << HSCPP_LOG_PREFIX << "Failed to open registry key '"
					<< registryName << "'." << log::OsError(result) << log::End();
				return;
			}

			char vsPath[MAX_PATH];
			DWORD size = sizeof(vsPath);
			
			result = RegQueryValueEx(registryKey, "14.0", nullptr, nullptr, reinterpret_cast<LPBYTE>(vsPath), &size);
			if (result != HSCPP_ERROR_SUCCESS)
			{
				log::Error() << HSCPP_LOG_PREFIX << "Failed to read registry key value."
					<< log::OsError(result) << log::End();
				return;
			}

			if (!StartVcVarsAllTask(vsPath, "VC"))
			{
				log::Error() << HSCPP_LOG_PREFIX << "Failed to start vcvarsall task." << log::End();
				return;
			}
		}
		else
		{
			// VS2017 and up ships with vswhere.exe, which can be used to find the Visual Studio install path.
			std::string query = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere\""
				" -version " + compilerVersion +
				" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
				" -property installationPath";

			m_pCmdShell->StartTask(query, static_cast<int>(CompilerTask::GetVsPath));
		}
    }

	bool CompilerInitializeTask_msvc::StartVcVarsAllTask(const fs::path& vsPath, const fs::path& vcVarsAllDirectoryPath)
	{
		fs::path vcVarsAllPath = vsPath / vcVarsAllDirectoryPath / "vcvarsall.bat";
		if (!fs::exists(vcVarsAllPath))
		{
			log::Error() << HSCPP_LOG_PREFIX
				<< "Could not find vcvarsall.bat in path " << vcVarsAllPath << log::End(".");
			return false;
		}

		std::string command = "\"" + vcVarsAllPath.string() + "\"";

		// Determine whether we are running in 32 or 64 bit.
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

    void CompilerInitializeTask_msvc::HandleTaskComplete(CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
            case CompilerTask::GetVsPath:
                HandleGetVsPathTaskComplete(output);
                break;
            case CompilerTask::SetVcVarsAll:
                HandleSetVcVarsAllTaskComplete(output);
                break;
            default:
                assert(false);
                break;
        }
    }

    void CompilerInitializeTask_msvc::HandleGetVsPathTaskComplete(
            const std::vector<std::string> &output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vswhere.exe command." << log::End();

            TriggerDoneCb(Result::Failure);
            return;
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

            TriggerDoneCb(Result::Failure);
            return;
        }

		if (!StartVcVarsAllTask(bestVsPath, "VC/Auxiliary/Build"))
		{
			log::Error() << HSCPP_LOG_PREFIX << "Failed to start vcvarsall task." << log::End();

            TriggerDoneCb(Result::Failure);
            return;
		}
    }

    void CompilerInitializeTask_msvc::HandleSetVcVarsAllTaskComplete(
            std::vector<std::string> output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to run vcvarsall.bat command." << log::End();

            TriggerDoneCb(Result::Failure);
            return;
        }

        for (auto rIt = output.rbegin(); rIt != output.rend(); ++rIt)
        {
            if (rIt->find("[vcvarsall.bat] Environment initialized") != std::string::npos)
            {
                // Environmental variables set, we can now use 'cl' to compile.
                TriggerDoneCb(Result::Success);
                return;
            }
        }

        log::Error() << HSCPP_LOG_PREFIX << "Failed to initialize environment with vcvarsall.bat." << log::End();
        TriggerDoneCb(Result::Failure);
    }

}