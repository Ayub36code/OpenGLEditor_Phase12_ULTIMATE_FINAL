#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Saturation = 1.0;

void main(){
    vec4 color = texture(u_Texture, v_TexCoord);
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    color.rgb = mix(vec3(gray), color.rgb, u_Saturation);
    outColor = color;
}
