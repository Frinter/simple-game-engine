#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/gl_core_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "adsrenderer.hh"
#include "shaderprogram.hh"
#include "types.hh"

using std::cout;
using std::endl;
using std::string;
using std::vector;

const float imageVertices[] = {
    1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f
};

const float imageNormals[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f
};

const IndexValue imageIndices[] = {
    0, 1, 2, 3, 4, 5
};

template <class T>
class VertexArrayBuffer
{
public:
    VertexArrayBuffer(GLenum targetType)
        : _targetType(targetType)
    {
        glGenBuffers(1, &_bufferHandle);
        _bindData = &VertexArrayBuffer::initialBindData;
    }

    ~VertexArrayBuffer()
    {
        glDeleteBuffers(1, &_bufferHandle);
    }

    void SetUp(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
    {
        Bind();
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    void Bind()
    {
        glBindBuffer(_targetType, _bufferHandle);
    }

    IndexValue RegisterDataCollection(const T *collectionData, int count)
    {
        IndexValue newIndex = _vertexDataCollections.size();
        vector<T> data(collectionData, collectionData + count);
        _vertexDataCollections.push_back(data);
        return newIndex;
    }

    IndexValue RegisterDataCollection(vector<T> vertexData)
    {
        IndexValue newIndex = _vertexDataCollections.size();
        _vertexDataCollections.push_back(vertexData);
        return newIndex;
    }

    void UseDataCollection(const vector<T> &vertexData)
    {
        bindUnindexedData(vertexData);
    }

    unsigned int currentSize() const
    {
        return _vertexDataCollections[_currentIndex].size();
    }

    void UseDataCollection(IndexValue collectionIndex)
    {
        (this->*_bindData)(collectionIndex);
    }

private:
    void (VertexArrayBuffer::*_bindData)(IndexValue);

    bool _isValid;
    GLuint _bufferHandle;
    GLenum _targetType;
    IndexValue _currentIndex;
    vector< vector<T> > _vertexDataCollections;

private:
    void initialBindData(IndexValue index)
    {
        _bindData = &VertexArrayBuffer::bindData;
        unconditionalBindData(index);
    }

    void bindData(IndexValue index)
    {
        if (index != _currentIndex)
            unconditionalBindData(index);
    }

    void bindUnindexedData(const vector<T> &vertexData)
    {
        _bindData = &VertexArrayBuffer::initialBindData;
        Bind();
        glBufferData(_targetType, vertexData.size() * sizeof(T), vertexData.data(), GL_STATIC_DRAW);
    }

    void unconditionalBindData(IndexValue index)
    {
        _currentIndex = index;
        Bind();
        vector<T> *vertexData = &_vertexDataCollections[index];
        glBufferData(_targetType, vertexData->size() * sizeof(T), vertexData->data(), GL_STATIC_DRAW);
    }
};

class ADSRendererImplementation : public ADSRenderer::IADSRendererImplementation
{
public:
    ADSRendererImplementation()
        : _indexBuffer(GL_ELEMENT_ARRAY_BUFFER),
          _positionBuffer(GL_ARRAY_BUFFER), _normalBuffer(GL_ARRAY_BUFFER),
          _uvBuffer(GL_ARRAY_BUFFER)
    {
        _shaderProgram = new ShaderProgram("ads");

        glEnable(GL_DEPTH_TEST);

        glGenVertexArrays(1, &_vaoHandle);
        glBindVertexArray(_vaoHandle);

        _shaderProgram->EnableAttribArray(0);
        _shaderProgram->EnableAttribArray(1);
        _shaderProgram->EnableAttribArray(2);

        _shaderProgram->BindAttribLocation(0, "VertexPosition");
        _shaderProgram->BindAttribLocation(1, "VertexNormal");
        _shaderProgram->BindAttribLocation(2, "VertexTexCoord");

        _shaderProgram->Link();
        _shaderProgram->Use();

        _positionBuffer.SetUp(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
        _normalBuffer.SetUp(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
        _uvBuffer.SetUp(2, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

        _modelViewMatrixLocation = _shaderProgram->GetUniformLocation("ModelViewMatrix");
        _normalMatrixLocation = _shaderProgram->GetUniformLocation("NormalMatrix");
        _projectionMatrixLocation = _shaderProgram->GetUniformLocation("ProjectionMatrix");
        _MVPMatrixLocation = _shaderProgram->GetUniformLocation("MVP");

        _imageVertexIndex = _positionBuffer.RegisterDataCollection(imageVertices, 24);
        _imageNormalIndex = _normalBuffer.RegisterDataCollection(imageNormals, 18);
        _imageIndexIndex = _indexBuffer.RegisterDataCollection(imageIndices, 6);
    }

    void Use()
    {
        _shaderProgram->Use();
    }

    IndexValue RegisterMaterial(MaterialInfo material)
    {
        IndexValue index = _materials.size();
        if (material.Kd_imageInfo != NULL)
        {
            material.hasKdMap = true;
            material.Kd_mapId = registerTexture(material.Kd_imageInfo);
        }
        else
        {
            material.hasKdMap = false;
        }

        _materials.push_back(material);
        return index;
    }

    void SetModelMatrix(const glm::mat4 &matrix)
    {
        _modelMatrix = matrix;

        _modelViewMatrix = _viewMatrix * _modelMatrix;
        _normalMatrix = glm::transpose(glm::inverse(glm::mat3(_modelViewMatrix)));
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;
    }

    void SetViewMatrix(const glm::mat4 &matrix)
    {
        _viewMatrix = matrix;

        _modelViewMatrix = _viewMatrix * _modelMatrix;
        _normalMatrix = glm::transpose(glm::inverse(glm::mat3(_modelViewMatrix)));
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;
    }

    void SetProjectionMatrix(const glm::mat4 &matrix)
    {
        _projectionMatrix = matrix;
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;
    }

    void SetLight(LightInfo info)
    {
        _light.position = info.position;
        _light.La = info.La;
        _light.Ld = info.Ld;
        _light.Ls = info.Ls;

        glm::vec4 viewLightPosition = _modelViewMatrix * _light.position;

        _shaderProgram->SetUniform("Light.Position", viewLightPosition);
        _shaderProgram->SetUniform("Light.La", glm::value_ptr(_light.La));
        _shaderProgram->SetUniform("Light.Ld", glm::value_ptr(_light.Ld));
        _shaderProgram->SetUniform("Light.Ls", glm::value_ptr(_light.Ls));
    }

    void Render(const vector<IndexValue> &indices,
                const vector<float> &vertices,
                const vector<float> &normals,
                const std::vector<float> &UVs,
                const IndexValue &materialId)
    {
        glBindVertexArray(_vaoHandle);

        glUniformMatrix4fv(_modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
        glUniformMatrix3fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));
        glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(_projectionMatrix));
        glUniformMatrix4fv(_MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(_MVPMatrix));

        _indexBuffer.UseDataCollection(indices);
        _positionBuffer.UseDataCollection(vertices);
        _normalBuffer.UseDataCollection(normals);
        _uvBuffer.UseDataCollection(UVs);
        useMaterial(materialId);

        glDrawArrays(GL_TRIANGLES, 0, indices.size());
    }

private:
    LightInfo _light;

    VertexArrayBuffer<IndexValue> _indexBuffer;
    VertexArrayBuffer<float> _positionBuffer;
    VertexArrayBuffer<float> _normalBuffer;
    VertexArrayBuffer<float> _uvBuffer;

    vector<MaterialInfo> _materials;

    ShaderProgram *_shaderProgram;
    GLuint _vaoHandle;

    GLuint _modelViewMatrixLocation;
    GLuint _normalMatrixLocation;
    GLuint _projectionMatrixLocation;
    GLuint _MVPMatrixLocation;

    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;
    glm::mat4 _modelMatrix;
    glm::mat4 _modelViewMatrix;
    glm::mat4 _MVPMatrix;

    glm::mat3 _normalMatrix;

    IndexValue _imageVertexIndex;
    IndexValue _imageNormalIndex;
    IndexValue _imageUVIndex;
    IndexValue _imageIndexIndex;
    IndexValue _imageMaterialIndex;

private:
    GLuint registerTexture(RawImageInfo *imageInfo)
    {
        GLuint textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, imageInfo->components,
                     imageInfo->width, imageInfo->height, 0,
                     imageInfo->components, GL_UNSIGNED_BYTE, imageInfo->data);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        return textureId;
    }

    void useMaterial(IndexValue index)
    {
        useMaterial(_materials[index]);
    }

    void useMaterial(const MaterialInfo &info)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, info.Kd_mapId);

        _shaderProgram->SetUniform("Material.Ka", glm::value_ptr(info.Ka));
        _shaderProgram->SetUniform("Material.Kd", glm::value_ptr(info.Kd));
        _shaderProgram->SetUniform("Material.Ks", glm::value_ptr(info.Ks));
        _shaderProgram->SetUniform("Material.Shininess", &info.shininess);
        _shaderProgram->SetUniform("Material.Kd_map", 0);
        _shaderProgram->SetUniform("Material.hasKdMap", (int)info.hasKdMap);
    }
};

ADSRenderer::ADSRenderer()
{
    _implementation = new ADSRendererImplementation();
}

ADSRenderer::~ADSRenderer()
{
    delete _implementation;
}

void ADSRenderer::Use()
{
    _implementation->Use();
}

IndexValue ADSRenderer::RegisterMaterial(MaterialInfo material)
{
    return _implementation->RegisterMaterial(material);
}

void ADSRenderer::SetModelMatrix(const glm::mat4 &matrix)
{
    _implementation->SetModelMatrix(matrix);
}

void ADSRenderer::SetViewMatrix(const glm::mat4 &matrix)
{
    _implementation->SetViewMatrix(matrix);
}

void ADSRenderer::SetProjectionMatrix(const glm::mat4 &matrix)
{
    _implementation->SetProjectionMatrix(matrix);
}

void ADSRenderer::SetLight(LightInfo info)
{
    _implementation->SetLight(info);
}

void ADSRenderer::Render(const std::vector<IndexValue> &indices,
                         const std::vector<float> &vertices,
                         const std::vector<float> &normals,
                         const std::vector<float> &UVs,
                         const IndexValue &materialId)
{
    _implementation->Render(indices, vertices, normals, UVs, materialId);
}
