
#include "graphics/Texture.h"
#include "core/Log.h"
#include <glad/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Editor {

static GLenum ToGLInternalFormat(TextureFormat f) {
    switch(f){
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGBA16F: return GL_RGBA16F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        default: return GL_RGBA8;
    }
}
static GLenum ToGLFormat(TextureFormat f){
    switch(f){ case TextureFormat::RGB8: return GL_RGB; case TextureFormat::RGBA8: case TextureFormat::RGBA16F: case TextureFormat::RGBA32F: return GL_RGBA; default: return GL_RGBA; }
}
static GLenum ToGLType(TextureFormat f){
    switch(f){ case TextureFormat::RGBA16F: return GL_HALF_FLOAT; case TextureFormat::RGBA32F: return GL_FLOAT; default: return GL_UNSIGNED_BYTE; }
}

Texture2D::Texture2D(const TextureSpecification& spec): m_Spec(spec) {
    Allocate();
    m_Loaded=true;
}

Texture2D::Texture2D(const std::string& path, const TextureSpecification& spec): m_Path(path) {
    int w,h,channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if(!data) { LOG_ERROR("Failed to load texture: " + path + " reason: " + stbi_failure_reason()); m_Spec={}; return; }

    m_Spec = spec;
    m_Spec.Width = w; m_Spec.Height = h;
    if(channels==3) m_Spec.Format = TextureFormat::RGB8;
    else if(channels==4) m_Spec.Format = TextureFormat::RGBA8;

    Allocate();
    GLenum format = ToGLFormat(m_Spec.Format);
    GLenum type = ToGLType(m_Spec.Format);
    glTextureSubImage2D(m_RendererID, 0, 0,0, w,h, format, type, data);

    if(m_Spec.GenerateMips) glGenerateTextureMipmap(m_RendererID);

    stbi_image_free(data);
    m_Loaded=true;
    LOG_INFO("Texture loaded: " + path + " " + std::to_string(w)+"x"+std::to_string(h));
}

Texture2D::~Texture2D(){ if(m_RendererID) glDeleteTextures(1,&m_RendererID); }

void Texture2D::Allocate() {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    GLenum internalFormat = ToGLInternalFormat(m_Spec.Format);
    glTextureStorage2D(m_RendererID, m_Spec.GenerateMips ? 4 : 1, internalFormat, m_Spec.Width, m_Spec.Height);
    SetParameters();
}

void Texture2D::SetParameters() {
    GLenum filter = (m_Spec.Filter==TextureFilter::Linear) ? GL_LINEAR : GL_NEAREST;
    GLenum minFilter = m_Spec.GenerateMips ? (filter==GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : filter;
    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, minFilter);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);
    GLenum wrap = GL_CLAMP_TO_EDGE;
    if(m_Spec.Wrap==TextureWrap::Repeat) wrap=GL_REPEAT;
    else if(m_Spec.Wrap==TextureWrap::Mirror) wrap=GL_MIRRORED_REPEAT;
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);
}

void Texture2D::Bind(uint32_t slot) const { glBindTextureUnit(slot, m_RendererID); }
void Texture2D::Unbind() const { glBindTextureUnit(0,0); }

void Texture2D::SetData(void* data, uint32_t size) {
    // size validation omitted for brevity - assumes correct
    GLenum format = ToGLFormat(m_Spec.Format);
    GLenum type = ToGLType(m_Spec.Format);
    glTextureSubImage2D(m_RendererID, 0, 0,0, m_Spec.Width, m_Spec.Height, format, type, data);
}

void Texture2D::Resize(uint32_t w, uint32_t h) {
    if(m_RendererID) glDeleteTextures(1,&m_RendererID);
    m_Spec.Width=w; m_Spec.Height=h;
    Allocate();
}

std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecification& spec){ return std::make_shared<Texture2D>(spec); }
std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path){ return std::make_shared<Texture2D>(path); }

}
