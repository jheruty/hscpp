#include <iostream>

#include "runtime-require-demo/BaseState.h"
#include "runtime-require-demo/BaseStateUtil.h"
#include "hscpp/module/PreprocessorMacros.h"

// The Preprocessor will recursively evaluate files. If a source file uses
// hscpp_require_source("./BaseState.cpp"), the hscpp_requires of BaseState.cpp will also be
// evaluated. Modify Printer.cpp to see how BaseStateUtil.cpp is added to the compilation.
hscpp_message("Processing BaseState.cpp!")
hscpp_require_source("./BaseStateUtil.cpp")

void BaseState::HandleSwap(hscpp::SwapInfo& info)
{
    info.Save("Base::m_State", m_State);
}

Variant BaseState::Get(const std::string& name)
{
    auto it = m_State.find(name);
    if (it != m_State.end())
    {
        return it->second;
    }

    return Variant(0);
}

void BaseState::Set(const std::string& name, Variant variant)
{
    m_State[name] = variant;
}

void BaseState::Enumerate(std::function<void(Variant&)> cb)
{
    for (auto& pair : m_State)
    {
        cb(pair.second);
    }
}

void BaseState::PrintBaseState()
{
    // From BaseStateUtil.
    std::cout << GetBaseStateString() << std::endl;
}
