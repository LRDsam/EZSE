#version 460 core

in  vec3 normal;
out vec4 fragColor;

void main() {
    fragColor = vec4(normal * 0.5 + 0.5, 1.0);
}
