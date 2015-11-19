#pragma once

#include "command.hh"
#include "constants.hh"
#include "mousetracker.hh"
#include "moveable.hh"

class RotateFromMouseMovementCommand : public Command
{
public:
    RotateFromMouseMovementCommand(Moveable *moveable, MouseTracker *tracker)
        : _moveable(moveable), _tracker(tracker)
    {
    }

    void Execute()
    {
        int deltaX, deltaY;
        _tracker->GetDeltaPosition(&deltaX, &deltaY);
        _moveable->Rotate(((float)deltaX / 10) * (PI / 180));
    }

private:
    Moveable *_moveable;
    MouseTracker *_tracker;
};
