#pragma once

#include "framework/platform.hh"

class ISleepService
{
public:
    virtual void Sleep(unsigned int milliseconds) = 0;
};

class SleepService : public ISleepService
{
public:
    SleepService(System::Utility *utility)
        : m_utility(utility)
    {
    }
	
    virtual void Sleep(unsigned int milliseconds)
    {
        m_utility->Sleep(milliseconds);
    }
	
private:
    System::Utility *m_utility;
};
