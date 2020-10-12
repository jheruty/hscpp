#include "hscpp/Hotswapper.h"

namespace hscpp
{

    Hotswapper::Hotswapper()
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config> pConfig)
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config> pConfig,
    std::unique_ptr<IFileWatcher> pFileWatcher,
            std::unique_ptr<ICompiler> pCompiler)
    {}

    AllocationResolver* Hotswapper::GetAllocationResolver()
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
    {}

    void Hotswapper::DisableFeature(Feature feature)
    {}

    bool Hotswapper::IsFeatureEnabled(Feature feature)
    {
        return false;
    }

    void Hotswapper::TriggerManualBuild()
    {}

    Hotswapper::UpdateResult Hotswapper::Update()
    {
        return UpdateResult::Idle;
    }

    bool Hotswapper::IsCompiling()
    {
        return false;
    }

    void Hotswapper::SetCallbacks(const Callbacks& callbacks)
    {}

    void Hotswapper::DoProtectedCall(const std::function<void()>& cb)
    {
        cb();
    }

    //============================================================================
    // Add & Remove Functions
    //============================================================================

    int Hotswapper::AddIncludeDirectory(const fs::path& directoryPath)
    {
        return -1;
    }

    bool Hotswapper::RemoveIncludeDirectory(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateIncludeDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {}

    void Hotswapper::ClearIncludeDirectories()
    {}

    int Hotswapper::AddSourceDirectory(const fs::path& directoryPath)
    {
        return -1;
    }

    bool Hotswapper::RemoveSourceDirectory(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {}

    void Hotswapper::ClearSourceDirectories()
    {}

    int Hotswapper::AddForceCompiledSourceFile(const fs::path& filePath)
    {
        return -1;
    }

    bool Hotswapper::RemoveForceCompiledSourceFile(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateForceCompiledSourceFiles(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {}

    void Hotswapper::ClearForceCompiledSourceFiles()
    {}

    int Hotswapper::AddLibraryDirectory(const fs::path& directoryPath)
    {
        return -1;
    }

    bool Hotswapper::RemoveLibraryDirectory(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateLibraryDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {}

    void Hotswapper::ClearLibraryDirectories()
    {}

    int Hotswapper::AddLibrary(const fs::path& libraryPath)
    {
        return -1;
    }

    bool Hotswapper::RemoveLibrary(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateLibraries(const std::function<void(int handle, const fs::path& libraryPath)>& cb)
    {}

    void Hotswapper::ClearLibraries()
    {}

    int Hotswapper::AddPreprocessorDefinition(const std::string& definition)
    {
        return -1;
    }

    bool Hotswapper::RemovePreprocessorDefinition(int handle)
    {
        return false;
    }

    void Hotswapper::EnumeratePreprocessorDefinitions(const std::function<void(int handle, const std::string& definition)>& cb)
    {}

    void Hotswapper::ClearPreprocessorDefinitions()
    {}

    int Hotswapper::AddCompileOption(const std::string& option)
    {
        return -1;
    }

    bool Hotswapper::RemoveCompileOption(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateCompileOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {}

    void Hotswapper::ClearCompileOptions()
    {}

    int Hotswapper::AddLinkOption(const std::string& option)
    {
        return -1;
    }

    bool Hotswapper::RemoveLinkOption(int handle)
    {
        return false;
    }

    void Hotswapper::EnumerateLinkOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {}

    void Hotswapper::ClearLinkOptions()
    {}

    void Hotswapper::SetVar(const std::string& name, const std::string& val)
    {}

    bool Hotswapper::RemoveVar(const std::string& name)
    {
        return false;
    }

}