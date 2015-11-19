#pragma once

#include "condition.hh"
#include "mousetracker.hh"

class MouseMovementCondition : public Condition
{
public:
    MouseMovementCondition(MouseTracker *tracker)
        : _tracker(tracker)
    {
    }

    bool Check()
    {
        int deltaX, deltaY;

        _tracker->GetDeltaPosition(&deltaX, &deltaY);

        return deltaX != 0 || deltaY != 0;
    }

private:
    MouseTracker *_tracker;
};
