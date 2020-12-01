#include <chrono>
#include <thread>

#include "hscpp/Platform.h"
#include "hscpp/Util.h"
#include "hscpp/Hotswapper.h"
#include "integration-test-log/Log.h"
#include "simple-printer-test/Printer.h"
#include "simple-printer-test/GlobalData.h"

hscpp::fs::path SANDBOX_PATH = hscpp::util::GetHscppTestPath() / "sandbox";
hscpp::fs::path TEST_PATH = hscpp::util::GetHscppTestPath() / "integration-tests";

int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(SANDBOX_PATH / "include");
    swapper.AddIncludeDirectory(TEST_PATH / "integration-test-log" / "include");
    swapper.AddSourceDirectory(SANDBOX_PATH / "src");

    GlobalData data;
    swapper.SetGlobalUserData(&data);

    data.pInstance = swapper.GetAllocationResolver()->Allocate<Printer>();

    LOG_INFO("Waiting for compiler to initialize.");

    while (!swapper.IsCompilerInitialized())
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_LOADED("simple-printer-test loaded.");

    while (data.pInstance->Update() != Printer::UpdateResult::Done)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_DONE("simple-printer-test finished.");
}