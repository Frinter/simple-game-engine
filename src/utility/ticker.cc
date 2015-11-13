#include <iostream>

#include "sleepservice.hh"
#include "systemtimer.hh"
#include "ticker.hh"

Ticker::Ticker(ISystemTimer *systemTimer, ISleepService *sleepService)
{
    m_systemTimer = systemTimer;
    m_sleepService = sleepService;
}

void Ticker::Start(unsigned int tickLengthInMilliseconds)
{
    _ticks = m_systemTimer->GetTicks();
    _tickLength = tickLengthInMilliseconds;
}

void Ticker::WaitUntilNextTick()
{
    unsigned int currentTicks = m_systemTimer->GetTicks();
    unsigned int targetTicks = _ticks + _tickLength;
	
    if (targetTicks > currentTicks)
    {
        unsigned int sleepTime = targetTicks - currentTicks;
        m_sleepService->Sleep(sleepTime);
        _ticks = targetTicks;
    }
    else
    {
        _ticks = targetTicks;
    }
}
