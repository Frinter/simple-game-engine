#pragma once

#include <vector>

#include "command.hh"
#include "condition.hh"
#include "types.hh"

class ConditionHandler
{
public:
    class StateHandler
    {
    public:
        StateHandler(Condition *_condition, Command *_command)
            : condition(_condition), command(_command)
        {
        }

        Condition *condition;
        Command *command;
    };

public:
    ConditionHandler()
    {
    }

    void Update()
    {
        for (int i = 0; i < _handlers.size(); ++i)
        {
            StateHandler *handler = &_handlers[i];

            if (handler->condition->Check())
                handler->command->Execute();
        }
    }

    void SetHandler(Condition *condition, Command *command)
    {
        _handlers.push_back(StateHandler(condition, command));
    }

    void RemoveHandler(Condition *condition, Command *command)
    {
        for (int i = 0; i < _handlers.size(); ++i)
        {
            StateHandler *handler = &_handlers[i];

            if (handler->condition == condition && handler->command == command)
            {
                _handlers.erase(_handlers.begin() + i);
                return;
            }
        }
    }

    void Clear()
    {
        _handlers.clear();
    }

private:
    std::vector<StateHandler> _handlers;
};
