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
    void SetModelMatrix(glm::mat4 matrix);
    void SetViewMatrix(glm::mat4 matrix);
    void SetLight(LightInfo info);

    void Render(const Model *model);

public:
    class IADSRendererImplementation
    {
    public:
        virtual ~IADSRendererImplementation() {}
        virtual void Use() = 0;
        virtual Model *CreateModelFromImporter(IRenderer::Importer &importer) = 0;
        virtual void SetModelMatrix(glm::mat4 matrix) = 0;
        virtual void SetViewMatrix(glm::mat4 matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;
        virtual void Render(const Model *model) = 0;
    };

private:
    IADSRendererImplementation *_implementation;
};
