#include <iostream>

#include "memory-allocation-demo/TrackedPrinter.h"

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

    std::cout << "Constructing TrackedPrinter" << std::endl;
    Hscpp_SetSwapHandler(cb);
}

TrackedPrinter::~TrackedPrinter()
{
    std::cout << "Destructing TrackedPrinter" << std::endl;
}

void TrackedPrinter::Init(int value)
{
    m_Value = value;
}

void TrackedPrinter::Update()
{
    std::cout << "Tracked Value: " << m_Value << std::endl;
}
