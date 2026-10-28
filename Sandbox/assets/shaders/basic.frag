#version 460 core

in  vec2 uv;

layout(binding = 0) uniform sampler2D u_texture;

out vec4 fragColor;

void main() {
    fragColor = vec4(texture(u_texture, uv).rgb, 1.0);
}
