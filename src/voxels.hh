#pragma once

#include <vector>
#include <unordered_map>

#include "types.hh"

const int VOXEL_SECTOR_SIZE = 16;
const int VOXEL_SECTOR_ARRAY_SIZE =
    VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE;

typedef unsigned char VoxelId;

class VoxelType
{
public:
    VoxelType(IndexValue tileX, IndexValue tileY)
        : _tileX(tileX), _tileY(tileY)
    {
    }

    IndexValue GetTileX() const
    {
        return _tileX;
    }

    IndexValue GetTileY() const
    {
        return _tileY;
    }

private:
    IndexValue _tileX;
    IndexValue _tileY;
};

typedef struct Voxel
{
    Voxel(const VoxelType &type)
        : _type(type)
    {
    }

    VoxelId id;
    VoxelType _type;
} Voxel;

class VoxelRepository
{
public:
    void AddVoxelType(const VoxelType &type)
    {
        Voxel voxel(type);
        voxel.id = _voxels.size();
        _voxels.push_back(voxel);
        _voxelMap[voxel.id] = &_voxels.back();
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
