#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "framework/platform.hh"
#include "GL/gl_core_3_3.h"
#include "model.hh"
#include "objparser.hh"
#include "renderer.hh"
#include "sleepservice.hh"
#include "systemtimer.hh"
#include "ticker.hh"

using std::string;
using std::vector;
using std::unordered_map;

using std::cout;
using std::endl;

typedef struct LightInfo
{
    glm::vec4 position;
    glm::vec3 La;
    glm::vec3 Ld;
    glm::vec3 Ls;
} LightInfo;

typedef struct MaterialInfo
{
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    float shininess;
} MaterialInfo;

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

class ADSRenderer : public IRenderer
{
public:
    ADSRenderer()
        : _indexBuffer(GL_ELEMENT_ARRAY_BUFFER), _positionBuffer(GL_ARRAY_BUFFER), _normalBuffer(GL_ARRAY_BUFFER)
    {
        _vertexShaderHandle = CreateShaderFromSource(GL_VERTEX_SHADER, "shaders/ads.vert");
        _fragmentShaderHandle = CreateShaderFromSource(GL_FRAGMENT_SHADER, "shaders/ads.frag");

        _shaderProgramHandle = glCreateProgram();

        glAttachShader(_shaderProgramHandle, _vertexShaderHandle);
        glAttachShader(_shaderProgramHandle, _fragmentShaderHandle);

        glBindAttribLocation(_shaderProgramHandle, 0, "VertexPosition");
        glBindAttribLocation(_shaderProgramHandle, 1, "VertexNormal");

        glLinkProgram(_shaderProgramHandle);

        Use();

        glGenVertexArrays(1, &_vaoHandle);
        glBindVertexArray(_vaoHandle);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        _modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        _viewMatrix = glm::lookAt(glm::vec3(2.0,2.0,1.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        _projectionMatrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
        _modelViewMatrix = _viewMatrix * _modelMatrix;
        _MVPMatrix = _projectionMatrix * _modelViewMatrix;

        _positionBuffer.SetUp(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
        _normalBuffer.SetUp(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

        _modelViewMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "ModelViewMatrix");
        _normalMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "NormalMatrix");
        _projectionMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "ProjectionMatrix");
        _MVPMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "MVP");

        _light.position = glm::vec4(0.3, 0.0, 1.0, 1.0);
        _light.La = glm::vec3(1.0, 1.0, 1.0);
        _light.Ld = glm::vec3(1.0, 1.0, 1.0);
        _light.Ls = glm::vec3(1.0, 1.0, 1.0);

        _material.Ka = glm::vec3(1.0, 0.0, 1.0);
        _material.Kd = glm::vec3(1.0, 1.0, 1.0);
        _material.Ks = glm::vec3(0.0, 0.0, 1.0);
        _material.shininess = 0.5;

        setUniform("Material.Ka", &_material.Ka);
        setUniform("Material.Kd", &_material.Kd);
        setUniform("Material.Ks", &_material.Ks);
        setUniform("Material.Shininess", &_material.shininess);
        
        setUniform("Light.Position", &_light.position);
        setUniform("Light.La", &_light.La);
        setUniform("Light.Ld", &_light.Ld);
        setUniform("Light.Ls", &_light.Ls);
    }
    
    void Use()
    {
        glUseProgram(_shaderProgramHandle);
    }

    IndexValue RegisterIndexCollection(vector<IndexValue> indices)
    {
        return _indexBuffer.RegisterDataCollection(indices);
    }
    
    IndexValue RegisterVertexCollection(vector<float> vertices)
    {
        return _positionBuffer.RegisterDataCollection(vertices);
    }
    
    IndexValue RegisterNormalCollection(vector<float> normals)
    {
        return _normalBuffer.RegisterDataCollection(normals);
    }

    void Render(const Model &model)
    {
        glBindVertexArray(_vaoHandle);

        _indexBuffer.UseDataCollection(model.GetVertexIndicesId());
        _positionBuffer.UseDataCollection(model.GetVertexPositionsId());
        _normalBuffer.UseDataCollection(model.GetVertexNormalsId());

        glUniformMatrix4fv(_modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
        glUniformMatrix4fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));
        glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(_projectionMatrix));
        glUniformMatrix4fv(_MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(_MVPMatrix));

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
    }

private:
    LightInfo _light;
    MaterialInfo _material;
    
    VertexArrayBuffer<IndexValue> _indexBuffer;
    VertexArrayBuffer<float> _positionBuffer;
    VertexArrayBuffer<float> _normalBuffer;
    
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

class ObjImporter
{
public:
    ObjImporter(ObjParser::IParseResult *result)
        : _parseResult(result)
    {
    }

    vector<float> GetVertices()
    {
        vector<float> processedVertices;
        vector<ObjParser::Vertex> vertices = _parseResult->GetVertices();

        for (int i = 0; i < vertices.size(); ++i)
        {
            ObjParser::Vertex vertex = vertices[i];
            for (int j = 0; j < 4; ++j)
            {
                processedVertices.push_back(vertex.coordinates[j]);
            }
        }

        return processedVertices;
    }

    vector<float> GetNormals()
    {
        vector<float> processedNormals;
        vector<ObjParser::Face> faces = _parseResult->GetFaces();
        vector<ObjParser::Normal> normals = _parseResult->GetNormals();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];
            for (int j = 0; j < face.normalIndices.size(); ++j)
            {
                ObjParser::Normal normal = normals[face.normalIndices[j]];
                for (int jj = 0; jj < 3; ++jj)
                {
                    processedNormals.push_back(normal.coordinates[jj]);
                }
            }
        }

        return processedNormals;
    }

    vector<IndexValue> GetIndices()
    {
        vector<IndexValue> processedIndices;
        vector<ObjParser::Face> faces = _parseResult->GetFaces();

        for (int i = 0; i < faces.size(); ++i)
        {
            ObjParser::Face face = faces[i];
            for (int j = 0; j < 3; ++j)
            {
                processedIndices.push_back(face.vertexIndices[j]);
            }
        }

        return processedIndices;
    }

private:
    ObjParser::IParseResult *_parseResult;
};

GraphicsThreadEntry_FunctionSignature(GraphicsThreadEntry)
{
    windowController->CreateContext();

    SystemTimer systemTimer(applicationContext->GetSystemUtility());
    SleepService sleepService(applicationContext->GetSystemUtility());
    Ticker ticker = Ticker(&systemTimer, &sleepService);

    // Load 3d assets
    ObjParser::ObjFileParser parser("assets/", "simple.obj");
    ObjParser::IParseResult *parseResult = parser.Parse();
    ObjImporter importer(parseResult);
    
    ADSRenderer *adsRenderer = new ADSRenderer();
    IndexValue vertexCollectionId = adsRenderer->RegisterVertexCollection(importer.GetVertices());
    IndexValue normalCollectionId = adsRenderer->RegisterNormalCollection(importer.GetNormals());
    IndexValue vertexIndicesId = adsRenderer->RegisterIndexCollection(importer.GetIndices());
    Model simpleModel(vertexCollectionId, normalCollectionId, vertexIndicesId);

    IRenderer *renderer = (IRenderer *)adsRenderer;

    ticker.Start(17);
    
    while (!applicationContext->IsClosing())
    {
        renderer->Render(simpleModel);
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
