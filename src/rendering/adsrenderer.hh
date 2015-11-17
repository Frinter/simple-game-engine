#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "irenderer.hh"
#include "model.hh"

class ADSRenderer : public IRenderer
{
public:
    ADSRenderer();
    ~ADSRenderer();
    
    void Use();
    Model *CreateModelFromImporter(IRenderer::Importer &importer);
    IndexValue RegisterMaterial(MaterialInfo material);
    void UseMaterial(IndexValue materialId);
    void SetModelMatrix(const glm::mat4 &matrix);
    void SetViewMatrix(const glm::mat4 &matrix);
    void SetProjectionMatrix(const glm::mat4 &matrix);
    void SetLight(LightInfo info);

    void Render(const Model *model);
    void Render(std::vector<IndexValue> indices, std::vector<float> vertices, std::vector<float> normals, std::vector<float> UVs);

public:
    class IADSRendererImplementation
    {
    public:
        virtual ~IADSRendererImplementation() {}
        virtual void Use() = 0;
        virtual Model *CreateModelFromImporter(IRenderer::Importer &importer) = 0;
        virtual IndexValue RegisterMaterial(MaterialInfo material) = 0;
        virtual void UseMaterial(IndexValue materialId) = 0;
        virtual void SetModelMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetViewMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetProjectionMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;

        virtual void Render(const Model *model) = 0;
        virtual void Render(std::vector<IndexValue> indices, std::vector<float> vertices, std::vector<float> normals, std::vector<float> UVs) = 0;
    };

private:
    IADSRendererImplementation *_implementation;
};
