#version 460 core
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform vec4 u_Tint = vec4(1.0);
uniform float u_Opacity = 1.0;
void main(){
    vec4 tex = texture(u_Texture, vTexCoord);
    FragColor = tex * u_Tint;
    FragColor.a *= u_Opacity;
}
