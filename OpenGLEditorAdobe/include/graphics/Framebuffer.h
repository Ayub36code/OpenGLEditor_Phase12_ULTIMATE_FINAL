
#pragma once
#include <memory>
#include <vector>
#include <cstdint>

namespace Editor {

enum class FramebufferTextureFormat { None=0, RGBA8, R8, Depth24Stencil8, Depth32F };

struct FramebufferTextureSpec {
    FramebufferTextureFormat TextureFormat=FramebufferTextureFormat::RGBA8;
    // Allow implicit from format for compatibility
    FramebufferTextureSpec() = default;
    FramebufferTextureSpec(FramebufferTextureFormat fmt) : TextureFormat(fmt) {}
};

struct FramebufferAttachmentSpec {
    std::vector<FramebufferTextureSpec> Attachments;
    // Support initializer list of formats
    FramebufferAttachmentSpec() = default;
    FramebufferAttachmentSpec(std::initializer_list<FramebufferTextureFormat> formats){
        for(auto f: formats) Attachments.push_back(FramebufferTextureSpec(f));
    }
    FramebufferAttachmentSpec(std::initializer_list<FramebufferTextureSpec> specs) : Attachments(specs) {}
};

struct FramebufferSpecification {
    uint32_t Width=1280, Height=720;
    FramebufferAttachmentSpec Attachments;
    uint32_t Samples=1;
    bool SwapChainTarget=false;
};

class Framebuffer {
public:
    virtual ~Framebuffer() = default;
    virtual void Bind() =0;
    virtual void Unbind() =0;
    virtual void Resize(uint32_t w, uint32_t h)=0;
    virtual uint32_t GetColorAttachmentRendererID(uint32_t index=0) const =0;
    virtual uint32_t GetDepthAttachmentRendererID() const =0;
    virtual const FramebufferSpecification& GetSpecification() const =0;
    static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
};

} // namespace
