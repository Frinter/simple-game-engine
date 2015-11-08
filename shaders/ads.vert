#version 330

in vec4 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    TexCoord = VertexTexCoord;
    Normal = normalize(NormalMatrix * VertexNormal);
    vec4 tmp = ModelViewMatrix * VertexPosition;
    Position = vec3(tmp);

    gl_Position = MVP * VertexPosition;
}