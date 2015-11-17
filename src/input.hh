#pragma once

#include <framework/platform.hh>

class ButtonState
{
public:
    ButtonState(System::KeyCode key, Framework::KeyState state)
        : _key(key), _state(state)
    {
    }

    System::KeyCode GetKey() const
    {
        return _key;
    }

    Framework::KeyState GetState() const
    {
        return _state;
    }

private:
    System::KeyCode _key;
    Framework::KeyState _state;
};

class InputCondition
{
public:
    virtual ~InputCondition() {}
    virtual bool Check() = 0;
};

class KeyboardInputCondition : public InputCondition
{
public:
    KeyboardInputCondition(Framework::ReadingKeyboardState *keyboardState,
                           const ButtonState &buttonState)
        : _keyboardState(keyboardState), _buttonState(buttonState)
    {
    }

    bool Check()
    {
        return _keyboardState->GetKeyState(_buttonState.GetKey()) == _buttonState.GetState();
    }

private:
    Framework::ReadingKeyboardState *_keyboardState;
    ButtonState _buttonState;
};
