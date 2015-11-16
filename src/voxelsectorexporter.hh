#pragma once

#include <string>
#include <vector>

#include "voxels.hh"

class VoxelSectorExporter
{
public:
    void ExportSector(const VoxelCollection &collection, const std::string &fileName);
};

class VoxelSectorImporter
{
public:
    VoxelSectorImporter(VoxelRepository *repository);
    VoxelCollection *ImportSector(const std::string &fileName);

private:
    VoxelRepository *_repository;
};
