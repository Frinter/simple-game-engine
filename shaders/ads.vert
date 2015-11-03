#version 330

struct MaterialInfo
{
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Shininess;
};

struct LightInfo
{
    vec4 Position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};

uniform LightInfo Light;
uniform MaterialInfo Material;

in vec3 VertexPosition;
in vec3 VertexNormal;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

out vec3 LightIntensity;

void main()
{
    vec3 tnorm = normalize(NormalMatrix * VertexNormal);
    vec4 eyeCoords = ModelViewMatrix * vec4(VertexPosition, 1.0);
    vec3 s = normalize(vec3(Light.Position - eyeCoords));
    vec3 v = normalize(-eyeCoords.xyz);
    vec3 r = reflect(-s, tnorm);
    vec3 ambient = Light.La * Material.Ka;

    float sDotN = max(dot(s, tnorm), 0.0);
    vec3 diffuse = Light.Ld * Material.Kd * sDotN;
    vec3 spec = vec3(0.0);
    if (sDotN > 0.0)
        spec = Light.Ls * Material.Ks * pow(max(dot(r,v), 0.0), Material.Shininess);

    LightIntensity = ambient + diffuse + spec;

    gl_Position = MVP * vec4(VertexPosition, 1.0);
}