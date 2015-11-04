#pragma once

#include "types.hh"

namespace ObjParser
{
    typedef struct ColorValue
    {
        float red, green, blue;
    } ColorValue;

    typedef struct Material
    {
        std::string name;
        ColorValue ambientColor;
        ColorValue diffuseColor;
        ColorValue specularColor;
    } Material;

    typedef struct Vertex
    {
        float coordinates[4];
    } Vertex;

    typedef struct Normal
    {
        float coordinates[3];
    } Normal;

    typedef struct Face
    {
        std::vector<IndexValue> vertexIndices;
        std::vector<IndexValue> normalIndices;
        Material *material;
    } Face;
}
