#pragma once

#include "command.hh"
#include "moveable.hh"

class MoveCommand : public Command
{
public:
    MoveCommand(Moveable *moveable, float x, float y, float z)
        : _moveable(moveable), _x(x), _y(y), _z(z)
    {
    }

    void Execute()
    {
        _moveable->Move(_x, _y, _z);
    }

private:
    Moveable *_moveable;
    float _x;
    float _y;
    float _z;
};
