#pragma once

class ObjFileParser
{
public:
    class IObjFileParserImplementation
    {
    public:
        virtual ~IObjFileParserImplementation() {}
        virtual void Parse() = 0;
    };

public:
    ObjFileParser(const char *filename);

    void Parse();

private:
    IObjFileParserImplementation *_implementation;
};
