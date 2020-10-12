#include "runtime-require-demo/BaseState.h"

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
