#pragma once

#include <vector>

#include <GL/gl_core_3_3.h>
#include <glm/glm.hpp>

#include "types.hh"

class ShaderProgram
{
public:
    ShaderProgram(const char *name);
    ~ShaderProgram();

    void Link();
    void Use();
    
    GLuint GenVertexArrayObject();
    void BindVertexArrayObject(GLuint vaoHandle);
    void BindAttribLocation(GLuint location, const char *name);
    void EnableAttribArray(GLuint location);
    
    GLuint GenBuffer();
    void BindBuffer(GLenum targetType, GLuint bufferHandle);
    void BindAttribPointer(GLuint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
    void BufferData(GLenum targetType, const int size, const void *data);

    GLuint RegisterTexture(RawImageInfo *imageInfo);
    
    GLuint GetUniformLocation(const char *uniformName);
    void SetUniform(const char *uniformName, const int info);
    void SetUniform(const char *uniformName, const float *info);
    void SetUniform(const char *uniformName, const glm::vec3 &info);
    void SetUniform(const char *uniformName, const glm::vec4 &info);
    
private:
    GLuint _vertexShaderHandle;
    GLuint _fragmentShaderHandle;
    GLuint _shaderProgramHandle;
};
