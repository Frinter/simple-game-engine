#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "objparser.hh"
#include "types.hh"

class ObjImporter
{
public:
    ObjImporter(ObjParser::IParseResult *result)
        : _parseResult(result)
    {
    }

    std::vector<float> GetVertices()
    {
        std::vector<float> processedVertices;
        std::vector<ObjParser::Face> faces = _parseResult->GetFaces();
        std::vector<ObjParser::Vertex> vertices = _parseResult->GetVertices();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];
            for (int ii = 0; ii < face.vertexIndices.size(); ++ii)
            {
                ObjParser::Vertex vertex = vertices[face.vertexIndices[ii]];
                for (int j = 0; j < 4; ++j)
                {
                    processedVertices.push_back(vertex.coordinates[j]);
                }
            }
        }

        return processedVertices;
    }

    std::vector<float> GetNormals()
    {
        std::vector<float> processedNormals;
        std::vector<ObjParser::Face> faces = _parseResult->GetFaces();
        std::vector<ObjParser::Normal> normals = _parseResult->GetNormals();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];
            for (int j = 0; j < face.normalIndices.size(); ++j)
            {
                ObjParser::Normal normal = normals[face.normalIndices[j]];
                for (int jj = 0; jj < 3; ++jj)
                {
                    processedNormals.push_back(normal.coordinates[jj]);
                }
            }
        }

        return processedNormals;
    }

    std::vector<IndexValue> GetIndices()
    {
        std::vector<IndexValue> processedIndices;
        std::vector<ObjParser::Face> faces = _parseResult->GetFaces();

        for (int i = 0; i < faces.size(); ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                processedIndices.push_back(3*i + j);
            }
        }
        return processedIndices;
    }

    MaterialInfo GetMaterial(const char *name)
    {
        std::vector<ObjParser::Material*> materials = _parseResult->GetMaterials();
        ObjParser::Material *material = NULL;

        for (int i = 0; i < materials.size(); ++i)
        {
            if (materials[i]->name == name)
            {
                return translateMaterial(materials[i]);
            }
        }

        std::stringstream errorStream;
        errorStream << "Runtime error: unable to find material in parse result: " << name;
        throw std::runtime_error(errorStream.str());
    }

private:
    ObjParser::IParseResult *_parseResult;

private:
    MaterialInfo translateMaterial(ObjParser::Material *material)
    {
        MaterialInfo info;

        info.Ka = translateColor(material->ambientColor);
        info.Kd = translateColor(material->diffuseColor);
        info.Ks = translateColor(material->specularColor);
        info.shininess = 0.5;

        return info;
    }

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
