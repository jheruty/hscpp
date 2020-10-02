#include "hscpp/Config.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"

namespace hscpp
{

    Config::Flag operator|(Config::Flag lhs, Config::Flag rhs)
    {
        return static_cast<Config::Flag>(static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs));
    }

    Config::Flag operator|=(Config::Flag& lhs, Config::Flag rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    bool operator&(Config::Flag lhs, Config::Flag rhs)
    {
        return (static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs)) != 0;
    }

    CompilerConfig::CompilerConfig()
    {
        // Has to be in .cpp file to avoid circular dependency with Platform.h.
        executable = platform::GetDefaultCompilerExecutable();
    }

}