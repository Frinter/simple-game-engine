#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "entity.hh"
#include "tilerenderer.hh"
#include "types.hh"
#include "voxels.hh"
#include "voxelsectorexporter.hh"

class VoxelSectorGraphicsComponent
{
public:
    VoxelSectorGraphicsComponent(IRenderer *renderer, TileRenderer *tileRenderer)
        : _renderer(renderer), _tileRenderer(tileRenderer)
    {
    }

    void update(const VoxelCollection &collection, const Position &sectorPosition)
    {
        glm::vec3 translation = glm::vec3(sectorPosition.x * VOXEL_SECTOR_SIZE,
                                          sectorPosition.y * VOXEL_SECTOR_SIZE,
                                          sectorPosition.z * VOXEL_SECTOR_SIZE);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        _renderer->SetModelMatrix(translationMatrix);

        int x, y, z;
        for (x = 0; x < VOXEL_SECTOR_SIZE; ++x)
        {
            for (y = 0; y < VOXEL_SECTOR_SIZE; ++y)
            {
                for (z = 0; z < VOXEL_SECTOR_SIZE; ++z)
                {
                    renderVoxel(collection, x, y, z);
                }
            }
        }
   }

private:
    IRenderer *_renderer;
    TileRenderer *_tileRenderer;

private:
    void renderVoxel(const VoxelCollection &collection, int x, int y, int z)
    {
        const Voxel *voxel = collection.GetVoxel(Position(x, y, z));
        if (voxel != NULL)
        {
            if (x == 0 || collection.GetVoxel(Position(x-1, y, z)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Left);
            }

            if (x == VOXEL_SECTOR_SIZE || collection.GetVoxel(Position(x+1, y, z)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Right);
            }

            if (y == 0 || collection.GetVoxel(Position(x, y-1, z)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Up);
            }

            if (y == VOXEL_SECTOR_SIZE || collection.GetVoxel(Position(x, y+1, z)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Down);
            }

            if (z == 0 || collection.GetVoxel(Position(x, y, z-1)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Forward);
            }

            if (z == VOXEL_SECTOR_SIZE || collection.GetVoxel(Position(x, y, z+1)) == NULL)
            {
                _tileRenderer->Render(voxel->tileX, voxel->tileY,
                                      glm::vec4(x, y, z, 1.0),
                                      Direction::Backward);
            }
        }
    }
};

class VoxelSector : public Entity
{
public:
    VoxelSector(VoxelSectorGraphicsComponent *graphicsComponent,
                VoxelRepository *voxelRepository,
                const Position &sectorPosition)
        : _graphicsComponent(graphicsComponent),
          _collection(voxelRepository),
          _sectorPosition(sectorPosition)
    {
    }

    void update()
    {
        _graphicsComponent->update(_collection, _sectorPosition);
    }

    void SetVoxel(const Position &position, IndexValue type)
    {
        _collection.SetVoxel(position, type);
    }

    void Export(const std::string &fileName)
    {
        VoxelSectorExporter().ExportSector(_collection, fileName);
    }

private:
    VoxelSectorGraphicsComponent *_graphicsComponent;
    VoxelCollection _collection;
    Position _sectorPosition;
};

VoxelSector *CreateVoxelSector(IRenderer *renderer, TileRenderer *tileRenderer,
                               VoxelRepository *voxelRepository,
                               const Position &sectorPosition)
{
    VoxelSectorGraphicsComponent *graphicsComponent = new VoxelSectorGraphicsComponent(renderer, tileRenderer);
    VoxelSector *sector = new VoxelSector(graphicsComponent, voxelRepository,
                                          sectorPosition);

    return sector;
}
