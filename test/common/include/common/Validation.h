#pragma once

#include <sstream>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "catch/catch.hpp"
#include "hscpp/preprocessor/LangError.h"

namespace hscpp { namespace test {

    template <typename T>
    std::string ToString(const std::vector<T>& vec)
    {
        std::stringstream ss;
        ss << "{\n";

        for (size_t i = 0; i < vec.size(); ++i)
        {
            ss << "\t" << vec.at(i);
            if (i != vec.size() - 1)
            {
                ss << ",";
            }

            ss << "\n";
        }

        ss << "}\n";
        return ss.str();
    }

    template <typename T>
    void ValidateOrderedVector(const std::vector<T>& result, const std::vector<T>& expected)
    {
        INFO("\nResult:\n" + ToString(result) + "\nExpected:\n" + ToString(expected));

        REQUIRE(expected.size() == result.size());

        size_t nElements = expected.size();
        for (size_t i = 0; i < nElements; ++i)
        {
            REQUIRE(expected.at(i) == result.at(i));
        }
    }

    template <typename T>
    void ValidateUnorderedVector(const std::vector<T>& result, const std::vector<T>& expected)
    {
        INFO("\nResult:\n" + ToString(result) + "\nExpected:\n" + ToString(expected));

        REQUIRE(expected.size() == result.size());
        REQUIRE(std::is_permutation(result.begin(), result.end(), expected.begin()));
    }

    void ValidateError(const LangError& result,
        const LangError::Code& expectedCode, size_t expectedLine, const std::vector<std::string>& expectedArgs);

}}