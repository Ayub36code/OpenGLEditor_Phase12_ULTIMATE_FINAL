#version 460 core
layout(location=0) out vec4 color;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Tiling = 1.0;
uniform vec4 u_Tint = vec4(1.0);
void main(){
    vec4 tex = texture(u_Texture, v_TexCoord * u_Tiling);
    color = tex * u_Tint;
}
