#include "dependent-compilation-demo/StringUtil.h"

hscpp_module("string")

std::string Concat(std::string a, std::string b)
{
    return a + b;
}
