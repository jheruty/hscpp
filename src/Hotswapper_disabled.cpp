#include "hscpp/Hotswapper.h"

namespace hscpp
{

    Hotswapper::Hotswapper()
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config>)
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config>,
            std::unique_ptr<IFileWatcher>,
            std::unique_ptr<ICompiler>,
            std::unique_ptr<IPreprocessor>)
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

    void Hotswapper::EnableFeature(Feature)
    {}

    void Hotswapper::DisableFeature(Feature)
    {}

    bool Hotswapper::IsFeatureEnabled(Feature)
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

    bool Hotswapper::IsCompilerInitialized()
    {
        // Return true, so that if user waits on compiler initialization, it will immediately succeed
        // when hscpp is disabled.
        return true;
    }

    void Hotswapper::SetCallbacks(const Callbacks&)
    {}

    void Hotswapper::DoProtectedCall(const std::function<void()>& cb)
    {
        cb();
    }

    //============================================================================
    // Add & Remove Functions
    //============================================================================

    int Hotswapper::AddIncludeDirectory(const fs::path&)
    {
        return -1;
    }

    bool Hotswapper::RemoveIncludeDirectory(int)
    {
        return false;
    }

    void Hotswapper::EnumerateIncludeDirectories(const std::function<void(int, const fs::path&)>&)
    {}

    void Hotswapper::ClearIncludeDirectories()
    {}

    int Hotswapper::AddSourceDirectory(const fs::path&)
    {
        return -1;
    }

    bool Hotswapper::RemoveSourceDirectory(int)
    {
        return false;
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int, const fs::path&)>&)
    {}

    void Hotswapper::ClearSourceDirectories()
    {}

    int Hotswapper::AddForceCompiledSourceFile(const fs::path&)
    {
        return -1;
    }

    bool Hotswapper::RemoveForceCompiledSourceFile(int)
    {
        return false;
    }

    void Hotswapper::EnumerateForceCompiledSourceFiles(const std::function<void(int, const fs::path&)>&)
    {}

    void Hotswapper::ClearForceCompiledSourceFiles()
    {}

    int Hotswapper::AddLibraryDirectory(const fs::path&)
    {
        return -1;
    }

    bool Hotswapper::RemoveLibraryDirectory(int)
    {
        return false;
    }

    void Hotswapper::EnumerateLibraryDirectories(const std::function<void(int, const fs::path&)>&)
    {}

    void Hotswapper::ClearLibraryDirectories()
    {}

    int Hotswapper::AddLibrary(const fs::path&)
    {
        return -1;
    }

    bool Hotswapper::RemoveLibrary(int)
    {
        return false;
    }

    void Hotswapper::EnumerateLibraries(const std::function<void(int, const fs::path&)>&)
    {}

    void Hotswapper::ClearLibraries()
    {}

    int Hotswapper::AddPreprocessorDefinition(const std::string&)
    {
        return -1;
    }

    bool Hotswapper::RemovePreprocessorDefinition(int)
    {
        return false;
    }

    void Hotswapper::EnumeratePreprocessorDefinitions(const std::function<void(int, const std::string&)>&)
    {}

    void Hotswapper::ClearPreprocessorDefinitions()
    {}

    int Hotswapper::AddCompileOption(const std::string&)
    {
        return -1;
    }

    bool Hotswapper::RemoveCompileOption(int)
    {
        return false;
    }

    void Hotswapper::EnumerateCompileOptions(const std::function<void(int, const std::string&)>&)
    {}

    void Hotswapper::ClearCompileOptions()
    {}

    int Hotswapper::AddLinkOption(const std::string&)
    {
        return -1;
    }

    bool Hotswapper::RemoveLinkOption(int)
    {
        return false;
    }

    void Hotswapper::EnumerateLinkOptions(const std::function<void(int, const std::string&)>&)
    {}

    void Hotswapper::ClearLinkOptions()
    {}

    void Hotswapper::SetVar(const std::string&, const std::string&)
    {}

    void Hotswapper::SetVar(const std::string&, const char*)
    {}

    bool Hotswapper::RemoveVar(const std::string&)
    {
        return false;
    }

}