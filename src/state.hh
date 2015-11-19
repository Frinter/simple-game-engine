#pragma once

class State
{
public:
    virtual ~State() {}
    virtual void Enter() = 0;
};
