#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
    vec3 u_cameraPosition;
};

layout(std140, binding = 1) uniform Light {
    vec3 u_lightDirection;      // offset 0, 12 bytes, then 4 bytes padding
    vec3 u_lightColor;          // offset 16, 12 bytes, then 4 bytes padding
    vec3 u_ambientColor;        // offset 32, 12 bytes, then 4 bytes padding
    mat4 u_lightViewProjection; // offset 48, 64 bytes
};

uniform mat4 u_model;

out vec3 normal;
out vec3 worldPosition;
out vec4 lightSpacePosition;
out vec2 uv;

void main() {
    vec4 relative = u_model * vec4(a_position, 1.0);
    gl_Position = u_viewProjection * relative;
    normal = transpose(inverse(mat3(u_model))) * a_normal;
    worldPosition = vec3(relative);
    uv = a_uv;
    lightSpacePosition = u_lightViewProjection * relative;
}
