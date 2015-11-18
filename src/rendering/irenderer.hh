#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

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
    virtual ~IRenderer() {}

    virtual void Render(const std::vector<IndexValue> &indices,
                        const std::vector<float> &vertices,
                        const std::vector<float> &normals,
                        const std::vector<float> &UVs,
                        const IndexValue &materialId) = 0;

    virtual void Use() = 0;
    virtual IndexValue RegisterMaterial(MaterialInfo material) = 0;

    virtual void SetModelMatrix(const glm::mat4 &matrix) = 0;
    virtual void SetViewMatrix(const glm::mat4 &matrix) = 0;
    virtual void SetProjectionMatrix(const glm::mat4 &matrix) = 0;
};
