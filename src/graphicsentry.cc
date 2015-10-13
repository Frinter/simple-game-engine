#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

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

float positionData[] = {
    -0.8f, -0.8f,  0.0f,
     0.8f, -0.8f,  0.0f,
     0.0f,  0.8f,  0.0f    
};

float colorData[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

GraphicsThreadEntry_FunctionSignature(GraphicsThreadEntry)
{
    windowController->CreateContext();

    // Load 3d assets
    GLuint vertexShaderHandle = CreateShaderFromSource(GL_VERTEX_SHADER, "shaders/basic.vert");
    GLuint fragmentShaderHandle = CreateShaderFromSource(GL_FRAGMENT_SHADER, "shaders/basic.frag");

    GLuint shaderProgramHandle = glCreateProgram();

    glAttachShader(shaderProgramHandle, vertexShaderHandle);
    glAttachShader(shaderProgramHandle, fragmentShaderHandle);

    glBindAttribLocation(shaderProgramHandle, 0, "VertexPosition");
    glBindAttribLocation(shaderProgramHandle, 1, "VertexColor");
    
    glLinkProgram(shaderProgramHandle);

    glUseProgram(shaderProgramHandle);

    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint colorBufferHandle = vboHandles[1];

    GLuint vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

    while (!applicationContext->IsClosing())
    {
        glBindVertexArray(vaoHandle);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        windowController->SwapBuffers();
    }
}
