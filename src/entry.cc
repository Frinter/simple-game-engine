#include "framework/platform.hh"

static Framework::ApplicationState applicationState = {
    .windowName = "Bandit Camp"
};

GetApplicationState_FunctionSignature(GetApplicationState)
{
    return &applicationState;
}

LogicThreadEntry_FunctionSignature(LogicThreadEntry)
{
    Framework::ReadingKeyboardState *keyboardState = windowController->GetKeyStateReader();

    while (!applicationContext->IsClosing())
    {
        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }
    }
}
