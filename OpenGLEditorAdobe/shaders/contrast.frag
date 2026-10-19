#version 460 core
// GPU Contrast - scales distance from 0.5
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Contrast = 1.0; // 0.0 to 3.0

void main(){
    vec4 color = texture(u_Texture, v_TexCoord);
    color.rgb = (color.rgb - 0.5) * u_Contrast + 0.5;
    outColor = color;
}
