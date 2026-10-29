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

layout(binding = 0) uniform sampler2D u_texture;

uniform vec3 u_tint;
uniform float u_shininess;
uniform float u_specularStrength;

out vec4 fragColor;

void main() {
    vec3 worldNormal = normalize(normal);
    float diffuse = max(dot(worldNormal, -u_lightDirection), 0.0);

    vec3 toLight = -u_lightDirection;
    vec3 toCamera = normalize(u_cameraPosition - worldPosition);
    vec3 halfway = normalize(toLight + toCamera);

    float specular = pow(max(dot(worldNormal, halfway), 0.0), u_shininess) *
        u_specularStrength;

    vec3 surfaceColor = texture(u_texture, uv).rgb * u_tint;
    fragColor = vec4(surfaceColor * (u_ambientColor + diffuse * u_lightColor) +
        specular * u_lightColor, 1.0);
}
