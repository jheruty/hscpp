#pragma once

#include "hscpp/Swappable.h"

class Printer : public hscpp::Swappable<Printer>
{
    virtual void Update();
};