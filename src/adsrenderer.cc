#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/gl_core_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "adsrenderer.hh"
#include "types.hh"

using std::cout;
using std::endl;
using std::string;
using std::vector;

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

    IndexValue RegisterDataCollection(vector<T> vertexData)
    {
        IndexValue newIndex = _vertexDataCollections.size();
        _vertexDataCollections.push_back(vertexData);
        return newIndex;
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
        _vertexShaderHandle = CreateShaderFromSource(GL_VERTEX_SHADER, "shaders/ads.vert");
        _fragmentShaderHandle = CreateShaderFromSource(GL_FRAGMENT_SHADER, "shaders/ads.frag");

        _shaderProgramHandle = glCreateProgram();

        glEnable(GL_DEPTH_TEST);

        glAttachShader(_shaderProgramHandle, _vertexShaderHandle);
        glAttachShader(_shaderProgramHandle, _fragmentShaderHandle);

        glBindAttribLocation(_shaderProgramHandle, 0, "VertexPosition");
        glBindAttribLocation(_shaderProgramHandle, 1, "VertexNormal");
        glBindAttribLocation(_shaderProgramHandle, 2, "VertexTexCoord");

        glLinkProgram(_shaderProgramHandle);

        Use();

        glGenVertexArrays(1, &_vaoHandle);
        glBindVertexArray(_vaoHandle);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        _projectionMatrix = glm::perspective(45.0f, 4.0f/3.0f, 1.0f, 100.0f);

        _positionBuffer.SetUp(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
        _normalBuffer.SetUp(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
        _uvBuffer.SetUp(2, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

        _modelViewMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "ModelViewMatrix");
        _normalMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "NormalMatrix");
        _projectionMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "ProjectionMatrix");
        _MVPMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "MVP");
    }
    
    void Use()
    {
        glUseProgram(_shaderProgramHandle);
    }

    Model CreateModelFromImporter(ADSRenderer::Importer &importer);
    
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

    void SetModelMatrix(glm::mat4 matrix)
    {
        _modelMatrix = matrix;

        _modelViewMatrix = _viewMatrix * _modelMatrix;
        _normalMatrix = glm::transpose(glm::inverse(glm::mat3(_modelViewMatrix)));
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;
    }

    void SetViewMatrix(glm::mat4 matrix)
    {
        _viewMatrix = matrix;

        _modelViewMatrix = _viewMatrix * _modelMatrix;
        _normalMatrix = glm::transpose(glm::inverse(glm::mat3(_modelViewMatrix)));
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;
    }

    void SetLight(LightInfo info)
    {
        _light.position = info.position;
        _light.La = info.La;
        _light.Ld = info.Ld;
        _light.Ls = info.Ls;

        glm::vec4 viewLightPosition = _modelViewMatrix * _light.position;

        setUniform("Light.Position", &viewLightPosition);
        setUniform("Light.La", glm::value_ptr(_light.La));
        setUniform("Light.Ld", glm::value_ptr(_light.Ld));
        setUniform("Light.Ls", glm::value_ptr(_light.Ls));
    }

    void Render(const Model &model)
    {
        glBindVertexArray(_vaoHandle);

        glUniformMatrix4fv(_modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
        glUniformMatrix3fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));
        glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(_projectionMatrix));
        glUniformMatrix4fv(_MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(_MVPMatrix));

        const std::vector<Model::RenderUnit> renderUnits = model.GetRenderUnits();

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
    
    GLuint _vertexShaderHandle;
    GLuint _fragmentShaderHandle;
    GLuint _shaderProgramHandle;
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

        setUniform("Material.Ka", glm::value_ptr(_materials[index].Ka));
        setUniform("Material.Kd", glm::value_ptr(_materials[index].Kd));
        setUniform("Material.Ks", glm::value_ptr(_materials[index].Ks));
        setUniform("Material.Shininess", &_materials[index].shininess);
        setUniform("Material.Kd_map", 0);
        setUniform("Material.hasKdMap", (int)_materials[index].hasKdMap);
    }

    void setUniform(const char *uniformName, const int info)
    {
        GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
        glUniform1i(uniformLocation, info);
    }

    void setUniform(const char *uniformName, const float *info)
    {
        GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
        glUniform3fv(uniformLocation, 1, info);
    }

    void setUniform(const char *uniformName, const glm::vec3 *info)
    {
        GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
        glUniform3fv(uniformLocation, 1, glm::value_ptr(*info));
    }

    void setUniform(const char *uniformName, const glm::vec4 *info)
    {
        GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
        glUniform4fv(uniformLocation, 1, glm::value_ptr(*info));
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

Model ADSRendererImplementation::CreateModelFromImporter(ADSRenderer::Importer &importer)
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

    return Model(renderUnits);
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

Model ADSRenderer::CreateModelFromImporter(ADSRenderer::Importer &importer)
{
    return _implementation->CreateModelFromImporter(importer);
}

void ADSRenderer::SetModelMatrix(glm::mat4 matrix)
{
    _implementation->SetModelMatrix(matrix);
}

void ADSRenderer::SetViewMatrix(glm::mat4 matrix)
{
    _implementation->SetViewMatrix(matrix);    
}

void ADSRenderer::SetLight(LightInfo info)
{
    _implementation->SetLight(info);
}

void ADSRenderer::Render(const Model &model)
{
    _implementation->Render(model);    
}

