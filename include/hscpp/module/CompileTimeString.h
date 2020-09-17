#pragma once

#include <array>
#include <cstdint>

namespace hscpp { namespace compile_time {

    // Simple holder of constexpr integral, allowing Stringlen() calls to be cached.
    template <int Len>
    struct KeylenCache
    {
        static_assert(Len <= 128, "Exceeds maximum key length of 128 bytes.");
        static constexpr int len = Len;
    };

    // String stored in integrals; each byte in each uint64_t corresponds to a char.
    // Maximum key length is capped at 128 (16 * sizeof(uint64_t)).
    template <uint64_t S1, uint64_t S2, uint64_t S3, uint64_t S4, uint64_t S5, uint64_t S6, uint64_t S7, uint64_t S8,
            uint64_t S9, uint64_t S10, uint64_t S11, uint64_t S12, uint64_t S13, uint64_t S14, uint64_t S15, uint64_t S16>
    struct String
    {
        std::string ToString() const
        {
            // Max length is 128, leave room for null terminator.
            std::array<char, 129> str = {};
            constexpr std::array<uint64_t, 16> segments = {
                    S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14, S15, S16 };

            size_t iTotalOffset = 0;
            for (uint64_t segment : segments)
            {
                for (int i = 0; i < 8; ++i)
                {
                    char c = static_cast<char>((((segment >> (i * 8u)) & 0xffu)));
                    if (c == 0)
                    {
                        return str.data();
                    }
                    else
                    {
                        str.at(iTotalOffset) = c;
                    }

                    ++iTotalOffset;
                }
            }

            return str.data();
        }
    };

    constexpr int Strlen(const char* pStr)
    {
        return (*pStr == 0)
            ? 0
            : 1 + Strlen(pStr + 1);
    }

    constexpr uint64_t StringSegmentToIntegral(const char* pStr, int iStartByte, int iByte, int len)
    {
        // Stop recursion if either:
        // a) Filled out entire uint64_t.
        // b) Hit end of string.
        return (iByte >= 8 || iStartByte + iByte >= len)
            ? 0
            : StringSegmentToIntegral(pStr, iStartByte, iByte + 1, len)
                + (static_cast<uint64_t>(pStr[iStartByte + iByte]) << (iByte * 8u));
    }

    constexpr uint64_t StringSegmentToIntegral(const char* pStr, int iUint64, int len)
    {
        return StringSegmentToIntegral(pStr, iUint64 * 8, 0, len);
    }

}}