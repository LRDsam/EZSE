#version 460 core

in  vec2 uv;

layout(binding = 0) uniform sampler2D u_texture;

uniform vec3 u_tint;

out vec4 fragColor;

void main() {
    fragColor = vec4(texture(u_texture, uv).rgb * u_tint, 1.0);
}
