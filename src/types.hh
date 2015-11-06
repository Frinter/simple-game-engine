#pragma once

#include <glm/glm.hpp>

typedef unsigned int IndexValue;

typedef struct LightInfo
{
    glm::vec4 position;
    glm::vec3 La;
    glm::vec3 Ld;
    glm::vec3 Ls;
} LightInfo;

typedef struct MaterialInfo
{
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    float shininess;
} MaterialInfo;
