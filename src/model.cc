#include "model.hh"
#include "objparser.hh"

Model::Model(const char *filename)
{
    LoadFromFile(filename);
}

void Model::LoadFromFile(const char *filename)
{
    ObjFileParser parser("assets/", filename);
    parser.Parse();

    _positions = parser.GetVertices();
    _indices = parser.GetIndices();
    _colors = {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };
}
