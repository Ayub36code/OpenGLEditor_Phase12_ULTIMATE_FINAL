#version 460 core
in vec3 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform vec4 u_Color = vec4(1.0);

void main() {
    FragColor = vec4(vColor, 1.0) * u_Color;
}
