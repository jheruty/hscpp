#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/module/SwapInfo.h"

namespace hscpp { namespace test
{

    TEST_CASE("SwapInfo can serialize and unserialize copyable items.")
    {
        int nBytes = 100;
        uint8_t* pMemory = new uint8_t[nBytes];

        struct Data
        {
            float float1 = 0;
            float float2 = 0;
        };

        Data data;
        data.float1 = -20;
        data.float2 = 7777;

        hscpp::SwapInfo info;

        info.Serialize("nBytes", nBytes);
        info.Serialize("pMemory", pMemory);
        info.Serialize("data", data);

        int nBytesUnserialized = 0;
        uint8_t* pMemoryUnserialized = nullptr;
        Data dataUnserialized;

        info.Unserialize("nBytes", nBytesUnserialized);
        info.Unserialize("pMemory", pMemoryUnserialized);
        info.Unserialize("data", dataUnserialized);

        REQUIRE(nBytes == nBytesUnserialized);
        REQUIRE(pMemory == pMemoryUnserialized);
        REQUIRE(data.float1 == dataUnserialized.float1);
        REQUIRE(data.float2 == dataUnserialized.float2);

        delete[] pMemory;
    }

    TEST_CASE("SwapInfo can serialize and unserialize movable items.")
    {
        struct InnerData
        {
            int int1;
        };

        struct Data
        {
            float float1 = 0;
            float float2 = 0;

            std::unique_ptr<InnerData> pInnerData;
        };

        auto pData = std::unique_ptr<Data>(new Data());
        pData->float1 = 55;
        pData->float2 = 66;
        pData->pInnerData = std::unique_ptr<InnerData>(new InnerData());
        pData->pInnerData->int1 = 77;

        hscpp::SwapInfo info;
        std::unique_ptr<Data> pDataUnserialized;

        info.SerializeMove("pData", std::move(pData));
        info.UnserializeMove("pData", pDataUnserialized);

        REQUIRE(pDataUnserialized->float1 == 55);
        REQUIRE(pDataUnserialized->float2 == 66);
        REQUIRE(pDataUnserialized->pInnerData->int1 == 77);
    }

}}