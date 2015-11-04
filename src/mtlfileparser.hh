#pragma once

#include <string>
#include <vector>

#include "objtypes.hh"

namespace ObjParser
{
    class MtlFileParser
    {
    public:
        class IMtlFileParserImplementation
        {
        public:
            virtual ~IMtlFileParserImplementation() {}
            virtual void Parse() = 0;
            virtual std::vector<Material*> GetMaterials() const = 0;
        };

    public:
        MtlFileParser(const char *path, const char *fileName);
        ~MtlFileParser();

        void Parse();

        std::vector<Material*> GetMaterials() const;
    
    private:
        IMtlFileParserImplementation *_implementation;
    };
}
