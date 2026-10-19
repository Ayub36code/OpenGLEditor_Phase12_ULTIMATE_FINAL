#version 460 core
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec2 u_TexelSize;
uniform float u_Amount = 0.5; // 0..2

void main(){
    vec4 center = texture(u_Texture, v_TexCoord);
    vec4 up = texture(u_Texture, v_TexCoord + vec2(0, u_TexelSize.y));
    vec4 down = texture(u_Texture, v_TexCoord + vec2(0, -u_TexelSize.y));
    vec4 left = texture(u_Texture, v_TexCoord + vec2(-u_TexelSize.x, 0));
    vec4 right = texture(u_Texture, v_TexCoord + vec2(u_TexelSize.x, 0));

    // Unsharp mask
    vec4 blur = (up + down + left + right) * 0.25;
    vec4 sharpened = center + (center - blur) * u_Amount;
    outColor = sharpened;
}
