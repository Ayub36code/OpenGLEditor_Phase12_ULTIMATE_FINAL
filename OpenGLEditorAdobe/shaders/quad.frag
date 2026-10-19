#version 460 core
layout(location=0) out vec4 color;
in vec2 v_TexCoord;
uniform vec4 u_Color = vec4(1.0);
void main(){ color = u_Color; }
