#version 460 core
// Two-pass Gaussian blur - this is single pass box blur for simplicity
// For production, use separable Gaussian with weights
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec2 u_TexelSize; // 1/width, 1/height
uniform float u_Radius = 5.0;

void main(){
    vec4 sum = vec4(0.0);
    // 9-tap box blur - GPU does this in parallel per pixel, no CPU loop over pixels
    for(float x=-u_Radius; x<=u_Radius; x+=1.0){
        for(float y=-u_Radius; y<=u_Radius; y+=1.0){
            sum += texture(u_Texture, v_TexCoord + vec2(x,y)*u_TexelSize);
        }
    }
    float count = (2.0*u_Radius+1.0)*(2.0*u_Radius+1.0);
    outColor = sum / count;
}
