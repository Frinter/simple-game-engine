#pragma once

#include <string>
#include <vector>

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
        float specularExponent;
        std::string diffuseMap;
    } Material;

    typedef struct Vertex
    {
        float coordinates[4];
    } Vertex;

    typedef struct Normal
    {
        float coordinates[3];
    } Normal;

    typedef struct UVCoord
    {
        float coordinates[2];
    } UVCoords;

    typedef struct Face
    {
        std::vector<IndexValue> vertexIndices;
        std::vector<IndexValue> normalIndices;
        std::vector<IndexValue> UVIndices;
        std::string material;
    } Face;
}
