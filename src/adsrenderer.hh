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
    IndexValue RegisterTileMap(RawImageInfo *tilemapImage, unsigned int tileWidth, unsigned int tileHeight);
    void SetModelMatrix(const glm::mat4 &matrix);
    void SetViewMatrix(const glm::mat4 &matrix);
    void SetProjectionMatrix(const glm::mat4 &matrix);
    void SetLight(LightInfo info);

    void RenderTile(IndexValue tilemapId, IndexValue x, IndexValue y);
    void Render(const Model *model);

public:
    class IADSRendererImplementation
    {
    public:
        virtual ~IADSRendererImplementation() {}
        virtual void Use() = 0;
        virtual Model *CreateModelFromImporter(IRenderer::Importer &importer) = 0;
        virtual IndexValue RegisterTileMap(RawImageInfo *tilemapImage, unsigned int tileWidth, unsigned int tileHeight) = 0;
        virtual void SetModelMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetViewMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetProjectionMatrix(const glm::mat4 &matrix) = 0;
        virtual void SetLight(LightInfo info) = 0;

        virtual void RenderTile(IndexValue tilemapId, IndexValue x, IndexValue y) = 0;
        virtual void Render(const Model *model) = 0;
    };

private:
    IADSRendererImplementation *_implementation;
};
