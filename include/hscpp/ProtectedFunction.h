#pragma once

#include <functional>

class ProtectedFunction
{
public:
    enum class Result
    {
        Success,
        Exception,
    };

    static Result Call(const std::function<void()>& cb);
};