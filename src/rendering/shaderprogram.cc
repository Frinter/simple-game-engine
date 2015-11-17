#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <GL/gl_core_3_3.h>
#include <glm/gtc/type_ptr.hpp>

#include "shaderprogram.hh"

std::string GetShaderLog(GLuint shaderHandle)
{
    std::string log;
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

std::string ReadFile(const std::string &filename)
{
    std::FILE *file = std::fopen(filename.c_str(), "rb");

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

    std::string str = contents;
    free(contents);
    
    return str;
}

GLuint CreateShaderFromSource(GLenum shaderType, const char *source)
{
    GLuint handle;
    handle = glCreateShader(shaderType);

    glShaderSource(handle, 1, &source, NULL);
    glCompileShader(handle);
	
    GLint compileResult;
	
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE)
    {
        std::cout << GetShaderLog(handle) << std::endl;
        std::cout << "Source:" << std::endl
                  << source
                  << std::endl;
        
        exit(1);
    }

    return handle;
}

GLuint CreateShader(GLenum type, const std::string &fileName)
{
    std::string shaderPath = std::string("shaders/") + fileName;
    std::string shaderSource = ReadFile(shaderPath);
    return CreateShaderFromSource(type, shaderSource.c_str());
}

ShaderProgram::ShaderProgram(const char *name)
{
    std::string vertexShaderFileName = std::string(name) + ".vert";
    _vertexShaderHandle = CreateShader(GL_VERTEX_SHADER, vertexShaderFileName);

    std::string fragmentShaderFileName = std::string(name) + ".frag";
    _fragmentShaderHandle = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderFileName);

    _shaderProgramHandle = glCreateProgram();

    glAttachShader(_shaderProgramHandle, _vertexShaderHandle);
    glAttachShader(_shaderProgramHandle, _fragmentShaderHandle);
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::Link()
{
    glLinkProgram(_shaderProgramHandle);
}

void ShaderProgram::Use()
{
    glUseProgram(_shaderProgramHandle);
}

GLuint ShaderProgram::GenVertexArrayObject()
{
    GLuint vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    return vaoHandle;
}

void ShaderProgram::BindVertexArrayObject(GLuint vaoHandle)
{
    glBindVertexArray(vaoHandle);
}

void ShaderProgram::BindAttribLocation(unsigned int location, const char *name)
{
    glBindAttribLocation(_shaderProgramHandle, location, name);
}

void ShaderProgram::EnableAttribArray(unsigned int location)
{
    glEnableVertexAttribArray(location);    
}

GLuint ShaderProgram::GenBuffer()
{
    GLuint bufferHandle;
    glGenVertexArrays(1, &bufferHandle);
    return bufferHandle;
}

void ShaderProgram::BindBuffer(GLenum targetType, GLuint bufferHandle)
{
    glBindBuffer(targetType, bufferHandle);
}

GLuint ShaderProgram::RegisterTexture(RawImageInfo *imageInfo)
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

void ShaderProgram::BindAttribPointer(GLuint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride)
{
    glVertexAttribPointer(location, size, type, normalized, stride, NULL);
}

void ShaderProgram::BufferData(GLenum targetType, const int size, const void *data)
{
    glBufferData(targetType, size, data, GL_STATIC_DRAW);
}

GLuint ShaderProgram::GetUniformLocation(const char *uniformName)
{
    return glGetUniformLocation(_shaderProgramHandle, uniformName);
}

void ShaderProgram::SetUniform(const char *uniformName, const int info)
{
    GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
    glUniform1i(uniformLocation, info);
}

void ShaderProgram::SetUniform(const char *uniformName, const float *info)
{
    GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
    glUniform3fv(uniformLocation, 1, info);
}

void ShaderProgram::SetUniform(const char *uniformName, const glm::vec3 &info)
{
    GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
    glUniform3fv(uniformLocation, 1, glm::value_ptr(info));
}

void ShaderProgram::SetUniform(const char *uniformName, const glm::vec4 &info)
{
    GLuint uniformLocation = glGetUniformLocation(_shaderProgramHandle, uniformName);
    glUniform4fv(uniformLocation, 1, glm::value_ptr(info));
}

