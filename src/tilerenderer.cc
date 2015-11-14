#include "tilerenderer.hh"

TileRenderer::TileRenderer(IRenderer *renderer, RawImageInfo *tilemapImage, float tileWidth, float tileHeight)
    : _renderer(renderer), _tilemapImage(tilemapImage), _tileWidth(tileWidth), _tileHeight(tileHeight)
{
    MaterialInfo tilemapMaterialInfo;
    tilemapMaterialInfo.Ka = glm::vec3(1.0f, 1.0f, 1.0f);
    tilemapMaterialInfo.Kd = glm::vec3(1.0f, 1.0f, 1.0f);
    tilemapMaterialInfo.Ks = glm::vec3(0.0f, 0.0f, 0.0f);
    tilemapMaterialInfo.shininess = 1.0f;
    tilemapMaterialInfo.Kd_imageInfo = tilemapImage;

    _materialId = _renderer->RegisterMaterial(tilemapMaterialInfo);
}

void TileRenderer::Render(IndexValue tileX, IndexValue tileY,
                          const glm::vec4 &location, Direction direction)
{
    std::vector<IndexValue> indices;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> UVs = GetUVsForTile(tileX, tileY);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(4);
    indices.push_back(5);

    switch(direction)
    {
    case Direction::Up:
        addToVector(vertices, location);
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));

        addToVector(vertices, location);
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));

        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case Direction::Down:
        addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 1.0f, 0.0f));

        addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 1.0f, 0.0f));

        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));

        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
        addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
        break;

    case Direction::Left:
        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));

        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));

        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
        break;

    case Direction::Right:
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));

        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));

        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
        addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
        break;

    case Direction::Forward:
        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));

        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));

        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));

        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        break;

    case Direction::Backward:
        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));

        addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));
        addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));

        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
        break;
    }

    _renderer->UseMaterial(_materialId);
    _renderer->Render(indices, vertices, normals, UVs);
}

void TileRenderer::addToVector(std::vector<float> &list, const glm::vec4 &vec)
{
    list.push_back(vec[0]);
    list.push_back(vec[1]);
    list.push_back(vec[2]);
    list.push_back(vec[3]);
}

void TileRenderer::addToVector(std::vector<float> &list, const glm::vec3 &vec)
{
    list.push_back(vec[0]);
    list.push_back(vec[1]);
    list.push_back(vec[2]);
}

std::vector<float> TileRenderer::GetUVsForTile(IndexValue x, IndexValue y)
{
    std::vector<float> UVs;

    unsigned int top, bottom, left, right;
    top = y * _tileHeight;
    bottom = (y + 1) * _tileHeight;
    left = x * _tileWidth;
    right = (x + 1) * _tileWidth;

    float UVtop, UVbottom, UVleft, UVright;
    UVleft = 0.995 * (float) left / _tilemapImage->width + 0.002f;
    UVright = 0.995 * (float) right / _tilemapImage->width + 0.002f;
    UVtop = 0.995 * (float) top / _tilemapImage->height + 0.002f;
    UVbottom = 0.995 * (float) bottom / _tilemapImage->height + 0.002f;

    UVs.push_back(UVleft);
    UVs.push_back(UVtop);
    UVs.push_back(UVleft);
    UVs.push_back(UVbottom);
    UVs.push_back(UVright);
    UVs.push_back(UVbottom);

    UVs.push_back(UVleft);
    UVs.push_back(UVtop);
    UVs.push_back(UVright);
    UVs.push_back(UVbottom);
    UVs.push_back(UVright);
    UVs.push_back(UVtop);

    return UVs;
}
