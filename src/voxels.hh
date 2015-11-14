#pragma once

#include "types.hh"

const int VOXEL_SECTOR_SIZE = 16;
const int VOXEL_SECTOR_ARRAY_SIZE =
    VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE;

typedef struct Voxel
{
    IndexValue tileX;
    IndexValue tileY;
} Voxel;

Voxel GenerateVoxel(IndexValue x, IndexValue y)
{
    Voxel voxel;
    voxel.tileX = x;
    voxel.tileY = y;
    return voxel;
}

class VoxelRepository
{
public:
    void AddVoxel(const Voxel &voxel)
    {
        _voxels.push_back(voxel);
    }

    const Voxel *GetVoxel(IndexValue index)
    {
        return &_voxels[index];
    }
    
private:
    std::vector<Voxel> _voxels;
};
