#include <cstdio>
#include <sstream>
#include <string>
#include <stdexcept>

#include "types.hh"
#include "objimporter.hh"
#include "imageloader.hh"

RawImageInfo *LoadImage(const std::string &fileName)
{
    std::string extension = fileName.substr(fileName.size() - 4, 4);
    if (extension == ".png")
        return LoadImageFromPNG(fileName);
    if (extension == ".jpg")
        return LoadImageFromJPG(fileName);
    return NULL;
}

MaterialInfo ObjImporter::translateMaterial(ObjParser::Material *material)
{
    MaterialInfo info;

    info.Ka = translateColor(material->ambientColor);
    info.Kd = translateColor(material->diffuseColor);
    info.Ks = translateColor(material->specularColor);
    info.shininess = 0.5;

    info.Kd_imageInfo = material->diffuseMap.empty() ? NULL : LoadImage(material->diffuseMap);
    
    return info;
}

MaterialInfo ObjImporter::GetMaterial(const std::string &name)
{
    std::vector<ObjParser::Material*> materials = _parseResult->GetMaterials();
    ObjParser::Material *material = NULL;

    for (int i = 0; i < materials.size(); ++i)
    {
        if (materials[i]->name == name)
        {
            return translateMaterial(materials[i]);
        }
    }

    std::stringstream errorStream;
    errorStream << "Runtime error: unable to find material in parse result: " << name;
    throw std::runtime_error(errorStream.str());
}
