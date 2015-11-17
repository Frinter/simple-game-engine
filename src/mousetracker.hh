#pragma once

class MouseTracker
{
public:
    MouseTracker(Framework::IWindowController *windowController)
        : _windowController(windowController), _scrollDelta(0)
    {
        _mouseState = _windowController->GetMouseReader();
        _previousMouseButtonState = Framework::KeyState::Unpressed;
    }

    void GetDeltaPosition(int *deltaX, int *deltaY)
    {
        *deltaX = _mouseDeltaX;
        *deltaY = _mouseDeltaY;
    }

    int GetScrollDelta() const
    {
        return _scrollDelta;
    }

    void update()
    {
        Framework::KeyState mouseButtonState = _mouseState->GetMouseButtonState(System::MouseButton::Button2);
        unsigned int mouseX = _mouseState->GetMouseX();
        unsigned int mouseY = _mouseState->GetMouseY();

        _scrollDelta = _mouseState->GetScrollDelta();

        if (mouseButtonState == Framework::KeyState::Pressed)
        {
            if (_previousMouseButtonState == Framework::KeyState::Unpressed)
            {
                _mouseLockX = mouseX;
                _mouseLockY = mouseY;
            }

            if (mouseX != _mouseLockX || mouseY != _mouseLockY)
            {
                _windowController->SetMousePosition(_mouseLockX, _mouseLockY);

                _mouseDeltaX = mouseX - _mouseLockX;
                _mouseDeltaY = mouseY - _mouseLockY;

                mouseX = _mouseState->GetMouseX();
                mouseY = _mouseState->GetMouseY();
            }
            else
            {
                _mouseDeltaX = 0;
                _mouseDeltaY = 0;
            }
        }
        else
        {
            _mouseDeltaX = mouseX - _mousePreviousX;
            _mouseDeltaY = mouseY - _mousePreviousY;

            _mousePreviousX = mouseX;
            _mousePreviousY = mouseY;
        }

        _previousMouseButtonState = mouseButtonState;
    }

private:
    Framework::IWindowController *_windowController;
    Framework::ReadingMouseState *_mouseState;

    Framework::KeyState _previousMouseButtonState;
    unsigned int _mouseLockX, _mouseLockY;
    unsigned int _mousePreviousX, _mousePreviousY;
    int _mouseDeltaX, _mouseDeltaY;
    int _scrollDelta;
};
