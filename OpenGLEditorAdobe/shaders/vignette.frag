#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Amount=0.5;
uniform float u_Softness=0.5;

void main(){
    vec4 color=texture(u_Texture, v_TexCoord);
    vec2 uv=v_TexCoord*2.0-1.0;
    float dist=dot(uv,uv);
    float vignette=1.0-dist*u_Amount;
    vignette=pow(vignette, 1.0/(u_Softness+0.1));
    vignette=clamp(vignette,0.0,1.0);
    color.rgb*=vignette;
    outColor=color;
}
