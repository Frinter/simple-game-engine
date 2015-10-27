#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "framework/platform.hh"
#include "GL/gl_core_3_3.h"
#include "model.hh"
#include "sleepservice.hh"
#include "systemtimer.hh"
#include "ticker.hh"

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

class IRenderer
{
public:
    virtual void Use() = 0;
    virtual void Render(const Model &model) = 0;
};

class BasicRenderer : public IRenderer
{
public:
    BasicRenderer()
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
        
        GLuint vboHandles[3];
        glGenBuffers(3, vboHandles);
        _positionBufferHandle = vboHandles[0];
        _colorBufferHandle = vboHandles[1];
        _indexBufferHandle = vboHandles[2];

        glGenVertexArrays(1, &_vaoHandle);
        glBindVertexArray(_vaoHandle);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, _positionBufferHandle);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
    
        glBindBuffer(GL_ARRAY_BUFFER, _colorBufferHandle);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

        _rotationMatrixLocation = glGetUniformLocation(_shaderProgramHandle, "RotationMatrix");
    }

    void Use()
    {
        glUseProgram(_shaderProgramHandle);
    }
    
    void Render(const Model &model)
    {
        glBindVertexArray(_vaoHandle);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.GetVertexIndices().size() * sizeof(IndexValue), model.GetVertexIndices().data(), GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, _positionBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, model.GetVertexPositions().size() * sizeof(float), model.GetVertexPositions().data(), GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, _colorBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, model.GetVertexColors().size() * sizeof(float), model.GetVertexColors().data(), GL_STATIC_DRAW);

        glUniformMatrix4fv(_rotationMatrixLocation, 1, GL_FALSE, _rotationMatrix);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
    }

private:
    GLuint _positionBufferHandle;
    GLuint _colorBufferHandle;
    GLuint _indexBufferHandle;

    GLuint _vertexShaderHandle;
    GLuint _fragmentShaderHandle;
    GLuint _shaderProgramHandle;
    GLuint _vaoHandle;
    GLuint _rotationMatrixLocation;

private:
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
            GLint logLength;
		
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0)
            {
                char *log = (char *)malloc(logLength);
                GLsizei written;
                glGetShaderInfoLog(handle, logLength, &written, log);
                std::cout << log << std::endl;
                free(log);
            }
		
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

    Model simpleModel("simple.obj");
    
    Model model(positionData, colorData, indexData);

    IRenderer *renderer = new BasicRenderer();

    ticker.Start(17);
    
    while (!applicationContext->IsClosing())
    {
        renderer->Render(simpleModel);
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
