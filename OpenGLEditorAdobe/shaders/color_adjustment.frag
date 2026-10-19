#version 460 core
// Unified color adjustment - single pass, most efficient
// Combines brightness, contrast, saturation, hue, exposure, gamma, temp, grayscale, invert, vignette
layout(location=0) out vec4 outColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Brightness=0.0;
uniform float u_Contrast=1.0;
uniform float u_Saturation=1.0;
uniform float u_Hue=0.0;
uniform float u_Exposure=0.0;
uniform float u_Gamma=1.0;
uniform float u_Temperature=0.0;
uniform bool u_Grayscale=false;
uniform bool u_Invert=false;
uniform float u_VignetteAmount=0.0;
uniform float u_VignetteSoftness=0.5;

vec3 rgb2hsv(vec3 c){ vec4 K=vec4(0.0,-1.0/3.0,2.0/3.0,-1.0); vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g)); vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r)); float d=q.x-min(q.w,q.y); float e=1.0e-10; return vec3(abs(q.z+(q.w-q.y)/(6.0*d+e)), d/(q.x+e), q.x); }
vec3 hsv2rgb(vec3 c){ vec4 K=vec4(1.0,2.0/3.0,1.0/3.0,3.0); vec3 p=abs(fract(c.xxx+K.xyz)*6.0-K.www); return c.z*mix(K.xxx,clamp(p-K.xxx,0.0,1.0),c.y); }

void main(){
    vec4 col=texture(u_Texture, v_TexCoord); vec3 rgb=col.rgb;
    rgb *= pow(2.0, u_Exposure);
    rgb += u_Brightness;
    rgb = (rgb-0.5)*u_Contrast+0.5;
    float gray=dot(rgb, vec3(0.299,0.587,0.114));
    rgb=mix(vec3(gray), rgb, u_Saturation);
    if(abs(u_Hue)>0.01){ vec3 hsv=rgb2hsv(rgb); hsv.x=fract(hsv.x+u_Hue/360.0); rgb=hsv2rgb(hsv); }
    if(abs(u_Temperature)>0.01){ if(u_Temperature>0.0){ rgb.r+=u_Temperature*0.2; rgb.b-=u_Temperature*0.1; } else { rgb.b+=-u_Temperature*0.2; rgb.r-=-u_Temperature*0.1; } }
    rgb=pow(max(rgb,vec3(0.0)), vec3(1.0/u_Gamma));
    if(u_Grayscale){ float g=dot(rgb, vec3(0.299,0.587,0.114)); rgb=vec3(g); }
    if(u_Invert) rgb=1.0-rgb;
    if(u_VignetteAmount>0.01){ vec2 uv=v_TexCoord*2.0-1.0; float d=dot(uv,uv); float v=1.0-d*u_VignetteAmount; v=pow(v,1.0/(u_VignetteSoftness+0.1)); rgb*=clamp(v,0.0,1.0); }
    outColor=vec4(clamp(rgb,0.0,1.0), col.a);
}
