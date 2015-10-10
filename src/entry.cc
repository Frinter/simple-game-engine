#include "framework/platform.hh"

static Framework::ApplicationState applicationState = {
    .windowName = "Bandit Camp"
};

GetApplicationState_FunctionSignature(GetApplicationState)
{
    return &applicationState;
}

GraphicsThreadEntry_FunctionSignature(GraphicsThreadEntry)
{
}

LogicThreadEntry_FunctionSignature(LogicThreadEntry)
{
}
