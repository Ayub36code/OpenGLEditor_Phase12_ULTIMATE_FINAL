#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main(){
    vec4 color = texture(u_Texture, v_TexCoord);
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    outColor = vec4(vec3(gray), color.a);
}
