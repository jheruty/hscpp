#pragma once

#include <functional>
#include <unordered_map>

#include "hscpp/module/SwapInfo.h"
#include "hscpp-example-utils/Variant.h"

class BaseState
{
public:
    void HandleSwap(hscpp::SwapInfo& info);

    Variant Get(const std::string& name);
    void Set(const std::string& name, Variant variant);
    void Enumerate(std::function<void(Variant&)> cb);

    void PrintBaseState();

private:
    std::unordered_map<std::string, Variant> m_State;
};

