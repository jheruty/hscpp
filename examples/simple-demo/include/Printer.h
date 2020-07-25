#pragma once

#include "hscpp/Register.h"

static const char key[] = "Printer";

//constexpr std::string_view sv = "Printer";
//static const char key2[] = sv;

class Printer
{
    HSCPP_TRACK(Printer, "Printer");

public:
    Printer();
    virtual void Update();

private:
    int value = 10;
};