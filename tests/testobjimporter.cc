#include "catch.hh"

#include "objimporter.hh"
#include "objparser.hh"

class SampleParseResult : public ObjParser::IParseResult
{
public:
    virtual std::vector<ObjParser::Vertex> GetVertices() const
    {
        return _vertices;
    }
    
    virtual std::vector<ObjParser::Normal> GetNormals() const
    {
        return _normals;
    }
    
    virtual std::vector<IndexValue> GetIndices() const
    {
        return _indices;
    }
    
    virtual std::vector<ObjParser::Face> GetFaces() const
    {
        return _faces;
    }
    
    virtual std::vector<ObjParser::Material*> GetMaterials() const
    {
        return _materials;
    }

public:
    std::vector<ObjParser::Vertex> _vertices;
    std::vector<ObjParser::Normal> _normals;
    std::vector<IndexValue> _indices;
    std::vector<ObjParser::Face> _faces;
    std::vector<ObjParser::Material*> _materials;
};

ObjParser::Vertex GenerateVertex(float x, float y, float z)
{
    ObjParser::Vertex vertex;
    vertex.coordinates[0] = x;
    vertex.coordinates[1] = y;
    vertex.coordinates[2] = z;
    vertex.coordinates[3] = 1;
    return vertex;
}

ObjParser::Normal GenerateNormal(float x, float y, float z)
{
    ObjParser::Normal normal;
    normal.coordinates[0] = x;
    normal.coordinates[1] = y;
    normal.coordinates[2] = z;
    return normal;
}

TEST_CASE("ObjImporter with no face elements in parse result", "[ObjImportert]")
{
    SampleParseResult result;

    result._vertices.push_back(GenerateVertex(1.0, 1.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 1.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 0.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 0.0, 0.0));

    result._normals.push_back(GenerateNormal(1.0, 1.0, 0.0));
    
    ObjImporter importer(&result);

    SECTION("produces no vertices")
    {
        REQUIRE(importer.GetVertices().size() == 0);
    }

    SECTION("produces no normals")
    {
        REQUIRE(importer.GetNormals().size() == 0);
    }
}

TEST_CASE("ObjImporter with one face in parse result", "[ObjImportert]")
{
    SampleParseResult result;

    result._vertices.push_back(GenerateVertex(1.0, 1.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 1.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 0.0, 1.0));
    result._vertices.push_back(GenerateVertex(0.0, 0.0, 0.0));

    result._normals.push_back(GenerateNormal(1.0, 1.0, 0.0));

    ObjParser::Face face;
    face.vertexIndices.push_back(0);
    face.vertexIndices.push_back(1);
    face.vertexIndices.push_back(2);

    face.normalIndices.push_back(0);
    face.normalIndices.push_back(0);
    face.normalIndices.push_back(0);

    result._faces.push_back(face);

    ObjImporter importer(&result);

    SECTION("produces appropriate vertices")
    {
        std::vector<float> vertices = importer.GetVertices();
        REQUIRE(vertices.size() == 12);

        CHECK(vertices[0]  == 1.0);
        CHECK(vertices[1]  == 1.0);
        CHECK(vertices[2]  == 1.0);
        CHECK(vertices[3]  == 1.0);

        CHECK(vertices[4]  == 0.0);
        CHECK(vertices[5]  == 1.0);
        CHECK(vertices[6]  == 1.0);
        CHECK(vertices[7]  == 1.0);

        CHECK(vertices[8]  == 0.0);
        CHECK(vertices[9]  == 0.0);
        CHECK(vertices[10] == 1.0);
        CHECK(vertices[11] == 1.0);
    }

    SECTION("produces appropriate normals")
    {
        std::vector<float> normals = importer.GetNormals();
        REQUIRE(normals.size() == 9);

        CHECK(normals[0] == 1.0);
        CHECK(normals[1] == 1.0);
        CHECK(normals[2] == 0.0);

        CHECK(normals[3] == 1.0);
        CHECK(normals[4] == 1.0);
        CHECK(normals[5] == 0.0);

        CHECK(normals[6] == 1.0);
        CHECK(normals[7] == 1.0);
        CHECK(normals[8] == 0.0);
    }

    SECTION("produces appropriate normals")
    {
        std::vector<IndexValue> indices = importer.GetIndices();
        REQUIRE(indices.size() == 3);
        CHECK(indices[0] == 0);
        CHECK(indices[1] == 1);
        CHECK(indices[2] == 2);
    }
}
