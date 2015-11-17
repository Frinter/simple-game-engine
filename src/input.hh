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

class MouseButtonState
{
public:
    MouseButtonState(System::MouseButton button, Framework::KeyState state)
        : _button(button), _state(state)
    {
    }

    System::MouseButton GetButton() const
    {
        return _button;
    }

    Framework::KeyState GetState() const
    {
        return _state;
    }

private:
    System::MouseButton _button;
    Framework::KeyState _state;
};

class InputCondition
{
public:
    virtual ~InputCondition() {}
    virtual bool Check() = 0;
};

class MultiConditionChecker : public InputCondition
{
public:
    bool Check()
    {
        for (int i = 0; i < _conditions.size(); ++i)
        {
            if (_conditions[i]->Check() == false)
                return false;
        }

        return true;
    }

    void AddCondition(InputCondition *condition)
    {
        _conditions.push_back(condition);
    }

private:
    std::vector<InputCondition*> _conditions;
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

class MouseInputCondition : public InputCondition
{
public:
    MouseInputCondition(Framework::ReadingMouseState *mouseState,
                        const MouseButtonState &buttonState)
        : _mouseState(mouseState), _buttonState(buttonState)
    {
    }

    bool Check()
    {
        return _mouseState->GetMouseButtonState(_buttonState.GetButton()) == _buttonState.GetState();
    }

private:
    Framework::ReadingMouseState *_mouseState;
    MouseButtonState _buttonState;
};
