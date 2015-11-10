#pragma once

#include "irenderer.hh"

class Entity
{
public:
    virtual void update() = 0;
};

// class GraphicsComponent
// {
// public:
//     virtual void update(const Entity *entity, IRenderer *renderer) = 0;
// };
