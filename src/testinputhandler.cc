#include "testinputhandler.hh"

using Framework::ReadingKeyboardState;
using Framework::ReadingMouseState;

TestInputHandlerState::TestInputHandlerState(ReadingKeyboardState *keyboardState,
                                             ReadingMouseState *mouseState,
                                             MouseTracker *mouseTracker,
                                             ConditionHandler *handler,
                                             Moveable *moveable,
                                             Camera *camera)
    : _handler(handler), _moveable(moveable), _camera(camera),
      _moveUp(_moveable, 0.0f, 0.1f, 0.0f),
      _moveDown(_moveable, 0.0f, -0.1f, 0.0f),
      _rotateFromMouse(_camera, mouseTracker),
      _zoomFromMouse(camera, mouseTracker),
      _upButtonDown(keyboardState, ButtonState(System::KeyCode::KeyUpArrow,
                                               Framework::KeyState::Pressed)),
      _downButtonDown(keyboardState, ButtonState(System::KeyCode::KeyDownArrow,
                                                 Framework::KeyState::Pressed)),
      _mouseButtonDown(mouseState, MouseButtonState(System::MouseButton::Button2,
                                                    Framework::KeyState::Pressed)),
      _mouseMovement(mouseTracker),
      _mouseScroll(mouseTracker)
{
    _rotateCameraCondition.AddCondition(&_mouseButtonDown);
    _rotateCameraCondition.AddCondition(&_mouseMovement);
}

void TestInputHandlerState::Enter()
{
    _handler->Clear();
    _handler->SetHandler(&_upButtonDown, &_moveUp);
    _handler->SetHandler(&_downButtonDown, &_moveDown);
    _handler->SetHandler(&_rotateCameraCondition, &_rotateFromMouse);
    _handler->SetHandler(&_mouseScroll, &_zoomFromMouse);
}
