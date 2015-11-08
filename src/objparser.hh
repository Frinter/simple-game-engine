#pragma once

#include <vector>

#include "types.hh"
#include "objtypes.hh"

namespace ObjParser
{
    class IParseResult
    {
    public:
        virtual std::vector<Vertex> GetVertices() const = 0;
        virtual std::vector<Normal> GetNormals() const = 0;
        virtual std::vector<UVCoord> GetUVCoords() const = 0;
        virtual std::vector<IndexValue> GetIndices() const = 0;
        virtual std::vector<Face> GetFaces() const = 0;
        virtual std::vector<Material*> GetMaterials() const = 0;
    };

    class ObjFileParser
    {
    public:
        class IObjFileParserImplementation
        {
        public:
            virtual ~IObjFileParserImplementation() {}
            virtual IParseResult *Parse() = 0;
        };

    public:
        ObjFileParser(const char *path, const char *filename);
        ~ObjFileParser();

        IParseResult *Parse();
    
    private:
        IObjFileParserImplementation *_implementation;
    };
}
