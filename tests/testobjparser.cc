#include "catch.hh"

#include "objparser.hh"

TEST_CASE("ObjParser parses simple example", "[ObjParser]")
{
    ObjParser::ObjFileParser parser("assets/", "test-simple.obj");
    ObjParser::IParseResult *result = parser.Parse();

    SECTION("Finds one vertex")
    {
        REQUIRE(result->GetVertices().size() == 1);
        ObjParser::Vertex vertex = result->GetVertices()[0];
        CHECK(vertex.coordinates[0] == 1.0);        
        CHECK(vertex.coordinates[1] == 1.0);        
        CHECK(vertex.coordinates[2] == 1.0);        
        CHECK(vertex.coordinates[3] == 1.0);        
    }

    SECTION("Finds one normal")
    {
        REQUIRE(result->GetNormals().size() == 1);
        ObjParser::Normal normal = result->GetNormals()[0];
        CHECK(normal.coordinates[0] == 0.0);
        CHECK(normal.coordinates[1] == 1.0);
        CHECK(normal.coordinates[2] == 0.0);
    }
}
