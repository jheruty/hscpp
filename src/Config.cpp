#include "hscpp/Config.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"

namespace hscpp
{

    CompilerConfig::CompilerConfig()
    {
        // Has to be in .cpp file to avoid circular dependency with Platform.h.
        executable = platform::GetDefaultCompilerExecutable();

        defaultCompileOptions = platform::GetDefaultCompileOptions();
        defaultPreprocessorDefinitions = platform::GetDefaultPreprocessorDefinitions();

        // Add hotswap-cpp include directory as a default include directory, since parts of the
        // library will need to be compiled into each new module.
        defaultIncludeDirectories.push_back(util::GetHscppIncludePath());

        // Add Module.cpp as a default force-compiled source, it contains things like statics
        // needed by each compiled module.
        defaultForceCompiledSourceFiles.push_back(util::GetHscppSourcePath() / "module" / "Module.cpp");
    }

}