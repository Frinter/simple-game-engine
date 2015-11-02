#pragma once

#include <vector>

#include "types.hh"

class ObjFileParser
{
public:
    class IObjFileParserImplementation
    {
    public:
        virtual ~IObjFileParserImplementation() {}
        virtual void Parse() = 0;
        virtual std::vector<float> GetVertices() = 0;
        virtual std::vector<float> GetNormals() = 0;
        virtual std::vector<IndexValue> GetIndices() = 0;
    };

public:
    ObjFileParser(const char *path, const char *filename);
    ~ObjFileParser();

    void Parse();

    std::vector<float> GetVertices();
    std::vector<float> GetNormals();
    std::vector<IndexValue> GetIndices();
    
private:
    IObjFileParserImplementation *_implementation;
};
