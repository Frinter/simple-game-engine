#version 330

in vec4 VertexPosition;
in vec3 VertexColor;

out vec3 Color;

uniform mat4 RotationMatrix;

void main()
{
    Color = VertexColor;
    gl_Position = RotationMatrix * VertexPosition;
}
