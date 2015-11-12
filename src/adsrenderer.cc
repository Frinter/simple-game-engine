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

class TileMap
{
public:
    TileMap(RawImageInfo *tilemapImage, GLuint textureId, unsigned int tileWidth, unsigned int tileHeight)
        : _tilemapImage(tilemapImage), _textureId(textureId), _tileWidth(tileWidth), _tileHeight(tileHeight)
    {
    }

    GLuint GetTextureId() const
    {
        return _textureId;
    }

    vector<float> GetUVsForTile(IndexValue x, IndexValue y)
    {
        vector<float> UVs;

        unsigned int top, bottom, left, right;
        top = y * _tileHeight;
        bottom = (y + 1) * _tileHeight - 1;
        left = x * _tileWidth;
        right = (x + 1) * _tileWidth - 1;

        float UVtop, UVbottom, UVleft, UVright;
        UVleft = (float) left / _tilemapImage->width;
        UVright = (float) right / _tilemapImage->width;
        UVtop = (float) top / _tilemapImage->height;
        UVbottom = (float) bottom / _tilemapImage->height;

        UVs.push_back(UVright);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVbottom);

        UVs.push_back(UVright);
        UVs.push_back(UVtop); 
        UVs.push_back(UVleft);
        UVs.push_back(UVbottom);
        UVs.push_back(UVright);
        UVs.push_back(UVbottom);

        return UVs;
    }

private:
    RawImageInfo *_tilemapImage;
    GLuint _textureId;
    unsigned int _tileWidth;
    unsigned int _tileHeight;
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

    Model *CreateModelFromImporter(IRenderer::Importer &importer);
    
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

    IndexValue RegisterTileMap(RawImageInfo *tilemapImage, unsigned int tileWidth, unsigned int tileHeight)
    {
        GLuint textureId = registerTexture(tilemapImage);
        TileMap map(tilemapImage, textureId, tileWidth, tileHeight);

        IndexValue tileMapId = _tileMaps.size();
        _tileMaps.push_back(map);
        return tileMapId;
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

    void RenderTile(IndexValue tilemapId, IndexValue x, IndexValue y)
    {
        glBindVertexArray(_vaoHandle);

        glUniformMatrix4fv(_modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
        glUniformMatrix3fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));
        glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(_projectionMatrix));
        glUniformMatrix4fv(_MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(_MVPMatrix));

        _indexBuffer.UseDataCollection(_imageIndexIndex);
        _positionBuffer.UseDataCollection(_imageVertexIndex);
        _normalBuffer.UseDataCollection(_imageNormalIndex);
        _uvBuffer.UseDataCollection(_tileMaps[tilemapId].GetUVsForTile(x, y));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _tileMaps[tilemapId].GetTextureId());

        _shaderProgram->SetUniform("Material.Ka", glm::vec3(0.0f, 0.0f, 0.0f));
        _shaderProgram->SetUniform("Material.Kd", glm::vec3(1.0f, 1.0f, 1.0f));
        _shaderProgram->SetUniform("Material.Ks", glm::vec3(0.0f, 0.0f, 0.0f));
        _shaderProgram->SetUniform("Material.Shininess", 1.0f);
        _shaderProgram->SetUniform("Material.Kd_map", 0);
        _shaderProgram->SetUniform("Material.hasKdMap", (int)true);

        glDrawArrays(GL_TRIANGLES, 0, _indexBuffer.currentSize());
    }

    void Render(const Model *model)
    {
        glBindVertexArray(_vaoHandle);

        glUniformMatrix4fv(_modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
        glUniformMatrix3fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));
        glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(_projectionMatrix));
        glUniformMatrix4fv(_MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(_MVPMatrix));

        const std::vector<Model::RenderUnit> renderUnits = model->GetRenderUnits();

        for (int i = 0; i < renderUnits.size(); ++i)
        {
            Model::RenderUnit renderUnit = renderUnits[i];

            _indexBuffer.UseDataCollection(renderUnit.GetVertexIndicesId());
            _positionBuffer.UseDataCollection(renderUnit.GetVertexPositionsId());
            _normalBuffer.UseDataCollection(renderUnit.GetVertexNormalsId());
            _uvBuffer.UseDataCollection(renderUnit.GetUVCoordsId());
            useMaterial(renderUnit.GetMaterialId());

            glDrawArrays(GL_TRIANGLES, 0, _indexBuffer.currentSize());
        }
    }

private:
    LightInfo _light;
    
    VertexArrayBuffer<IndexValue> _indexBuffer;
    VertexArrayBuffer<float> _positionBuffer;
    VertexArrayBuffer<float> _normalBuffer;
    VertexArrayBuffer<float> _uvBuffer;

    vector<MaterialInfo> _materials;
    vector<TileMap> _tileMaps;

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
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        return textureId;
    }

    void useMaterial(IndexValue index)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _materials[index].Kd_mapId);

        _shaderProgram->SetUniform("Material.Ka", glm::value_ptr(_materials[index].Ka));
        _shaderProgram->SetUniform("Material.Kd", glm::value_ptr(_materials[index].Kd));
        _shaderProgram->SetUniform("Material.Ks", glm::value_ptr(_materials[index].Ks));
        _shaderProgram->SetUniform("Material.Shininess", &_materials[index].shininess);
        _shaderProgram->SetUniform("Material.Kd_map", 0);
        _shaderProgram->SetUniform("Material.hasKdMap", (int)_materials[index].hasKdMap);
    }

    string GetShaderLog(GLuint shaderHandle)
    {
        string log;
        GLint logLength;
		
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            char *logBuffer = (char *)malloc(logLength);
            GLsizei written;
            glGetShaderInfoLog(shaderHandle, logLength, &written, logBuffer);
            log = logBuffer;
            free(logBuffer);
        }

        return log;
    }
    
    GLuint CreateShaderFromSource(GLenum shaderType, const char *sourceFilename)
    {
        char *source = ReadFile(sourceFilename);
    
        GLuint handle;
        handle = glCreateShader(shaderType);

        glShaderSource(handle, 1, &source, NULL);
        glCompileShader(handle);
	
        GLint compileResult;
	
        glGetShaderiv(handle, GL_COMPILE_STATUS, &compileResult);
        if (compileResult == GL_FALSE)
        {
            cout << GetShaderLog(handle) << endl;
            exit(1);
        }

        return handle;
    }

    char *ReadFile(const char *filename)
    {
        std::FILE *file = std::fopen(filename, "rb");

        if (file == NULL)
            throw errno;

        char *contents;
        int fileSize;

        std::fseek(file, 0, SEEK_END);
        fileSize = std::ftell(file);
        std::rewind(file);
        contents = (char *)std::malloc(fileSize + 1);
        std::fread(contents, 1, fileSize, file);
        contents[fileSize] = 0;
        std::fclose(file);

        return contents;
    }
};

Model *ADSRendererImplementation::CreateModelFromImporter(IRenderer::Importer &importer)
{
    vector<Model::RenderUnit> renderUnits;
    vector<RenderObject> renderObjects = importer.GetRenderObjects();

    for (int i = 0; i < renderObjects.size(); ++i)
    {
        RenderObject object = renderObjects[i];

        IndexValue vertexCollectionId = _positionBuffer.RegisterDataCollection(object._vertices);
        IndexValue normalCollectionId = _normalBuffer.RegisterDataCollection(object._normals);
        IndexValue uvCollectionId = _uvBuffer.RegisterDataCollection(object._uvCoords);
        IndexValue vertexIndicesId = _indexBuffer.RegisterDataCollection(object._indices);
        IndexValue materialId = RegisterMaterial(importer.GetMaterial(object._materialName));

        renderUnits.push_back(Model::RenderUnit(vertexCollectionId, normalCollectionId,
                                                vertexIndicesId, materialId, uvCollectionId));
    }

    return new Model(renderUnits);
}

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

Model *ADSRenderer::CreateModelFromImporter(IRenderer::Importer &importer)
{
    return _implementation->CreateModelFromImporter(importer);
}

IndexValue ADSRenderer::RegisterTileMap(RawImageInfo *tilemapImage, unsigned int tileWidth, unsigned int tileHeight)
{
    return _implementation->RegisterTileMap(tilemapImage, tileWidth, tileHeight);
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

void ADSRenderer::RenderTile(IndexValue tilemapId, IndexValue x, IndexValue y)
{
    _implementation->RenderTile(tilemapId, x, y);
}

void ADSRenderer::Render(const Model *model)
{
    _implementation->Render(model);    
}

