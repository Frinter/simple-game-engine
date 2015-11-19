#pragma once

#include <map>

#include <framework/platform.hh>

class MouseTracker
{
private:
public:
    MouseTracker(Framework::ReadingMouseState *mouseReader)
        : _mouseState(mouseReader), _scrollDelta(0)
    {
        _mouseCurrentX = _mouseState->GetMouseX();
        _mouseCurrentY = _mouseState->GetMouseY();

        _previousButtonState[System::MouseButton::Button2] = Framework::KeyState::Unpressed;
        _currentButtonState[System::MouseButton::Button2] = Framework::KeyState::Unpressed;
    }

    void GetPosition(int *mouseX, int *mouseY)
    {
        *mouseX = _mouseCurrentX;
        *mouseY = _mouseCurrentY;
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

    bool IsButtonDownFrame(System::MouseButton button) const
    {
        return _previousButtonState.at(button) == Framework::KeyState::Unpressed &&
            _currentButtonState.at(button) == Framework::KeyState::Pressed;
    }

    bool IsButtonUpFrame(System::MouseButton button) const
    {
        return _previousButtonState.at(button) == Framework::KeyState::Pressed &&
            _currentButtonState.at(button) == Framework::KeyState::Unpressed;
    }

    void ResetPreviousPosition()
    {
        _mousePreviousX = _mouseCurrentX;
        _mousePreviousY = _mouseCurrentY;

        _mouseCurrentX = _mouseState->GetMouseX();
        _mouseCurrentY = _mouseState->GetMouseY();
    }

    void update()
    {
        ResetPreviousPosition();

        _previousButtonState[System::MouseButton::Button2] = _currentButtonState[System::MouseButton::Button2];
        _currentButtonState[System::MouseButton::Button2] = _mouseState->GetMouseButtonState(System::MouseButton::Button2);

        _scrollDelta = _mouseState->GetScrollDelta();

        _mouseDeltaX = _mouseCurrentX - _mousePreviousX;
        _mouseDeltaY = _mouseCurrentY - _mousePreviousY;
    }

private:
    Framework::ReadingMouseState *_mouseState;

    unsigned int _mousePreviousX, _mousePreviousY;
    unsigned int _mouseCurrentX, _mouseCurrentY;
    int _mouseDeltaX, _mouseDeltaY;
    int _scrollDelta;

    std::map<System::MouseButton, Framework::KeyState> _currentButtonState;
    std::map<System::MouseButton, Framework::KeyState> _previousButtonState;
};
