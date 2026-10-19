#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Threshold=0.5;

void main(){
    vec4 c=texture(u_Texture, v_TexCoord);
    float gray=dot(c.rgb, vec3(0.299,0.587,0.114));
    float t=step(u_Threshold, gray);
    outColor=vec4(vec3(t), c.a);
}
