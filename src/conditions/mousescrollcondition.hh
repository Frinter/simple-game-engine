#pragma once

#include "condition.hh"
#include "mousetracker.hh"

class MouseScrollCondition : public Condition
{
public:
    MouseScrollCondition(MouseTracker *mouseTracker)
        : _mouseTracker(mouseTracker)
    {
    }

    bool Check()
    {
        return _mouseTracker->GetScrollDelta() != 0;
    }

private:
    MouseTracker *_mouseTracker;
};
