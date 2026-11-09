#version 460 core

in  vec3 normal;
in  vec3 worldPosition;
in  vec2 uv;

layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
    vec3 u_cameraPosition;
};
layout(std140, binding = 1) uniform Light {
    vec3 u_lightDirection;   // offset 0, 12 bytes, then 4 bytes padding
    vec3 u_lightColor;       // offset 16
    vec3 u_ambientColor;     // offset 32
};

struct PointLight {
    vec3  position;  // offset 0
    float range;     // offset 12: a float may fill the hole after a vec3
    vec3  color;     // offset 16, then 4 bytes padding
};                   // one element takes 32 bytes
layout(std140, binding = 2) uniform PointLights {
    PointLight u_pointLights[16];  // offset 0
    int        u_pointLightCount;  // offset 16 * 32 = 512
};

layout(binding = 0) uniform sampler2D u_texture;

uniform vec3 u_tint;
uniform float u_shininess;
uniform float u_specularStrength;

out vec4 fragColor;

vec3 Shade(vec3 toLight, vec3 lightColor, vec3 normal, vec3 toCamera, vec3 surfaceColor) {
    float diffuse = max(dot(normal, toLight), 0.0);

    vec3 halfway = normalize(toLight + toCamera);
    float specular = pow(max(dot(normal, halfway), 0.0), u_shininess) * u_specularStrength;

    return surfaceColor * (diffuse * lightColor) + specular * lightColor;
}

void main() {
    vec3 worldNormal = normalize(normal);
    vec3 toLight = -u_lightDirection;
    vec3 toCamera = normalize(u_cameraPosition - worldPosition);
    vec3 surfaceColor = texture(u_texture, uv).rgb * u_tint;
    vec3 sunShade = Shade(toLight, u_lightColor, worldNormal, toCamera, surfaceColor);

    vec3 color = surfaceColor * u_ambientColor + sunShade;

    for (int i = 0; i < u_pointLightCount; ++i) {
        PointLight light = u_pointLights[i];

        vec3 toPointLight = light.position - worldPosition;
        float d = length(toPointLight);
        toPointLight /= d;

        float window = clamp(1.0 - pow(d / light.range, 4.0), 0.0, 1.0);
        float attenuation = window * window / (d * d + 1.0);

        color += Shade(toPointLight, light.color, worldNormal, toCamera, surfaceColor) * attenuation;
    }

    fragColor = vec4(color, 1.0);
}

