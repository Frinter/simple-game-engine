#pragma once

#include <vector>

#include "types.hh"

class Model
{
public:
    Model(IndexValue positionsId, IndexValue normalsId, IndexValue indicesId, IndexValue materialId)
        : _positionsId(positionsId), _normalsId(normalsId), _indicesId(indicesId), _materialId(materialId)
    {}

    IndexValue GetVertexPositionsId() const { return _positionsId; }
    IndexValue GetVertexNormalsId() const { return _normalsId; }
    IndexValue GetVertexIndicesId() const { return _indicesId; }
    IndexValue GetMaterialId() const { return _materialId; }

private:
    IndexValue _positionsId;
    IndexValue _normalsId;
    IndexValue _indicesId;
    IndexValue _materialId;
};
