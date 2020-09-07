#pragma once

class IUpdatable
{
public:
    virtual ~IUpdatable() {};
    virtual void Update() = 0;
};