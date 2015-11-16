#include <cstdio>
#include <iostream>

#include "voxelsectorexporter.hh"

typedef struct VoxelRecord
{
    unsigned char index;
} VoxelRecord;

IndexValue GetIndexLocation(int x, int y, int z)
{
    return z * VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE +
        y * VOXEL_SECTOR_SIZE +
        x;
}

VoxelSectorExporter::VoxelSectorExporter()
{
}

void VoxelSectorExporter::ExportSector(const VoxelCollection &collection, const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "wb");
    if (file == NULL)
        return;

    std::vector<VoxelRecord> records;

    for (int z = 0; z < VOXEL_SECTOR_SIZE; ++z)
    {
        for (int y = 0; y < VOXEL_SECTOR_SIZE; ++y)
        {
            for (int x = 0; x < VOXEL_SECTOR_SIZE; ++x)
            {
                VoxelRecord record;
                const Voxel *voxel = collection.GetVoxel(Position(x, y, z));
                if (voxel == NULL)
                    record.index = 0;
                else
                    record.index = voxel->id + 1;
                records.push_back(record);
            }
        }
    }

    fwrite(records.data(), sizeof(VoxelRecord), records.size(), file);

    fclose(file);
}

VoxelSectorImporter::VoxelSectorImporter(VoxelRepository *repository)
    : _repository(repository)
{
}

VoxelCollection *VoxelSectorImporter::ImportSector(const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (file == NULL)
        return NULL;

    VoxelRecord records[VOXEL_SECTOR_ARRAY_SIZE];

    fread(records, sizeof(VoxelRecord), VOXEL_SECTOR_ARRAY_SIZE, file);

    fclose(file);

    VoxelCollection *collection = new VoxelCollection(_repository);

    for (int z = 0; z < VOXEL_SECTOR_SIZE; ++z)
    {
        for (int y = 0; y < VOXEL_SECTOR_SIZE; ++y)
        {
            for (int x = 0; x < VOXEL_SECTOR_SIZE; ++x)
            {
                VoxelRecord record = records[GetIndexLocation(x, y, z)];
                if (record.index != 0)
                    collection->SetVoxel(Position(x, y, z), record.index - 1);
            }
        }
    }

    return collection;
}
