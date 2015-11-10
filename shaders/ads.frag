#version 330

struct LightInfo
{
    vec4 Position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};

struct MaterialInfo
{
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Shininess;
    sampler2D Kd_map;
    bool hasKdMap;
};

uniform LightInfo Light;
uniform MaterialInfo Material;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

void phongModel(vec3 position, vec3 normal,
                out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    ambient = Light.La * Material.Ka;

    vec3 n = normalize(normal);
    vec3 s = normalize(vec3(Light.Position.xyz - position));
    float sDotN = max(dot(s, n), 0.0);
    diffuse = Light.Ld * Material.Kd * sDotN;

    vec3 v = normalize(-position);
    vec3 r = reflect(-s, n);
    vec3 spec = vec3(0.0);
    if (sDotN > 0.0)
        spec = Light.Ls * Material.Ks * pow(max(dot(r,v), 0.0), Material.Shininess);

    specular = spec;
}

void main()
{
    vec3 ambient, diffuse, specular;
    vec4 texColor;
    phongModel(Position, Normal, ambient, diffuse, specular);
    if (Material.hasKdMap)
    {
        texColor = texture(Material.Kd_map, TexCoord);
    }
    else
    {
        texColor = vec4(1.0);
    }
    FragColor = vec4(ambient + diffuse, 1.0) * texColor + vec4(specular, 1.0);
}