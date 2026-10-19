#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec2 u_TexelSize;

void main(){
    vec3 c1=texture(u_Texture, v_TexCoord + u_TexelSize).rgb;
    vec3 c2=texture(u_Texture, v_TexCoord - u_TexelSize).rgb;
    vec3 diff=c1-c2+0.5;
    float gray=dot(diff, vec3(0.299,0.587,0.114));
    outColor=vec4(vec3(gray),1.0);
}
