#pragma once

#include <framework/platform.hh>
#include "conditionhandler.hh"
#include "conditions/mousemovementcondition.hh"
#include "conditions/mousescrollcondition.hh"
#include "commands/movecommand.hh"
#include "commands/rotatefrommousemovementcommand.hh"
#include "commands/zoomfrommousecommand.hh"
#include "input.hh"
#include "mousetracker.hh"
#include "moveable.hh"
#include "state.hh"

class TestInputHandlerState : public State
{
public:
    TestInputHandlerState(Framework::ReadingKeyboardState *keyboardState,
                          Framework::ReadingMouseState *mouseState,
                          MouseTracker *mouseTracker,
                          ConditionHandler *handler, Moveable *moveable,
                          Camera *camera);

    void Enter();

private:
    ConditionHandler *_handler;
    Moveable *_moveable;
    Camera *_camera;

    MoveCommand _moveUp;
    MoveCommand _moveDown;
    RotateFromMouseMovementCommand _rotateFromMouse;
    ZoomFromMouseCommand _zoomFromMouse;

    KeyboardInputCondition _upButtonDown;
    KeyboardInputCondition _downButtonDown;
    MouseInputCondition _mouseButtonDown;
    MouseMovementCondition _mouseMovement;
    MouseScrollCondition _mouseScroll;

    MultiConditionChecker _rotateCameraCondition;
};
