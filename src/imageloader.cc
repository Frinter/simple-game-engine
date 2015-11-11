#include <cstdio>
#include <string>

#include "imageloader.hh"
#include "types.hh"

#include <jpeglib.h>
#include <lodepng.h>

using std::string;

RawImageInfo *LoadImageFromJPG(const std::string &fileName)
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

RawImageInfo *LoadImageFromPNG(const std::string &fileName)
{
    RawImageInfo *imageInfo = (RawImageInfo *) malloc(sizeof(RawImageInfo));
    std::vector<unsigned char> pixels;
    unsigned int width, height;
    
    lodepng::decode(pixels, imageInfo->width, imageInfo->height, fileName.c_str());

    imageInfo->components = GL_RGBA;
    imageInfo->data = (ImageData) malloc((sizeof (unsigned char)) *pixels.size());
    memcpy(imageInfo->data, pixels.data(), (sizeof (unsigned char)) *pixels.size());

    return imageInfo;
}
