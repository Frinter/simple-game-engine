#pragma once

#include <string>

typedef struct ColorValue
{
    float red, green, blue;
} ColorValue;

class IMaterial
{
public:
    virtual std::string GetName() const = 0;
    virtual ColorValue GetAmbientColor() const = 0;
};

class MtlFileParser
{
public:
    class IMtlFileParserImplementation
    {
    public:
        virtual ~IMtlFileParserImplementation() {}
        virtual void Parse() = 0;
    };

public:
    MtlFileParser(const char *path, const char *fileName);
    ~MtlFileParser();

    void Parse();
    
private:
    IMtlFileParserImplementation *_implementation;
};
