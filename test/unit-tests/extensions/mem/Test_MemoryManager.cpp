#include <thread>

#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/mem/MemoryManager.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"
#include "hscpp/Hotswapper.h"

namespace hscpp { namespace test {

    template <typename T>
    using UniqueRef = hscpp::mem::UniqueRef<T>;

    template <typename T>
    using Ref = hscpp::mem::Ref<T>;

    void RunTest(const std::function<void(UniqueRef<hscpp::mem::MemoryManager>)>& cb)
    {
        UniqueRef<hscpp::mem::MemoryManager> rMemoryManager = hscpp::mem::MemoryManager::Create();
        CALL(cb, std::move(rMemoryManager));

        hscpp::Hotswapper swapper;

        hscpp::mem::MemoryManager::Config config;
        config.pAllocationResolver = swapper.GetAllocationResolver();

        rMemoryManager = hscpp::mem::MemoryManager::Create(config);
        swapper.SetAllocator(&rMemoryManager);

        CALL(cb, std::move(rMemoryManager));
    }

    TEST_CASE("MemoryManager can handle basic allocations.")
    {
        auto cb = [](UniqueRef<hscpp::mem::MemoryManager> rMemoryManager){
            struct Data
            {
                int a = 100;
                double b = 1.5;
            };

            // Intentional scope.
            {
                UniqueRef<Data> rUniqueData = rMemoryManager->Allocate<Data>();
                REQUIRE(rMemoryManager->GetNumBlocks() == 1);
                REQUIRE(rUniqueData->a == 100);
                REQUIRE(rUniqueData->b == 1.5);

                Ref<Data> rData = rUniqueData;
                REQUIRE(rData->a == 100);
                REQUIRE(rData->b == 1.5);

                // Both refer to same memory location.
                rUniqueData->a = 200;
                rData->b = 2.5;

                REQUIRE(rUniqueData->a == 200);
                REQUIRE(rUniqueData->b == 2.5);
                REQUIRE(rData->a == 200);
                REQUIRE(rData->b == 2.5);

                // New instance moved to rUniqueData.
                rUniqueData = rMemoryManager->Allocate<Data>();
                REQUIRE(rMemoryManager->GetNumBlocks() == 1);
                REQUIRE(rUniqueData->a == 100);
                REQUIRE(rUniqueData->b == 1.5);

                // Refs refer to freed memory.
                CHECK_THROWS(rData->a);
                CHECK_THROWS(rData->b);

                UniqueRef<Data> rMoreUniqueData = rMemoryManager->Allocate<Data>();
                REQUIRE(rMemoryManager->GetNumBlocks() == 2);

                // The original memory should be reused, so the old Ref should work again. This
                // is because the MemoryManager will recycle old memory blocks.
                rUniqueData = std::move(rMoreUniqueData);
                REQUIRE(rMemoryManager->GetNumBlocks() == 1);
                REQUIRE(rData->a == 100);
                REQUIRE(rData->b == 1.5);

                // Data has been moved.
                CHECK_THROWS(rMoreUniqueData->a);
                CHECK_THROWS(rMoreUniqueData->b);
            }

            // UniqueRefs went out of scope.
            REQUIRE(rMemoryManager->GetNumBlocks() == 0);
        };

        CALL(RunTest, cb);
    }

    TEST_CASE("MemoryManager can handle many allocations.")
    {
        auto cb = [](UniqueRef<hscpp::mem::MemoryManager> rMemoryManager){
            struct Data
            {
                std::string Str;
            };

            std::vector<UniqueRef<Data>> data;
            std::unordered_set<size_t> indices;

            size_t nData = 10000;
            for (size_t i = 0; i < nData; ++i)
            {
                data.push_back(rMemoryManager->Allocate<Data>());
                data.back()->Str = "Data " + std::to_string(i);

                indices.insert(i);
            }

            // Do a simple check of all items in list.
            REQUIRE(rMemoryManager->GetNumBlocks() == nData);
            for (size_t i = 0; i < nData; ++i)
            {
                REQUIRE(data.at(i)->Str == "Data " + std::to_string(i));
            }

            // Delete items from the list.
            size_t step = 1;
            for (size_t i = 0; i < nData; i += step)
            {
                data.at(i).Free();
                indices.erase(i);
                ++step;
            }

            for (size_t i = 0; i < nData; ++i)
            {
                if (indices.find(i) != indices.end())
                {
                    // Validate that non-deleted items still point to same memory locations.
                    REQUIRE(data.at(i)->Str == "Data " + std::to_string(i));
                }
                else
                {
                    // Place new memory in freed memory locations.
                    data.at(i) = rMemoryManager->Allocate<Data>();
                    data.at(i)->Str = "NewData " + std::to_string(i);
                }
            }

            for (size_t i = 0; i < nData; ++i)
            {
                if (indices.find(i) != indices.end())
                {
                    REQUIRE(data.at(i)->Str == "Data " + std::to_string(i));
                }
                else
                {
                    REQUIRE(data.at(i)->Str == "NewData " + std::to_string(i));
                }
            }

            data.clear();
            REQUIRE(rMemoryManager->GetNumBlocks() == 0);
        };

        CALL(RunTest, cb);
    }
}}