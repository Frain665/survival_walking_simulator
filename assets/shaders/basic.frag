#version 330 core

in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;

void main()
{
    vec3 norm     = normalize(vNormal);
    vec3 lightDir = normalize(-uLightDir);

    float diff    = max(dot(norm, lightDir), 0.0);
    vec3  diffuse = diff * uLightColor;

    vec3 result = (uAmbient + diffuse) * uColor;
    FragColor   = vec4(result, 1.0);
}
