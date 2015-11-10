#include <cstdio>
#include <sstream>
#include <string>
#include <stdexcept>

#include "types.hh"
#include "objimporter.hh"

#include <jpeglib.h>

using std::string;

RawImageInfo *LoadImage(string fileName)
{
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr jpegError;
    ImageData imageData;
    ImageData scanlinePointer[1];
    unsigned int imageWidth, imageHeight, dataSize, dataWidth;
    RawImageInfo *imageInfo = (RawImageInfo *) malloc(sizeof(RawImageInfo));
    
    FILE* file = fopen(fileName.c_str(), "rb");

    info.err = jpeg_std_error(&jpegError);
    jpeg_create_decompress(&info);

    jpeg_stdio_src(&info, file);
    jpeg_read_header(&info, TRUE);

    jpeg_start_decompress(&info);

    imageInfo->width = info.output_width;
    imageInfo->height = info.output_height;
    imageInfo->components = info.output_components == 4 ? GL_RGBA : GL_RGB;
    dataWidth = imageInfo->width * info.output_components;
    dataSize = imageInfo->height * dataWidth;
    imageInfo->data = (ImageData) malloc(dataSize);

    while (info.output_scanline < info.output_height)
    {
        scanlinePointer[0] = imageInfo->data + dataWidth * info.output_scanline;
        jpeg_read_scanlines(&info, scanlinePointer, 1);
    }
    
    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);

    fclose(file);
    
    return imageInfo;
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
