#pragma once

#include <vector>

#include "types.hh"

class Model
{
public:
    Model(IndexValue positionsId, std::vector<float> colors, IndexValue indicesId)
        : _positionsId(positionsId), _colors(colors), _indicesId(indicesId)
    {}

    IndexValue GetVertexPositionsId() const { return _positionsId; }
    IndexValue GetVertexIndicesId() const { return _indicesId; }
    std::vector<float> GetVertexColors() const { return _colors; }

private:
    IndexValue _positionsId;
    IndexValue _indicesId;
    std::vector<float> _colors;
};
