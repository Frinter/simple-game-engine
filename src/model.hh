#pragma once

typedef unsigned char IndexValue;

class Model
{
public:
    Model(std::vector<float> positions, std::vector<float> colors, std::vector<IndexValue> indices)
        : _positions(positions), _colors(colors), _indices(indices)
    {}
    
    std::vector<float> GetVertexPositions() const { return _positions; }
    std::vector<float> GetVertexColors() const { return _colors; }
    std::vector<IndexValue> GetVertexIndices() const { return _indices; }

private:
    std::vector<float> _positions;
    std::vector<float> _colors;
    std::vector<IndexValue> _indices;
};
