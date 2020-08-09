#include <unordered_set>
#include <fstream>
#include <assert.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    const static std::string HSCPP_TEMP_DIRECTORY_NAME = "HSCPP_7c9279ff-25af-488c-a634-b6aa68f47a65";

    // Users may not override these file options.
    const static std::unordered_set<std::string> FORBIDDEN_COMPILE_OPTIONS = {
        "Fe", // Name of compiled module.
        "Fo", // Name of directory to place all object files.
    };
    
    const static std::vector<std::string> DEFAULT_COMPILE_OPTIONS = {
        "/std:c++17", // Use C++17 standard.
        "/Z7", // Add full debugging information.
        "/FC", // Print full filepath in diagnostic messages.
        "/MP", // Build with multiple processes.
        "/EHsc", // Full support for standard C++ exception handling.
#ifdef _DEBUG
        // Debug flags.
        "/MDd", // Use multithreaded debug DLL version of run-time library.
        "/LDd", // Create debug DLL. 
#else
        // Release flags.
        "/MD", // Use multithreaded release DLL version of run-time library.
        "/Zo", // Enable enhanced debugging for optimized code.
        "/LD", // Create release DLL.
#endif
    };

    const static std::vector<std::string> DEFAULT_FILE_EXTENSIONS = {
        ".h",
        ".hh",
        ".hpp",
        ".cpp",
        ".c",
        ".cc",
        ".cxx",
    };

    Hotswapper::Hotswapper(bool bUseDefaults /* = true */)
    {
        if (bUseDefaults)
        {
            for (const auto& option : DEFAULT_COMPILE_OPTIONS)
            {
                Add(option, m_NextCompileOptionHandle, m_CompileOptionsByHandle);
            }

            for (const auto& extension : DEFAULT_FILE_EXTENSIONS)
            {
                Add(extension, m_NextFileExtensionHandle, m_FileExtensionsByHandle);
            }

            // Add hotswap-cpp include directory as a default include directory, since parts of the
            // library will need to be compiled into each new module.
            Add(GetHscppIncludePath(), m_NextIncludeDirectoryHandle, m_IncludeDirectoriesByHandle);
        }
    }

    hscpp::AllocationResolver* Hotswapper::GetAllocationResolver()
    {
        return &m_AllocationResolver;
    }

    void Hotswapper::SetAllocator(IAllocator* pAllocator)
    {
        m_ModuleManager.SetAllocator(pAllocator);
    }

    void Hotswapper::SetGlobalUserData(void* pGlobalUserData)
    {
        m_ModuleManager.SetGlobalUserData(pGlobalUserData);
    }

    void Hotswapper::EnableFeature(Feature feature)
    {
        m_Features.insert(feature);
    }

    void Hotswapper::DisableFeature(Feature feature)
    {
        auto it = m_Features.find(feature);
        if (it != m_Features.end())
        {
            m_Features.erase(it);
        }
    }

    bool Hotswapper::IsFeatureEnabled(Feature feature)
    {
        auto it = m_Features.find(feature);
        return it != m_Features.end();
    }

    void Hotswapper::Update()
    {
        m_FileWatcher.PollChanges(m_FileEvents);
        if (m_FileEvents.size() > 0)
        {
            if (CreateBuildDirectory())
            {
                Compiler::CompileInfo info;
                info.buildDirectory = m_BuildDirectory;
                info.files = GetChangedFiles();
                info.includeDirectories = AsVector(m_IncludeDirectoriesByHandle);
                info.libraries = AsVector(m_LibrariesByHandle);
                info.compileOptions = AsVector(m_CompileOptionsByHandle);
                info.linkOptions = AsVector(m_LinkOptionsByHandle);

                if (IsFeatureEnabled(Feature::HscppRequire))
                {
                    ParseHscppRequire(info);
                }

                if (!info.files.empty())
                {
                    m_Compiler.StartBuild(info);
                }
            }
        }

        m_Compiler.Update();
        if (m_Compiler.HasCompiledModule())
        {
            m_ModuleManager.PerformRuntimeSwap(m_Compiler.PopModule());
        }
    }

    //============================================================================
    // Add & Remove Functions
    //============================================================================

    int Hotswapper::AddIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        return Add(directory, m_NextIncludeDirectoryHandle, m_IncludeDirectoriesByHandle);
    }

    bool Hotswapper::RemoveIncludeDirectory(int handle)
    {
        auto it = m_IncludeDirectoriesByHandle.find(handle);
        if (it != m_IncludeDirectoriesByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_IncludeDirectoriesByHandle);
    }

    void Hotswapper::EnumerateIncludeDirectories(const std::function<void(int handle, const std::filesystem::path& directory)>& cb)
    {
        Enumerate(cb, m_IncludeDirectoriesByHandle);
    }

    void Hotswapper::ClearIncludeDirectories()
    {
        m_IncludeDirectoriesByHandle.clear();
    }

    int Hotswapper::AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive)
    {
        m_FileWatcher.AddWatch(directory, bRecursive);
        return Add(directory, m_NextSourceDirectoryHandle, m_SourceDirectoriesByHandle);
    }

    bool Hotswapper::RemoveSourceDirectory(int handle)
    {
        auto it = m_SourceDirectoriesByHandle.find(handle);
        if (it != m_SourceDirectoriesByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_SourceDirectoriesByHandle);
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int handle, const std::filesystem::path& directory)>& cb)
    {
        Enumerate(cb, m_SourceDirectoriesByHandle);
    }

    void Hotswapper::ClearSourceDirectories()
    {
        m_SourceDirectoriesByHandle.clear();
    }

    int Hotswapper::AddLibrary(const std::filesystem::path& libraryPath)
    {
        return Add(libraryPath, m_NextLibraryHandle, m_LibrariesByHandle);
    }

    bool Hotswapper::RemoveLibrary(int handle)
    {
        return Remove(handle, m_LibrariesByHandle);
    }

    void Hotswapper::EnumerateLibraries(const std::function<void(int handle, const std::filesystem::path& libraryPath)>& cb)
    {
        Enumerate(cb, m_LibrariesByHandle);
    }

    void Hotswapper::ClearLibraries()
    {
        m_LibrariesByHandle.clear();
    }

    int Hotswapper::AddCompileOption(const std::string& option)
    {
        return Add(option, m_NextCompileOptionHandle, m_CompileOptionsByHandle);
    }

    bool Hotswapper::RemoveCompileOption(int handle)
    {
        return Remove(handle, m_CompileOptionsByHandle);
    }

    void Hotswapper::EnumerateCompileOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {
        Enumerate(cb, m_CompileOptionsByHandle);
    }

    void Hotswapper::ClearCompileOptions()
    {
        m_CompileOptionsByHandle.clear();
    }

    int Hotswapper::AddLinkOption(const std::string& option)
    {
        return Add(option, m_NextLinkOptionHandle, m_LinkOptionsByHandle);
    }

    bool Hotswapper::RemoveLinkOption(int handle)
    {
        return Remove(handle, m_LinkOptionsByHandle);
    }

    void Hotswapper::EnumerateLinkOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {
        Enumerate(cb, m_LinkOptionsByHandle);
    }

    void Hotswapper::ClearLinkOptions()
    {
        m_LinkOptionsByHandle.clear();
    }

    int Hotswapper::AddFileExtension(const std::string& extension)
    {
        return Add(extension, m_NextFileExtensionHandle, m_FileExtensionsByHandle);
    }

    bool Hotswapper::RemoveFileExtension(int handle)
    {
        return Remove(handle, m_FileExtensionsByHandle);
    }

    void Hotswapper::EnumerateFileExtensions(const std::function<void(int handle, const std::string& option)>& cb)
    {
        Enumerate(cb, m_FileExtensionsByHandle);
    }

    void Hotswapper::ClearFileExtensions()
    {
        m_FileExtensionsByHandle.clear();
    }

    void Hotswapper::SetHscppRequireVariable(const std::string& name, const std::string& val)
    {
        m_HscppRequireVariables[name] = val;
    }

    //============================================================================

    bool Hotswapper::CreateHscppTempDirectory()
    {
        std::error_code error;
        std::filesystem::path temp = std::filesystem::temp_directory_path(error);

        if (error.value() != ERROR_SUCCESS)
        {
            Log::Write(LogLevel::Error, "%s: Failed to find temp directory path. [%s]\n",
                __func__, GetErrorString(error.value()).c_str());
            return false;
        }

        std::filesystem::path hscppTemp = temp / HSCPP_TEMP_DIRECTORY_NAME;

        std::filesystem::remove_all(hscppTemp, error);
        if (!std::filesystem::create_directory(hscppTemp, error))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create directory '%s'. [%s]\n",
                __func__, hscppTemp.string().c_str(), GetErrorString(error.value()).c_str());
            return false;
        }

        m_HscppTempDirectory = hscppTemp;

        return true;
    }

    bool Hotswapper::CreateBuildDirectory()
    {
        if (m_HscppTempDirectory.empty())
        {
            if (!CreateHscppTempDirectory())
            {
                return false;
            }
        }

        std::string guid = CreateGuid();
        m_BuildDirectory = m_HscppTempDirectory / guid;

        std::error_code error;
        if (!std::filesystem::create_directory(m_BuildDirectory, error))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create directory '%s'. [%s]\n",
                __func__, m_BuildDirectory.string().c_str(), GetErrorString(error.value()).c_str());
            return false;
        }

        return true;
    }

    std::vector<std::filesystem::path> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates.
        std::unordered_set<std::wstring> files;
        for (const auto& event : m_FileEvents)
        {
            if (event.type == FileWatcher::EventType::Removed)
            {
                // Removed files don't matter, we simply will not attempt to compile them.
                continue;
            }

            std::error_code error;
            std::filesystem::path canonicalPath = std::filesystem::canonical(event.filepath, error);

            if (error.value() == ERROR_FILE_NOT_FOUND)
            {
                // While saving a file, temporary copies may be created and then removed. Skip them.
                continue;
            }
            else if (error.value() != ERROR_SUCCESS)
            {
                Log::Write(LogLevel::Error, "%s: Failed to get canonical path of '%s'. [%s]\n",
                    __func__, event.filepath.string().c_str(), GetLastErrorString().c_str());
                continue;
            }

            std::filesystem::path extension = canonicalPath.extension();
            auto extensionIt = std::find_if(m_FileExtensionsByHandle.begin(), m_FileExtensionsByHandle.end(),
                [extension](auto pair) {
                    return extension == pair.second;
                });

            if (extensionIt == m_FileExtensionsByHandle.end())
            {
                Log::Write(LogLevel::Trace, "%s: File '%s' will be skipped; its extension is not being watched.\n",
                    __func__, canonicalPath.string().c_str());
                continue;
            }

            switch (event.type)
            {
            case FileWatcher::EventType::Added:
            case FileWatcher::EventType::Modified:
                files.insert(canonicalPath.wstring());
                break;
            default:
                assert(false);
                break;
            }
        }

        return std::vector<std::filesystem::path>(files.begin(), files.end());
    }

    void Hotswapper::ParseHscppRequire(Compiler::CompileInfo& info)
    {
        std::vector<std::filesystem::path> additionalFiles;
        std::vector<std::filesystem::path> additionalIncludes;
        std::vector<std::filesystem::path> additionalLibraries;

        for (const auto& file : info.files)
        {
            FileParser::ParseInfo parseInfo;
            if (m_FileParser.ParseFile(file, parseInfo))
            {
                for (const auto& dependency : parseInfo.dependencies)
                {
                    for (const auto& path : dependency.paths)
                    {
                        std::filesystem::path fullpath = path;
                        if (path.is_relative())
                        {
                            fullpath = file.parent_path() / path;
                        }

                        std::string replace = fullpath.u8string();
                        for (const auto& var : m_HscppRequireVariables)
                        {
                            std::string search = "%" + var.first + "%";
                            size_t i = replace.find(search);
                            if (i != std::string::npos)
                            {
                                replace.replace(i, search.size(), var.second);
                            }
                        }

                        fullpath = std::filesystem::u8path(replace);
                        fullpath = std::filesystem::canonical(fullpath);

                        switch (dependency.type)
                        {
                        case RuntimeDependency::Type::Source:
                            additionalFiles.push_back(fullpath);
                            break;
                        case RuntimeDependency::Type::Include:
                            additionalIncludes.push_back(fullpath);
                            break;
                        case RuntimeDependency::Type::Library:
                            additionalLibraries.push_back(fullpath);
                            break;
                        default:
                            assert(false);
                            break;
                        }
                    }
                }              
            }
        }

        info.files.insert(info.files.end(), additionalFiles.begin(), additionalFiles.end());
        info.includeDirectories.insert(info.includeDirectories.end(), additionalIncludes.begin(), additionalIncludes.end());
        info.libraries.insert(info.libraries.begin(), additionalLibraries.begin(), additionalLibraries.end());
    }

}
