#pragma once

#include <string>

#include "types.hh"

RawImageInfo *LoadImageFromJPG(const std::string &fileName);
RawImageInfo *LoadImageFromPNG(const std::string &fileName);
