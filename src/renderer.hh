#pragma once

class IRenderer
{
public:
    virtual void Use() = 0;
    virtual void Render(const Model &model) = 0;
};
