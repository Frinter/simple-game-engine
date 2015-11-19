#pragma once

class Moveable
{
public:
    virtual ~Moveable() {}
    virtual void Move(float x, float y, float z) = 0;
    virtual void Rotate(float radians) = 0;
};
