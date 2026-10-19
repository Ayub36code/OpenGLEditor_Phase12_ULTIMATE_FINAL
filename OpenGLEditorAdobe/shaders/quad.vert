#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
out vec2 v_TexCoord;
void main(){
    v_TexCoord = aTexCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(aPos,1.0);
}
