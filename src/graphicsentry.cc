#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "framework/platform.hh"
#include "GL/gl_core_3_3.h"

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

GLubyte _indexData[] = { 0, 1, 2 };

GraphicsThreadEntry_FunctionSignature(GraphicsThreadEntry)
{
    windowController->CreateContext();
    
    // Load 3d assets
    std::vector<float> positionData, colorData;
    positionData.assign(_positionData, _positionData+9);
    colorData.assign(_colorData, _colorData+9);

    std::vector<GLubyte> indexData;
    indexData.assign(_indexData, _indexData+3);
    
    GLuint vertexShaderHandle = CreateShaderFromSource(GL_VERTEX_SHADER, "shaders/basic.vert");
    GLuint fragmentShaderHandle = CreateShaderFromSource(GL_FRAGMENT_SHADER, "shaders/basic.frag");

    GLuint shaderProgramHandle = glCreateProgram();

    glAttachShader(shaderProgramHandle, vertexShaderHandle);
    glAttachShader(shaderProgramHandle, fragmentShaderHandle);

    glBindAttribLocation(shaderProgramHandle, 0, "VertexPosition");
    glBindAttribLocation(shaderProgramHandle, 1, "VertexColor");
    
    glLinkProgram(shaderProgramHandle);

    glUseProgram(shaderProgramHandle);

    GLuint vboHandles[3];
    glGenBuffers(3, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint colorBufferHandle = vboHandles[1];
    GLuint indexBufferHandle = vboHandles[2];

    GLuint vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size(), indexData.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(float), positionData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(float), colorData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

    while (!applicationContext->IsClosing())
    {
        glBindVertexArray(vaoHandle);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, NULL);
        windowController->SwapBuffers();
    }
}
