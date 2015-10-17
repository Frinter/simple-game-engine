#pragma once

#include "sleepservice.hh"
#include "systemtimer.hh"

class Ticker
{
    typedef unsigned int TickValue;
    
public:
    Ticker(ISystemTimer *systemTimer, ISleepService *sleepService);

    void Start(unsigned int tickLengthInMilliseconds);
    void WaitUntilNextTick();

private:
    ISystemTimer *m_systemTimer;
    ISleepService *m_sleepService;

    TickValue _ticks;
    TickValue _tickLength;
};
