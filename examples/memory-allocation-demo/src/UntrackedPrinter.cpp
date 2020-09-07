#include <iostream>

#include "memory-allocation-demo/UntrackedPrinter.h"

UntrackedPrinter::UntrackedPrinter()
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

    std::cout << "Constructing UntrackedPrinter" << std::endl;

    // Comment out if enabling HSCPP_TRACK
    //HSCPP_SET_SWAP_HANDLER(cb);
}

UntrackedPrinter::~UntrackedPrinter()
{
    std::cout << "Destructing UntrackedPrinter" << std::endl;
}

void UntrackedPrinter::Init(int value)
{
    m_Value = value;
}

void UntrackedPrinter::Update()
{
    std::cout << "Untracked Value: " << m_Value << std::endl;
}
