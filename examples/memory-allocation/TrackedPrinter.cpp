#include <iostream>

#include "TrackedPrinter.h"

TrackedPrinter::TrackedPrinter()
{
    auto cb = [this](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
        case hscpp::SwapPhase::BeforeSwap:
            info.Serialize("Value", m_Value);
            break;
        case hscpp::SwapPhase::AfterSwap:
            info.Unserialize("Value", m_Value);
            break;
        }
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

void TrackedPrinter::Init(int value)
{
    m_Value = value;
}

void TrackedPrinter::Update()
{
    std::cout << "Value: " << m_Value << std::endl;
}
