#include "framework/platform.hh"
#include "ticker.hh"

static Framework::ApplicationState applicationState = {
    .windowName = "Rendering Engine"
};

GetApplicationState_FunctionSignature(GetApplicationState)
{
    return &applicationState;
}

LogicThreadEntry_FunctionSignature(LogicThreadEntry)
{
    SystemTimer systemTimer(applicationContext->GetSystemUtility());
    SleepService sleepService(applicationContext->GetSystemUtility());
    Ticker ticker = Ticker(&systemTimer, &sleepService);

    Framework::ReadingKeyboardState *keyboardState = windowController->GetKeyStateReader();

    ticker.Start(5);
        
    while (!applicationContext->IsClosing())
    {
        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }

        ticker.WaitUntilNextTick();
    }
}
