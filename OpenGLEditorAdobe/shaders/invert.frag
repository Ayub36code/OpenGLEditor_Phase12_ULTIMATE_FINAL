#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main(){
    vec4 color = texture(u_Texture, v_TexCoord);
    outColor = vec4(1.0 - color.rgb, color.a);
}
