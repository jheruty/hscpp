#pragma once

#include <vector>
#include <unordered_set>
#include <algorithm>

#include "catch/catch.hpp"

namespace hscpp { namespace test {

    template <typename T>
    void ValidateOrderedVector(const std::vector<T>& result, const std::vector<T>& expected)
    {
        REQUIRE(expected.size() == result.size());

        size_t nElements = expected.size();
        for (size_t i = 0; i < nElements; ++i)
        {
            REQUIRE(expected.at(i) == result.at(i));
        }
    }

    template <typename T, typename THasher = std::hash<T>>
    void ValidateUnorderedVector(const std::vector<T>& result, const std::vector<T>& expected)
    {
        REQUIRE(expected.size() == result.size());
        REQUIRE(std::is_permutation(result.begin(), result.end(), expected.begin(), expected.end()));
    }

}}