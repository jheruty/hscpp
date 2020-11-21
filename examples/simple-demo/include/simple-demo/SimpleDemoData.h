#pragma once

#include <vector>

#include "Printer.h"

struct SimpleDemoData
{
    std::vector<Printer*> printers = { nullptr, nullptr };
};