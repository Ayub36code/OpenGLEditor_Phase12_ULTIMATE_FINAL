#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
layout(location=2) in vec2 aTexCoord;

out vec3 vColor;
out vec2 vTexCoord;

uniform mat4 u_MVP = mat4(1.0);

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = u_MVP * vec4(aPos, 1.0);
}
