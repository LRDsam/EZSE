#version 460 core

in  vec3 color;
in  vec3 normal;

layout(std140, binding = 1) uniform Light {
    vec3 u_lightDirection;   // offset 0, 12 bytes, then 4 bytes padding
    vec3 u_lightColor;       // offset 16
};

out vec4 fragColor;

void main() {
    vec3 worldNormal = normalize(normal);
    float diffuse = max(dot(worldNormal, -u_lightDirection), 0.0);
    fragColor = vec4(diffuse * u_lightColor * color, 1.0);
}
