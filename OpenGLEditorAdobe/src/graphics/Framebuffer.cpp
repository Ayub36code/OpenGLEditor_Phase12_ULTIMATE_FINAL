
#include "graphics/Framebuffer.h"
#include "core/Log.h"
#include <glad/gl.h>

namespace Editor {

static GLenum FBTextureFormatToGL(FramebufferTextureFormat format){
    switch(format){
        case FramebufferTextureFormat::RGBA8: return GL_RGBA8;
        case FramebufferTextureFormat::RGBA16F: return GL_RGBA16F;
        case FramebufferTextureFormat::RED_INTEGER: return GL_R32I;
        case FramebufferTextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        default: return 0;
    }
}

Framebuffer::Framebuffer(const FramebufferSpecification& spec): m_Spec(spec) {
    for(auto& att: m_Spec.Attachments.Attachments){
        if(att.TextureFormat==FramebufferTextureFormat::Depth24Stencil8 || att.TextureFormat==FramebufferTextureFormat::Depth32F)
            m_DepthSpec=att;
        else m_ColorAttachmentSpecs.push_back(att);
    }
    Invalidate();
}

Framebuffer::~Framebuffer(){ if(m_RendererID) glDeleteFramebuffers(1,&m_RendererID);
    if(!m_ColorAttachments.empty()) glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
    if(m_DepthAttachment) glDeleteTextures(1,&m_DepthAttachment);
}

void Framebuffer::Invalidate(){
    if(m_RendererID){ glDeleteFramebuffers(1,&m_RendererID);
        if(!m_ColorAttachments.empty()) glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        if(m_DepthAttachment) glDeleteTextures(1,&m_DepthAttachment);
        m_ColorAttachments.clear(); m_DepthAttachment=0;
    }

    glCreateFramebuffers(1,&m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    if(!m_ColorAttachmentSpecs.empty()){
        m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
        glCreateTextures(GL_TEXTURE_2D, m_ColorAttachments.size(), m_ColorAttachments.data());

        for(size_t i=0;i<m_ColorAttachments.size();++i){
            auto& spec = m_ColorAttachmentSpecs[i];
            glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[i]);
            GLenum internalFormat = FBTextureFormatToGL(spec.TextureFormat);
            if(spec.TextureFormat==FramebufferTextureFormat::RED_INTEGER){
                glTexImage2D(GL_TEXTURE_2D,0,internalFormat,m_Spec.Width,m_Spec.Height,0,GL_RED_INTEGER,GL_INT,nullptr);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            } else {
                glTexImage2D(GL_TEXTURE_2D,0,internalFormat,m_Spec.Width,m_Spec.Height,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)i, GL_TEXTURE_2D, m_ColorAttachments[i], 0);
        }
    }

    if(m_DepthSpec.TextureFormat!=FramebufferTextureFormat::None){
        glCreateTextures(GL_TEXTURE_2D,1,&m_DepthAttachment);
        glBindTexture(GL_TEXTURE_2D,m_DepthAttachment);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH24_STENCIL8,m_Spec.Width,m_Spec.Height,0,GL_DEPTH_STENCIL,GL_UNSIGNED_INT_24_8,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
    }

    if(m_ColorAttachments.size()>1){
        GLenum buffers[8]; for(size_t i=0;i<m_ColorAttachments.size();++i) buffers[i]=GL_COLOR_ATTACHMENT0 + (GLenum)i;
        glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
    } else if(m_ColorAttachments.empty()){
        glDrawBuffer(GL_NONE);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        LOG_ERROR("Framebuffer incomplete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Framebuffer::Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); glViewport(0,0,m_Spec.Width,m_Spec.Height); }
void Framebuffer::Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER,0); }

void Framebuffer::Resize(uint32_t w,uint32_t h){
    if(w==0||h==0|| w>8192 || h>8192){ LOG_WARNING("Invalid framebuffer resize"); return; }
    m_Spec.Width=w; m_Spec.Height=h; Invalidate();
}

uint32_t Framebuffer::GetColorAttachmentRendererID(uint32_t index) const {
    if(index < m_ColorAttachments.size()) return m_ColorAttachments[index];
    return 0;
}

int Framebuffer::ReadPixel(uint32_t attachmentIndex,int x,int y){
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
    int pixel; glReadPixels(x,y,1,1,GL_RED_INTEGER,GL_INT,&pixel); return pixel;
}
void Framebuffer::ClearAttachment(uint32_t attachmentIndex,int value){
    GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;
    glClearTexImage(m_ColorAttachments[attachmentIndex],0,GL_RED_INTEGER,GL_INT,&value);
}
void Framebuffer::ClearAttachment(uint32_t attachmentIndex,float value){
    GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;
    glClearTexImage(m_ColorAttachments[attachmentIndex],0,GL_RGBA,GL_FLOAT,&value);
}

std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec){ return std::make_shared<Framebuffer>(spec); }

std::shared_ptr<Framebuffer> FramebufferPool::GetOrCreate(const FramebufferSpecification& spec){
    // simple pool: find unused matching size or create new
    for(auto& fb: m_Pool){ if(fb->GetSpecification().Width==spec.Width && fb->GetSpecification().Height==spec.Height) return fb; }
    auto fb = Framebuffer::Create(spec); m_Pool.push_back(fb); return fb;
}
void FramebufferPool::ReleaseAll(){ m_Pool.clear(); }

}
