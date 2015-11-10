#pragma once

#include <GL/gl_core_3_3.h>
#include <glm/glm.hpp>

typedef unsigned int IndexValue;

typedef unsigned char *ImageData;

typedef struct RawImageInfo
{
    ImageData data;
    unsigned int width;
    unsigned int height;
    GLuint components;
} RawImageInfo;

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
    RawImageInfo *Kd_imageInfo;
    GLuint Kd_mapId;
    bool hasKdMap;
} MaterialInfo;
