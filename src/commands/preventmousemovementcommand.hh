#pragma once

#include <framework/platform.hh>

#include "command.hh"

class PreventMouseMovementCommand : public Command
{
public:
    PreventMouseMovementCommand(Framework::IWindowController *windowController,
                                MouseTracker *mouseTracker,
                                int x, int y)
        : _windowController(windowController), _mouseTracker(mouseTracker),
          _lockedX(x), _lockedY(y)
    {
    }

    void Execute()
    {
        _windowController->SetMousePosition(_lockedX, _lockedY);
        _mouseTracker->ResetPreviousPosition();
    }

private:
    Framework::IWindowController *_windowController;
    MouseTracker *_mouseTracker;
    int _lockedX, _lockedY;
};
