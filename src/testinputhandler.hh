#pragma once

#include <framework/platform.hh>
#include "conditionhandler.hh"
#include "conditions/mousemovementcondition.hh"
#include "conditions/mousescrollcondition.hh"
#include "commands/preventmousemovementcommand.hh"
#include "commands/movecommand.hh"
#include "commands/rotatefrommousemovementcommand.hh"
#include "commands/zoomfrommousecommand.hh"
#include "input.hh"
#include "mousetracker.hh"
#include "moveable.hh"
#include "state.hh"

class MouseButtonDownStateChangeCondition : public Condition
{
public:
    MouseButtonDownStateChangeCondition(MouseTracker *mouseTracker,
                                        System::MouseButton button)
        : _mouseTracker(mouseTracker), _button(button)
    {
    }

    bool Check()
    {
        return _mouseTracker->IsButtonDownFrame(_button);
    }

private:
    MouseTracker *_mouseTracker;
    System::MouseButton _button;
};

class MouseButtonUpStateChangeCondition : public Condition
{
public:
    MouseButtonUpStateChangeCondition(MouseTracker *mouseTracker,
                                      System::MouseButton button)
        : _mouseTracker(mouseTracker), _button(button)
    {
    }

    bool Check()
    {
        return _mouseTracker->IsButtonUpFrame(_button);
    }

private:
    MouseTracker *_mouseTracker;
    System::MouseButton _button;
};

class TestInputHandlerState : public State
{
private:
    class LockMouseCommand : public Command
    {
    public:
        LockMouseCommand(TestInputHandlerState *inputHandler)
            : _inputHandler(inputHandler)
        {
        }

        void Execute()
        {
            _inputHandler->LockMouse();
        }

    private:
        TestInputHandlerState *_inputHandler;
    };

    class UnlockMouseCommand : public Command
    {
    public:
        UnlockMouseCommand(TestInputHandlerState *inputHandler)
            : _inputHandler(inputHandler)
        {
        }

        void Execute()
        {
            _inputHandler->UnlockMouse();
        }

    private:
        TestInputHandlerState *_inputHandler;
    };

public:
    TestInputHandlerState(Framework::IWindowController *windowController,
                          Framework::ReadingKeyboardState *keyboardState,
                          Framework::ReadingMouseState *mouseState,
                          MouseTracker *mouseTracker,
                          ConditionHandler *handler, Moveable *moveable,
                          Camera *camera);

    void Enter();

    void LockMouse();
    void UnlockMouse();

private:
    ConditionHandler *_handler;
    Framework::IWindowController *_windowController;
    Moveable *_moveable;
    Camera *_camera;
    MouseTracker *_mouseTracker;

    MoveCommand _moveUp;
    MoveCommand _moveDown;
    PreventMouseMovementCommand *_preventMouseMovement;
    RotateFromMouseMovementCommand _rotateFromMouse;
    ZoomFromMouseCommand _zoomFromMouse;
    LockMouseCommand _lockMouseCommand;
    UnlockMouseCommand _unlockMouseCommand;

    KeyboardInputCondition _upButtonDown;
    KeyboardInputCondition _downButtonDown;
    MouseInputCondition _mouseButtonDown;
    MouseMovementCondition _mouseMovement;
    MouseScrollCondition _mouseScroll;
    MouseButtonDownStateChangeCondition _mouseButtonDownChange;
    MouseButtonUpStateChangeCondition _mouseButtonUpChange;

    MultiConditionChecker _rotateCameraCondition;
};
