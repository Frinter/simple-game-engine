#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "GL/gl_core_3_3.h"

#include "adsrenderer.hh"
#include "objparser.hh"
#include "types.hh"

class ObjImporter : public ADSRenderer::Importer
{
public:
    ObjImporter(ObjParser::IParseResult *result)
        : _parseResult(result)
    {
    }

    std::vector<float> GetVertices()
    {
        translateAllVertices();
        return _vertices;
    }

    std::vector<float> GetNormals()
    {
        translateAllVertices();
        return _normals;
    }

    std::vector<float> GetUVCoords()
    {
        translateAllVertices();
        return _uvCoords;
    }

    std::vector<IndexValue> GetIndices()
    {
        std::vector<IndexValue> processedIndices;
        std::vector<ObjParser::Face> faces = _parseResult->GetFaces();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];

            for (int j = 0; j < face.vertexIndices.size(); ++j)
            {
                if (j >= 3)
                {
                    processedIndices.push_back(face.vertexIndices.size()*i + j - 3);
                    processedIndices.push_back(face.vertexIndices.size()*i + j - 1);
                }
                processedIndices.push_back(face.vertexIndices.size()*i + j);
            }
        }
        return processedIndices;
    }

    MaterialInfo GetMaterial(const char *name);

private:
    ObjParser::IParseResult *_parseResult;

    std::vector<float> _vertices;
    std::vector<float> _normals;
    std::vector<float> _uvCoords;

private:
    void translateAllVertices()
    {
        _vertices.clear();
        _normals.clear();
        std::vector<ObjParser::Face> faces = _parseResult->GetFaces();
        std::vector<ObjParser::Vertex> vertices = _parseResult->GetVertices();
        std::vector<ObjParser::Normal> normals = _parseResult->GetNormals();
        std::vector<ObjParser::UVCoord> uvCoords = _parseResult->GetUVCoords();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];
            for (int j = 0; j < face.normalIndices.size(); ++j)
            {
                if (j >= 3)
                {
                    addUVCoord(uvCoords[face.UVIndices[j-3]]);
                    addNormal(normals[face.normalIndices[j-3]]);
                    addVertex(vertices[face.vertexIndices[j-3]]);
                    addUVCoord(uvCoords[face.UVIndices[j-1]]);
                    addNormal(normals[face.normalIndices[j-1]]);
                    addVertex(vertices[face.vertexIndices[j-1]]);
                }

                addUVCoord(uvCoords[face.UVIndices[j]]);
                addNormal(normals[face.normalIndices[j]]);
                addVertex(vertices[face.vertexIndices[j]]);
            }
        }
    }

    void addUVCoord(ObjParser::UVCoord uvCoord)
    {
        _uvCoords.push_back(uvCoord.coordinates[0]);
        _uvCoords.push_back(uvCoord.coordinates[1]);
    }

    void addVertex(ObjParser::Vertex vertex)
    {
        for (int i = 0; i < 4; ++i)
        {
            _vertices.push_back(vertex.coordinates[i]);
        }
    }

    void addNormal(ObjParser::Normal normal)
    {
        for (int i = 0; i < 3; ++i)
        {
            _normals.push_back(normal.coordinates[i]);
        }
    }

    MaterialInfo translateMaterial(ObjParser::Material *material);

    glm::vec3 translateColor(ObjParser::ColorValue color)
    {
        return glm::vec3(color.red, color.green, color.blue);
    }

    glm::vec3 translateNormal(ObjParser::Normal vertex)
    {
        return glm::vec3(vertex.coordinates[0], vertex.coordinates[1], vertex.coordinates[2]);
    }

    glm::vec4 translateVertex(ObjParser::Vertex vertex)
    {
        return glm::vec4(vertex.coordinates[0], vertex.coordinates[1], vertex.coordinates[2], vertex.coordinates[3]);
    }
};
