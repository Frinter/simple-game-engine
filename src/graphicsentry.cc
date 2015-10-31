#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

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

float _rotationMatrix[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

float _positionData[] = {
    -0.8f, -0.8f,  0.0f,
     0.8f, -0.8f,  0.0f,
     0.0f,  0.8f,  0.0f    
};

float _colorData[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

IndexValue _indexData[] = { 0, 1, 2 };

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
        glBufferData(_targetType, vertexData->size() * sizeof(float), vertexData->data(), GL_STATIC_DRAW);
    }
};

class BasicRenderer : public IRenderer
{
public:
    BasicRenderer()
        : _indexBuffer(GL_ELEMENT_ARRAY_BUFFER), _positionBuffer(GL_ARRAY_BUFFER)
    {
        _vertexShaderHandle = CreateShaderFromSource(GL_VERTEX_SHADER, "shaders/basic.vert");
        _fragmentShaderHandle = CreateShaderFromSource(GL_FRAGMENT_SHADER, "shaders/basic.frag");

        _shaderProgramHandle = glCreateProgram();

        glAttachShader(_shaderProgramHandle, _vertexShaderHandle);
        glAttachShader(_shaderProgramHandle, _fragmentShaderHandle);

        glBindAttribLocation(_shaderProgramHandle, 0, "VertexPosition");
        glBindAttribLocation(_shaderProgramHandle, 1, "VertexColor");

        glLinkProgram(_shaderProgramHandle);

        Use();

        GLuint vboHandles[2];
        glGenBuffers(2, vboHandles);
        _colorBufferHandle = vboHandles[0];
        _indexBufferHandle = vboHandles[1];

        glGenVertexArrays(1, &_vaoHandle);
        glBindVertexArray(_vaoHandle);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        _positionBuffer.SetUp(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
    
        glBindBuffer(GL_ARRAY_BUFFER, _colorBufferHandle);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

        _rotationMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "RotationMatrix");
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
    
    void Render(const Model &model)
    {
        glBindVertexArray(_vaoHandle);

        _indexBuffer.UseDataCollection(model.GetVertexIndicesId());
        _positionBuffer.UseDataCollection(model.GetVertexPositionsId());
    
        glBindBuffer(GL_ARRAY_BUFFER, _colorBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, model.GetVertexColors().size() * sizeof(float), model.GetVertexColors().data(), GL_STATIC_DRAW);

        glUniformMatrix4fv(_rotationMatrixLocation, 1, GL_FALSE, _rotationMatrix);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
    }

private:
    VertexArrayBuffer<IndexValue> _indexBuffer;
    VertexArrayBuffer<float> _positionBuffer;
    
    GLuint _colorBufferHandle;
    GLuint _indexBufferHandle;

    GLuint _vertexShaderHandle;
    GLuint _fragmentShaderHandle;
    GLuint _shaderProgramHandle;
    GLuint _vaoHandle;
    GLuint _rotationMatrixLocation;

    vector< vector<float> > _vertexCollections;

private:
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

GraphicsThreadEntry_FunctionSignature(GraphicsThreadEntry)
{
    windowController->CreateContext();

    SystemTimer systemTimer(applicationContext->GetSystemUtility());
    SleepService sleepService(applicationContext->GetSystemUtility());
    Ticker ticker = Ticker(&systemTimer, &sleepService);

    // Load 3d assets
    std::vector<float> positionData, colorData;
    positionData.assign(_positionData, _positionData+9);
    colorData.assign(_colorData, _colorData+9);

    std::vector<IndexValue> indexData;
    indexData.assign(_indexData, _indexData+3);

    ObjFileParser parser("assets/", "simple.obj");
    parser.Parse();
    
    BasicRenderer *basicRenderer = new BasicRenderer();
    IndexValue vertexCollectionId = basicRenderer->RegisterVertexCollection(parser.GetVertices());
    IndexValue vertexIndicesId = basicRenderer->RegisterIndexCollection(parser.GetIndices());
    Model simpleModel(vertexCollectionId, colorData, vertexIndicesId);

    IRenderer *renderer = (IRenderer *)basicRenderer;

    ticker.Start(17);
    
    while (!applicationContext->IsClosing())
    {
        renderer->Render(simpleModel);
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
