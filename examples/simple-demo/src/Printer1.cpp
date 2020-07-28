#include <iostream>

#include "hscpp/Tracker.h"
#include "Printer1.h"
#include <chrono>
#include <thread>

Printer1::Printer1()
{
    HSCPP_SET_SWAP_HANDLER(
        [this](hscpp::SwapInfo& info) {
            switch (info.Phase())
            {
            case hscpp::SwapPhase::BeforeSwap:
                info.Serialize("m_Value", m_Value);
                break;
            case hscpp::SwapPhase::AfterSwap:
                info.Unserialize("m_Value", m_Value);
                break;
            default:
                break;
            }
        });

    std::cout << "CONSTRUCTING" << m_Value << std::endl;
}

Printer1::~Printer1()
{
    std::cout << "DESTRUCTING" << m_Value << std::endl;
}

void Printer1::Update()
{
    std::cout << ++m_Value << std::endl;
    ++m_Value;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}