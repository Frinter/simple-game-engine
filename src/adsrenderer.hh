#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "irenderer.hh"
#include "model.hh"

typedef struct RenderObject
{
    std::vector<IndexValue> _indices;
    std::vector<float> _vertices;
    std::vector<float> _normals;
    std::vector<float> _uvCoords;
    std::string _materialName;
} RenderObject;

class ADSRenderer : public IRenderer
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
