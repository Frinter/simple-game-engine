#pragma once

#include <vector>

#include "types.hh"

class Model
{
public:
    Model(IndexValue positionsId, IndexValue normalsId, IndexValue indicesId)
        : _positionsId(positionsId), _normalsId(normalsId), _indicesId(indicesId)
    {}

    IndexValue GetVertexPositionsId() const { return _positionsId; }
    IndexValue GetVertexNormalsId() const { return _normalsId; }
    IndexValue GetVertexIndicesId() const { return _indicesId; }

private:
    IndexValue _positionsId;
    IndexValue _normalsId;
    IndexValue _indicesId;
};
