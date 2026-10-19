#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec2 u_TexelSize;
uniform float u_Strength=1.0;

void main(){
    float kernel[9] = float[](-1,-1,-1,-1,8,-1,-1,-1,-1);
    vec3 sum=vec3(0);
    int idx=0;
    for(int y=-1;y<=1;y++) for(int x=-1;x<=1;x++){
        vec2 offset=vec2(x,y)*u_TexelSize;
        vec3 c=texture(u_Texture, v_TexCoord+offset).rgb;
        sum+=c*kernel[idx++];
    }
    sum*=u_Strength;
    outColor=vec4(sum, 1.0);
}
