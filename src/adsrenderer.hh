#pragma once

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
    IndexValue RegisterIndexCollection(std::vector<IndexValue> indices, GLenum primitiveType);
    IndexValue RegisterVertexCollection(std::vector<float> vertices);
    IndexValue RegisterNormalCollection(std::vector<float> normals);
    IndexValue RegisterUVCollection(std::vector<float> uvCoords);
    IndexValue RegisterMaterial(MaterialInfo material);
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
        virtual IndexValue RegisterIndexCollection(std::vector<IndexValue> indices, GLenum primitiveType) = 0;
        virtual IndexValue RegisterVertexCollection(std::vector<float> vertices) = 0;
        virtual IndexValue RegisterNormalCollection(std::vector<float> normals) = 0;
        virtual IndexValue RegisterUVCollection(std::vector<float> uvCoords) = 0;
        virtual IndexValue RegisterMaterial(MaterialInfo material) = 0;
        virtual void SetModelMatrix(glm::mat4 matrix) = 0;
        virtual void SetViewMatrix(glm::mat4 matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;
        virtual void Render(const Model &model) = 0;
    };    

private:
    IADSRendererImplementation *_implementation;
};
