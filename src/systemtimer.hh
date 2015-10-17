#pragma once

#include "framework/platform.hh"

class ISystemTimer
{
public:
    virtual unsigned int GetTicks() = 0;
};

class SystemTimer : public ISystemTimer
{
public:
    SystemTimer(System::Utility *utility)
        : m_utility(utility)
    {
    }

    virtual unsigned int GetTicks()
    {
        return m_utility->GetTicks();
    }

private:
    System::Utility *m_utility;
};
