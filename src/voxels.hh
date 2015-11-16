#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>

#include "types.hh"

const int VOXEL_SECTOR_SIZE = 16;
const int VOXEL_SECTOR_ARRAY_SIZE =
    VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE;

typedef unsigned char VoxelType;
typedef unsigned char VoxelId;

typedef struct Voxel
{
    VoxelId id;
    VoxelType type;
    IndexValue tileX;
    IndexValue tileY;
} Voxel;

Voxel GenerateVoxel(VoxelType type, IndexValue x, IndexValue y);

class VoxelRepository
{
public:
    void AddVoxel(const Voxel &voxel)
    {
        Voxel _voxel = voxel;
        _voxel.id = _voxels.size();
        _voxels.push_back(_voxel);
        _voxelMap[_voxel.id] = &_voxels.back();
    }

    const Voxel *GetVoxel(IndexValue index)
    {
        return &_voxels[index];
    }

    const Voxel *GetVoxelById(VoxelId id)
    {
        return _voxelMap[id];
    }

private:
    std::vector<Voxel> _voxels;
    std::unordered_map<VoxelId, Voxel*> _voxelMap;
};

class VoxelCollection
{
public:
    VoxelCollection(VoxelRepository *voxelRepository)
        : _repository(voxelRepository)
    {
        _voxels = new const Voxel*[VOXEL_SECTOR_ARRAY_SIZE];
        for (int i = 0; i < VOXEL_SECTOR_ARRAY_SIZE; ++i)
        {
            _voxels[i] = NULL;
        }
    }

    const Voxel *GetVoxel(const Position &position) const
    {
        return *getVoxelAtIndex(position);
    }

    void SetVoxel(const Position &position, IndexValue type)
    {
        *getVoxelAtIndex(position) = _repository->GetVoxel(type);
    }

private:
    VoxelRepository *_repository;

    const Voxel **_voxels;

private:
    const Voxel **getVoxelAtIndex(const Position &position) const
    {
        return &_voxels[VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * position.y +
                        VOXEL_SECTOR_SIZE * position.z +
                        position.x];
    }
};
