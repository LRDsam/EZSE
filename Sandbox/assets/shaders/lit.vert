#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
    vec3 u_cameraPosition;
};
uniform mat4 u_model;

out vec3 color;
out vec3 normal;
out vec3 worldPosition;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 1.0);
    color = a_color;
    normal = transpose(inverse(mat3(u_model))) * a_normal;
    worldPosition = vec3(u_model * vec4(a_position, 1.0));
}
