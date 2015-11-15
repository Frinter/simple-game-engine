#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "voxelsector.hh"

class VoxelSectorExporter
{
private:
    typedef struct VoxelRecord
    {
        unsigned char type;
    } VoxelRecord;

public:
    void ExportSector(const VoxelCollection &collection, const std::string &fileName)
    {
        FILE *file = fopen(fileName.c_str(), "wb");
        if (file == NULL)
            return;

        std::vector<VoxelRecord> records;

        for (int x = 0; x < VOXEL_SECTOR_SIZE; ++x)
        {
            for (int z = 0; z < VOXEL_SECTOR_SIZE; ++z)
            {
                for (int y = 0; y < VOXEL_SECTOR_SIZE; ++y)
                {
                    VoxelRecord record;
                    const Voxel *voxel = collection.GetVoxel(Position(x, y, z));
                    if (voxel == NULL)
                        record.type = 0;
                    else
                        record.type = voxel->type;
                    records.push_back(record);
                }
            }
        }

        fwrite(records.data(), sizeof(VoxelRecord), records.size(), file);

        fclose(file);
    }
};
