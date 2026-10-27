#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;

layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
    vec3 u_cameraPosition;
};
uniform mat4 u_model;

out vec3 normal;
out vec3 worldPosition;
out vec2 uv;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 1.0);
    normal = transpose(inverse(mat3(u_model))) * a_normal;
    worldPosition = vec3(u_model * vec4(a_position, 1.0));
    uv = a_uv;
}
