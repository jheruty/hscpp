#include <thread>

#include "catch/catch.hpp"
#include "common/UpdateLoop.h"

namespace hscpp { namespace test
{

    void StartUpdateLoop(Milliseconds timeoutMs, Milliseconds pollMs,
                         const std::function<UpdateLoop(Milliseconds)>& cb)
    {
        auto now = std::chrono::steady_clock::now();
        auto then = now;

        Milliseconds elapsedMs(0);

        bool bTimedOut = false;
        while (true)
        {
            Milliseconds dt = std::chrono::duration_cast<Milliseconds>(now - then);

            elapsedMs += dt;
            UpdateLoop result = cb(elapsedMs);

            if (elapsedMs > timeoutMs)
            {
                bTimedOut = true;
                break;
            }
            else if (result == UpdateLoop::Timeout)
            {
                bTimedOut = true;
                break;
            }
            else if (result == UpdateLoop::Done)
            {
                break;
            }

            then = now;
            now = std::chrono::steady_clock::now();

            std::this_thread::sleep_for(pollMs);
        }

        if (bTimedOut)
        {
            FAIL("UpdateLoop timed out.");
        }
    }

}}