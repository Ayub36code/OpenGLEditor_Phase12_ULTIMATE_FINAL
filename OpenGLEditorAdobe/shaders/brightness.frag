#version 460 core
// GPU Brightness - adds uniform offset to RGB
// CPU->GPU: texture stays on GPU, only u_Brightness uniform updated
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Brightness = 0.0; // -1.0 to 1.0

void main(){
    vec4 color = texture(u_Texture, v_TexCoord);
    color.rgb += u_Brightness; // simple add, GPU parallel per pixel
    outColor = color;
}
