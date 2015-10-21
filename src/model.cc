#include "model.hh"
#include "objparser.hh"

Model::Model(const char *filename)
{
    LoadFromFile(filename);
}

void Model::LoadFromFile(const char *filename)
{
    ObjFileParser parser(filename);
    parser.Parse();
}
