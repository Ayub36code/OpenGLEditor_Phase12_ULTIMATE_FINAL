#version 460 core
// Phase 6 - unified color adjustment shader
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform float u_Brightness = 0.0; // -1..1
uniform float u_Contrast = 1.0;   // 0..3
uniform float u_Saturation = 1.0; // 0..3
uniform float u_Gamma = 1.0;

vec3 adjustSaturation(vec3 color, float sat){
    float gray = dot(color, vec3(0.299,0.587,0.114));
    return mix(vec3(gray), color, sat);
}

void main(){
    vec4 c = texture(u_Texture, vTexCoord);
    c.rgb += u_Brightness;
    c.rgb = (c.rgb - 0.5) * u_Contrast + 0.5;
    c.rgb = adjustSaturation(c.rgb, u_Saturation);
    c.rgb = pow(c.rgb, vec3(1.0/u_Gamma));
    FragColor = c;
}
