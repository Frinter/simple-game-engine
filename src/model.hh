#pragma once

#include <vector>

#include "types.hh"

class Model
{
public:
    Model(IndexValue positionsId, IndexValue normalsId, std::vector<float> colors, IndexValue indicesId)
        : _positionsId(positionsId), _normalsId(normalsId), _colors(colors), _indicesId(indicesId)
    {}

    IndexValue GetVertexPositionsId() const { return _positionsId; }
    IndexValue GetVertexNormalsId() const { return _normalsId; }
    IndexValue GetVertexIndicesId() const { return _indicesId; }
    std::vector<float> GetVertexColors() const { return _colors; }

private:
    IndexValue _positionsId;
    IndexValue _normalsId;
    IndexValue _indicesId;
    std::vector<float> _colors;
};
