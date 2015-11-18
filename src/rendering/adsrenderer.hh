#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "irenderer.hh"

class ADSRenderer : public IRenderer
{
public:
    ADSRenderer();
    ~ADSRenderer();

    void Use();
    IndexValue RegisterMaterial(MaterialInfo material);
    void SetModelMatrix(const glm::mat4 &matrix);
    void SetViewMatrix(const glm::mat4 &matrix);
    void SetProjectionMatrix(const glm::mat4 &matrix);
    void SetLight(LightInfo info);

    void Render(const std::vector<IndexValue> &indices,
                const std::vector<float> &vertices,
                const std::vector<float> &normals,
                const std::vector<float> &UVs,
                const IndexValue &materialId);

public:
    class IADSRendererImplementation
    {
    public:
        virtual ~IADSRendererImplementation() {}
        virtual void Use() = 0;
        virtual IndexValue RegisterMaterial(MaterialInfo material) = 0;
        virtual void SetModelMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetViewMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetProjectionMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;

        virtual void Render(const std::vector<IndexValue> &indices,
                        const std::vector<float> &vertices,
                        const std::vector<float> &normals,
                        const std::vector<float> &UVs,
                        const IndexValue &materialId) = 0;
    };

private:
    IADSRendererImplementation *_implementation;
};
