#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "model.hh"
#include "types.hh"

typedef struct RenderObject
{
    std::vector<IndexValue> _indices;
    std::vector<float> _vertices;
    std::vector<float> _normals;
    std::vector<float> _uvCoords;
    std::string _materialName;
} RenderObject;

class IRenderer
{
public:
    class Importer
    {
    public:
        virtual ~Importer() {}
        virtual MaterialInfo GetMaterial(const std::string &name) = 0;
        virtual std::vector<RenderObject> GetRenderObjects() = 0;
    };

public:
    virtual void Use() = 0;
    virtual void Render(const Model *model) = 0;
    virtual void Render(std::vector<IndexValue> indices, std::vector<float> vertices, std::vector<float> normals, std::vector<float> UVs) = 0;

    virtual IndexValue RegisterMaterial(MaterialInfo material) = 0;
    virtual void UseMaterial(IndexValue materialId) = 0;

    virtual void SetModelMatrix(const glm::mat4 &matrix) = 0;
    virtual void SetViewMatrix(const glm::mat4 &matrix) = 0;
    virtual void SetProjectionMatrix(const glm::mat4 &matrix) = 0;

    virtual Model *CreateModelFromImporter(Importer &importer) = 0;
};
