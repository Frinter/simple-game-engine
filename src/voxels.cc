#include "voxels.hh"

Voxel GenerateVoxel(VoxelType type, IndexValue x, IndexValue y)
{
    Voxel voxel;
    voxel.type = type;
    voxel.tileX = x;
    voxel.tileY = y;
    return voxel;
}
