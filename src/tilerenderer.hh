#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "rendering/irenderer.hh"
#include "types.hh"

enum class Direction
{
    Up,
    Down,
    Left,
    Right,
    Forward,
    Backward
};

class TileRenderer
{
public:
    TileRenderer(IRenderer *renderer, RawImageInfo *tilemapImage,
                 float tileWidth, float tileHeight);

    void Render(IndexValue tileX, IndexValue tileY,
                const glm::vec4 &location, Direction direction);

private:
    IRenderer *_renderer;
    IndexValue _materialId;
    unsigned int _tileWidth;
    unsigned int _tileHeight;
    unsigned int _tilemapWidth;
    unsigned int _tilemapHeight;

private:
    void addToVector(std::vector<float> &list, const glm::vec4 &vec);
    void addToVector(std::vector<float> &list, const glm::vec3 &vec);
    std::vector<float> GetUVsForTile(IndexValue x, IndexValue y);
};
