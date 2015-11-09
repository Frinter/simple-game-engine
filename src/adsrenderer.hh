#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "irenderer.hh"
#include "model.hh"

class ADSRenderer : public IRenderer
{
public:
    class Importer
    {
    public:
        virtual ~Importer() {}
        virtual std::vector<float> GetVertices() = 0;
        virtual std::vector<float> GetNormals() = 0;
        virtual std::vector<float> GetUVCoords() = 0;
        virtual std::vector<IndexValue> GetIndices() = 0;
        virtual MaterialInfo GetMaterial(const char *name) = 0;
    };

public:
    ADSRenderer();
    ~ADSRenderer();
    
    void Use();
    Model CreateModelFromImporter(Importer &importer);
    void SetModelMatrix(glm::mat4 matrix);
    void SetViewMatrix(glm::mat4 matrix);
    void SetLight(LightInfo info);

    void Render(const Model &model);

public:
    class IADSRendererImplementation
    {
    public:
        virtual ~IADSRendererImplementation() {}
        virtual void Use() = 0;
        virtual Model CreateModelFromImporter(Importer &importer) = 0;
        virtual void SetModelMatrix(glm::mat4 matrix) = 0;
        virtual void SetViewMatrix(glm::mat4 matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;
        virtual void Render(const Model &model) = 0;
    };

private:
    IADSRendererImplementation *_implementation;
};
