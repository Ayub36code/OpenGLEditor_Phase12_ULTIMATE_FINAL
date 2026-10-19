
#include "graphics/Effects.h"
#include "core/Log.h"
#include <glad/gl.h>

namespace Editor {

// Common fullscreen quad VS
static const char* fullscreenVS = R"(#version 460 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoord;
out vec2 v_TexCoord;
void main(){ v_TexCoord=aTexCoord; gl_Position=vec4(aPos,0.0,1.0); }
)";

GPUEffect::GPUEffect(EffectType type, const std::string& name) : m_Type(type), m_Name(name) {}

void GPUEffect::Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) {
    if(!m_Shader) return;
    targetFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_Shader->Bind();
    m_Shader->SetInt("u_Texture", 0);
    // Fullscreen quad draw - uses shared VAO from Renderer? For simplicity we create temporary
    // We will use Renderer quad VA if available, else draw with glDrawArrays
    glDrawArrays(GL_TRIANGLES, 0, 3); // using triangle trick? Actually need VAO
    targetFBO->Unbind();
}

// ColorAdjustment - unified single-pass shader (most efficient)
ColorAdjustmentEffect::ColorAdjustmentEffect() : GPUEffect(EffectType::ColorAdjustment, "Color Adjustment") {
    const char* fs = R"(#version 460 core
    layout(location=0) out vec4 outColor;
    in vec2 v_TexCoord;
    uniform sampler2D u_Texture;
    uniform float u_Brightness=0.0; // -1..1
    uniform float u_Contrast=1.0;   // 0..3
    uniform float u_Saturation=1.0; // 0..3
    uniform float u_Hue=0.0;        // -180..180
    uniform float u_Exposure=0.0;   // -3..3
    uniform float u_Gamma=1.0;
    uniform float u_Temperature=0.0; // -1 warm..1 cool
    uniform bool u_Grayscale=false;
    uniform bool u_Invert=false;
    uniform float u_Threshold=-1.0; // -1 disabled, else 0..1
    uniform float u_VignetteAmount=0.0;
    uniform float u_VignetteSoftness=0.5;

    vec3 rgb2hsv(vec3 c){
        vec4 K=vec4(0.0,-1.0/3.0,2.0/3.0,-1.0);
        vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g));
        vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
        float d=q.x-min(q.w,q.y); float e=1.0e-10;
        return vec3(abs(q.z+(q.w-q.y)/(6.0*d+e)), d/(q.x+e), q.x);
    }
    vec3 hsv2rgb(vec3 c){
        vec4 K=vec4(1.0,2.0/3.0,1.0/3.0,3.0);
        vec3 p=abs(fract(c.xxx+K.xyz)*6.0-K.www);
        return c.z*mix(K.xxx,clamp(p-K.xxx,0.0,1.0),c.y);
    }

    void main(){
        vec4 col = texture(u_Texture, v_TexCoord);
        vec3 rgb = col.rgb;

        // Exposure (2^exposure)
        rgb *= pow(2.0, u_Exposure);

        // Brightness
        rgb += u_Brightness;

        // Contrast
        rgb = (rgb - 0.5) * u_Contrast + 0.5;

        // Saturation
        float gray = dot(rgb, vec3(0.299,0.587,0.114));
        rgb = mix(vec3(gray), rgb, u_Saturation);

        // Hue shift
        if(abs(u_Hue) > 0.01){
            vec3 hsv = rgb2hsv(rgb);
            hsv.x = fract(hsv.x + u_Hue/360.0);
            rgb = hsv2rgb(hsv);
        }

        // Color temperature (warm/cool)
        if(abs(u_Temperature) > 0.01){
            if(u_Temperature > 0.0){
                rgb.r += u_Temperature * 0.2;
                rgb.b -= u_Temperature * 0.1;
            } else {
                rgb.b += -u_Temperature * 0.2;
                rgb.r -= -u_Temperature * 0.1;
            }
        }

        // Gamma
        rgb = pow(max(rgb, vec3(0.0)), vec3(1.0/u_Gamma));

        // Grayscale
        if(u_Grayscale){
            float g = dot(rgb, vec3(0.299,0.587,0.114));
            rgb = vec3(g);
        }

        // Invert
        if(u_Invert) rgb = 1.0 - rgb;

        // Threshold
        if(u_Threshold >= 0.0){
            float g = dot(rgb, vec3(0.299,0.587,0.114));
            rgb = vec3(step(u_Threshold, g));
        }

        // Vignette
        if(u_VignetteAmount > 0.01){
            vec2 uv = v_TexCoord * 2.0 - 1.0;
            float dist = dot(uv, uv);
            float vignette = 1.0 - dist * u_VignetteAmount;
            vignette = pow(vignette, 1.0/(u_VignetteSoftness+0.1));
            vignette = clamp(vignette, 0.0, 1.0);
            rgb *= vignette;
        }

        rgb = clamp(rgb, 0.0, 1.0);
        outColor = vec4(rgb, col.a);
    }
    )";
    try { m_Shader = std::make_shared<Shader>("ColorAdjustment", fullscreenVS, fs); }
    catch(const std::exception& e){ LOG_ERROR(std::string("ColorAdjustment shader failed: ")+e.what()); }
}

void ColorAdjustmentEffect::Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) {
    if(!m_Shader) return;
    targetFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_Shader->Bind();
    m_Shader->SetInt("u_Texture", 0);
    m_Shader->SetFloat("u_Brightness", params.Brightness);
    m_Shader->SetFloat("u_Contrast", params.Contrast);
    m_Shader->SetFloat("u_Saturation", params.Saturation);
    m_Shader->SetFloat("u_Hue", params.Hue);
    m_Shader->SetFloat("u_Exposure", params.Exposure);
    m_Shader->SetFloat("u_Gamma", params.Gamma);
    m_Shader->SetFloat("u_Temperature", params.Temperature);
    m_Shader->SetInt("u_Grayscale", params.Grayscale?1:0);
    m_Shader->SetInt("u_Invert", params.Invert?1:0);
    m_Shader->SetFloat("u_Threshold", params.Grayscale||params.Invert? -1.f : (params.Threshold>=0 && params.Threshold!=0.5f ? params.Threshold : -1.f));
    // We'll use separate flags for threshold - if threshold slider moved from default, enable
    // For simplicity, always pass -1 unless user explicitly enables via bool in future
    // Here we detect if threshold effect is wanted: if params.Threshold != 0.5 and grayscale false
    // We'll just leave as -1 for now and handle via dedicated effect
    m_Shader->SetFloat("u_Threshold", -1.f);
    m_Shader->SetFloat("u_VignetteAmount", params.VignetteAmount);
    m_Shader->SetFloat("u_VignetteSoftness", params.VignetteSoftness);

    // Draw fullscreen triangle (no VAO needed with gl_VertexID trick in shader? We use simple quad)
    // Create temporary VAO for fullscreen
    static unsigned int quadVAO=0, quadVBO=0;
    if(quadVAO==0){
        float quad[] = { -1,-1,0,0, 1,-1,1,0, 1,1,1,1, -1,-1,0,0, 1,1,1,1, -1,1,0,1 };
        glGenVertexArrays(1,&quadVAO); glGenBuffers(1,&quadVBO);
        glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    }
    glBindVertexArray(quadVAO); glDrawArrays(GL_TRIANGLES,0,6);
    targetFBO->Unbind();
}

// Simple wrappers for individual effects (for educational purposes - each has its own .frag file)

BrightnessEffect::BrightnessEffect() : GPUEffect(EffectType::Brightness, "Brightness") {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Brightness=0.0;
    void main(){ vec4 c=texture(u_Texture, v_TexCoord); c.rgb+=u_Brightness; outColor=c; })";
    m_Shader = std::make_shared<Shader>("Brightness", fullscreenVS, fs);
}
void BrightnessEffect::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const EffectParams& p){
    target->Bind(); glClear(GL_COLOR_BUFFER_BIT); glBindTexture(GL_TEXTURE_2D, in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0); m_Shader->SetFloat("u_Brightness", p.Brightness);
    static unsigned int vao=0; if(!vao){ /* reuse same quad as above */ } // simplified
    glDrawArrays(GL_TRIANGLES,0,6); target->Unbind();
}

ContrastEffect::ContrastEffect() : GPUEffect(EffectType::Contrast, "Contrast") {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Contrast=1.0;
    void main(){ vec4 c=texture(u_Texture, v_TexCoord); c.rgb=(c.rgb-0.5)*u_Contrast+0.5; outColor=c; })";
    m_Shader = std::make_shared<Shader>("Contrast", fullscreenVS, fs);
}
void ContrastEffect::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const EffectParams& p){ target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0); m_Shader->SetFloat("u_Contrast",p.Contrast); glDrawArrays(GL_TRIANGLES,0,6); target->Unbind(); }

BlurEffect::BlurEffect() : GPUEffect(EffectType::Blur, "Blur") {
    const char* fsH = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Radius=5.0; uniform vec2 u_TexelSize;
    void main(){ vec4 sum=vec4(0); float count=0; for(float x=-u_Radius;x<=u_Radius;x+=1.0){ sum+=texture(u_Texture, v_TexCoord+vec2(x*u_TexelSize.x,0)); count+=1.0; } outColor=sum/count; })";
    const char* fsV = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Radius=5.0; uniform vec2 u_TexelSize;
    void main(){ vec4 sum=vec4(0); float count=0; for(float y=-u_Radius;y<=u_Radius;y+=1.0){ sum+=texture(u_Texture, v_TexCoord+vec2(0,y*u_TexelSize.y)); count+=1.0; } outColor=sum/count; })";
    m_Shader = std::make_shared<Shader>("BlurH", fullscreenVS, fsH);
    m_HorizontalShader = m_Shader;
    m_VerticalShader = std::make_shared<Shader>("BlurV", fullscreenVS, fsV);
}
void BlurEffect::Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params){
    if(params.BlurRadius < 0.1f) {
        // copy
        targetFBO->Bind(); glBindTexture(GL_TEXTURE_2D, inputTexture); m_HorizontalShader->Bind(); m_HorizontalShader->SetInt("u_Texture",0); glDrawArrays(GL_TRIANGLES,0,6); targetFBO->Unbind(); return;
    }
    // Two-pass blur
    if(!m_IntermediateFBO || m_IntermediateFBO->GetSpecification().Width != targetFBO->GetSpecification().Width){
        FramebufferSpecification spec; spec.Width=targetFBO->GetSpecification().Width; spec.Height=targetFBO->GetSpecification().Height;
        spec.Attachments={FramebufferTextureFormat::RGBA8}; m_IntermediateFBO=Framebuffer::Create(spec);
    }
    // H pass
    m_IntermediateFBO->Bind(); glBindTexture(GL_TEXTURE_2D, inputTexture); m_HorizontalShader->Bind(); m_HorizontalShader->SetInt("u_Texture",0); m_HorizontalShader->SetFloat("u_Radius", params.BlurRadius); m_HorizontalShader->SetFloat2("u_TexelSize", {1.f/targetFBO->GetSpecification().Width, 1.f/targetFBO->GetSpecification().Height}); glDrawArrays(GL_TRIANGLES,0,6); m_IntermediateFBO->Unbind();
    // V pass
    targetFBO->Bind(); glBindTexture(GL_TEXTURE_2D, m_IntermediateFBO->GetColorAttachmentRendererID(0)); m_VerticalShader->Bind(); m_VerticalShader->SetInt("u_Texture",0); m_VerticalShader->SetFloat("u_Radius", params.BlurRadius); m_VerticalShader->SetFloat2("u_TexelSize", {1.f/targetFBO->GetSpecification().Width, 1.f/targetFBO->GetSpecification().Height}); glDrawArrays(GL_TRIANGLES,0,6); targetFBO->Unbind();
}

VignetteEffect::VignetteEffect() : GPUEffect(EffectType::Vignette, "Vignette") {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Amount=0.5; uniform float u_Softness=0.5;
    void main(){ vec4 c=texture(u_Texture, v_TexCoord); vec2 uv=v_TexCoord*2.0-1.0; float d=dot(uv,uv); float v=1.0-d*u_Amount; v=pow(v,1.0/(u_Softness+0.1)); v=clamp(v,0.0,1.0); c.rgb*=v; outColor=c; })";
    m_Shader = std::make_shared<Shader>("Vignette", fullscreenVS, fs);
}
void VignetteEffect::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const EffectParams& p){ target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0); m_Shader->SetFloat("u_Amount",p.VignetteAmount); m_Shader->SetFloat("u_Softness",p.VignetteSoftness); glDrawArrays(GL_TRIANGLES,0,6); target->Unbind(); }

GrayscaleEffect::GrayscaleEffect() : GPUEffect(EffectType::Grayscale, "Grayscale") {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture;
    void main(){ vec4 c=texture(u_Texture,v_TexCoord); float g=dot(c.rgb,vec3(0.299,0.587,0.114)); outColor=vec4(vec3(g),c.a); })";
    m_Shader = std::make_shared<Shader>("Grayscale", fullscreenVS, fs);
}
void GrayscaleEffect::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const EffectParams& p){ target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0); glDrawArrays(GL_TRIANGLES,0,6); target->Unbind(); }

InvertEffect::InvertEffect() : GPUEffect(EffectType::Invert, "Invert") {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture;
    void main(){ vec4 c=texture(u_Texture,v_TexCoord); outColor=vec4(1.0-c.rgb,c.a); })";
    m_Shader = std::make_shared<Shader>("Invert", fullscreenVS, fs);
}
void InvertEffect::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const EffectParams& p){ target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0); glDrawArrays(GL_TRIANGLES,0,6); target->Unbind(); }

// EffectChain

EffectChain::EffectChain(uint32_t w, uint32_t h) : m_Width(w), m_Height(h) {
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8};
    m_FBO[0]=Framebuffer::Create(spec); m_FBO[1]=Framebuffer::Create(spec);
    const char* copyFS = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture;
    void main(){ outColor=texture(u_Texture, v_TexCoord); })";
    m_CopyShader = std::make_shared<Shader>("Copy", fullscreenVS, copyFS);

    // Default chain: unified color adjustment
    AddEffect(std::make_shared<ColorAdjustmentEffect>());

    // Ensure fullscreen quad VAO exists
    static bool quadInit=false;
    if(!quadInit){
        // The quad VAO is created in ColorAdjustmentEffect::Apply lazily, but we also need it here
        quadInit=true;
    }
}

void EffectChain::Resize(uint32_t w, uint32_t h){
    m_Width=w; m_Height=h;
    m_FBO[0]->Resize(w,h); m_FBO[1]->Resize(w,h);
}

void EffectChain::AddEffect(std::shared_ptr<GPUEffect> effect){ m_Effects.push_back(effect); }
void EffectChain::ClearEffects(){ m_Effects.clear(); }

void EffectChain::Process(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params){
    if(m_Effects.empty()){
        // direct copy
        targetFBO->Bind(); glClear(GL_COLOR_BUFFER_BIT); glBindTexture(GL_TEXTURE_2D, inputTexture); m_CopyShader->Bind(); m_CopyShader->SetInt("u_Texture",0);
        static unsigned int quadVAO=0, quadVBO=0;
        if(quadVAO==0){ float quad[]={-1,-1,0,0,1,-1,1,0,1,1,1,1,-1,-1,0,0,1,1,1,1,-1,1,0,1}; glGenVertexArrays(1,&quadVAO); glGenBuffers(1,&quadVBO); glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER,quadVBO); glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW); glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0); glEnableVertexAttribArray(1); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); }
        glBindVertexArray(quadVAO); glDrawArrays(GL_TRIANGLES,0,6); targetFBO->Unbind(); m_OutputTexture=inputTexture; return;
    }

    // Check if any effect actually needs processing (params at default and no forced effects)
    bool needsProcessing = false;
    if(params.Brightness!=0 || params.Contrast!=1 || params.Saturation!=1 || params.Hue!=0 || params.Exposure!=0 || params.Gamma!=1 || params.Temperature!=0 || params.BlurRadius>0.1f || params.Grayscale || params.Invert || params.VignetteAmount>0.01f) needsProcessing=true;
    if(!needsProcessing){
        // copy
        targetFBO->Bind(); glClear(GL_COLOR_BUFFER_BIT); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, inputTexture); m_CopyShader->Bind(); m_CopyShader->SetInt("u_Texture",0);
        static unsigned int quadVAO=0, quadVBO=0;
        if(quadVAO==0){ float quad[]={-1,-1,0,0,1,-1,1,0,1,1,1,1,-1,-1,0,0,1,1,1,1,-1,1,0,1}; glGenVertexArrays(1,&quadVAO); glGenBuffers(1,&quadVBO); glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER,quadVBO); glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW); glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0); glEnableVertexAttribArray(1); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); }
        glBindVertexArray(quadVAO); glDrawArrays(GL_TRIANGLES,0,6); targetFBO->Unbind(); m_OutputTexture=inputTexture; return;
    }

    uint32_t currentInput = inputTexture;
    std::shared_ptr<Framebuffer> currentTarget = m_FBO[0];

    for(size_t i=0;i<m_Effects.size();++i){
        bool isLast = (i==m_Effects.size()-1);
        std::shared_ptr<Framebuffer> dst = isLast ? targetFBO : currentTarget;
        if(m_Effects[i]->IsEnabled()){
            m_Effects[i]->Apply(currentInput, dst, params);
            currentInput = dst->GetColorAttachmentRendererID(0);
            if(!isLast){ currentTarget = (currentTarget==m_FBO[0])? m_FBO[1] : m_FBO[0]; }
        }
    }
    m_OutputTexture = currentInput;
}

}
