#include "testinputhandler.hh"

using Framework::IWindowController;
using Framework::ReadingKeyboardState;
using Framework::ReadingMouseState;

TestInputHandlerState::TestInputHandlerState(IWindowController *windowController,
                                             ReadingKeyboardState *keyboardState,
                                             ReadingMouseState *mouseState,
                                             MouseTracker *mouseTracker,
                                             ConditionHandler *handler,
                                             Moveable *moveable,
                                             Camera *camera)
    : _handler(handler), _moveable(moveable), _camera(camera),
      _mouseTracker(mouseTracker), _windowController(windowController),
      _moveUp(_moveable, 0.0f, 0.1f, 0.0f),
      _moveDown(_moveable, 0.0f, -0.1f, 0.0f),
      _preventMouseMovement(NULL),
      _rotateFromMouse(_camera, mouseTracker),
      _zoomFromMouse(camera, mouseTracker),
      _lockMouseCommand(this),
      _unlockMouseCommand(this),
      _upButtonDown(keyboardState, ButtonState(System::KeyCode::KeyUpArrow,
                                               Framework::KeyState::Pressed)),
      _downButtonDown(keyboardState, ButtonState(System::KeyCode::KeyDownArrow,
                                                 Framework::KeyState::Pressed)),
      _mouseButtonDown(mouseState, MouseButtonState(System::MouseButton::Button2,
                                                    Framework::KeyState::Pressed)),
      _mouseMovement(mouseTracker),
      _mouseScroll(mouseTracker),
      _mouseButtonDownChange(mouseTracker, System::MouseButton::Button2),
      _mouseButtonUpChange(mouseTracker, System::MouseButton::Button2)
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

    _handler->SetHandler(&_mouseButtonDownChange, &_lockMouseCommand);
    _handler->SetHandler(&_mouseButtonUpChange, &_unlockMouseCommand);
}

void TestInputHandlerState::LockMouse()
{
    UnlockMouse();
    int mouseX, mouseY;
    _mouseTracker->GetPosition(&mouseX, &mouseY);
    _preventMouseMovement = new PreventMouseMovementCommand(_windowController,
                                                            _mouseTracker,
                                                            mouseX, mouseY);
    _handler->SetHandler(&_mouseMovement, _preventMouseMovement);
}

void TestInputHandlerState::UnlockMouse()
{
    if (_preventMouseMovement != NULL)
    {
        _handler->RemoveHandler(&_mouseMovement, _preventMouseMovement);
        delete _preventMouseMovement;
        _preventMouseMovement = NULL;
    }
}
