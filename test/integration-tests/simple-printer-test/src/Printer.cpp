#include "simple-printer-test/Printer.h"
#include "simple-printer-test/GlobalData.h"
#include "integration-test-log/Log.h"

Printer::Printer()
{
    auto cb = [&](hscpp::SwapInfo& info){
        info.Save("m_Count", m_Count);
        hscpp::GlobalUserData::GetAs<GlobalData>()->pInstance = this;
    };

    Hscpp_SetSwapHandler(cb);
}

Printer::UpdateResult Printer::Update()
{
    // <iteration>
    if (m_Count < 3)
    // </iteration>
    {
        // <log>
        LOG_RESULT("Printer update - original code. Count: " << m_Count);
        // </log>

        m_Count++;
    }

    // <return>
    return UpdateResult::Running;
    // </return>
}