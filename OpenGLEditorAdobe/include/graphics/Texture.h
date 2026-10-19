
#pragma once
#include <string>
#include <cstdint>
#include <memory>

namespace Editor {

enum class TextureFormat { None=0, RGB8, RGBA8, RGBA16F, RGBA32F, Depth24Stencil8 };
enum class TextureFilter { Linear, Nearest };
enum class TextureWrap { Repeat, Clamp, Mirror };

struct TextureSpecification {
    uint32_t Width=1, Height=1;
    TextureFormat Format=TextureFormat::RGBA8;
    TextureFilter Filter=TextureFilter::Linear;
    TextureWrap Wrap=TextureWrap::Clamp;
    bool GenerateMips=true;
    bool SRGB=false;
};

class Texture {
public:
    virtual ~Texture() = default;
    virtual uint32_t GetRendererID() const =0;
    virtual uint32_t GetWidth() const =0;
    virtual uint32_t GetHeight() const =0;
    virtual void Bind(uint32_t slot=0) const =0;
    virtual void SetData(void* data, uint32_t size)=0;
    virtual bool IsLoaded() const { return true; }
};

class Texture2D : public Texture {
public:
    Texture2D(const TextureSpecification& spec);
    Texture2D(const std::string& path, const TextureSpecification& spec = {});
    ~Texture2D();

    uint32_t GetRendererID() const override { return m_RendererID; }
    uint32_t GetWidth() const override { return m_Spec.Width; }
    uint32_t GetHeight() const override { return m_Spec.Height; }
    void Bind(uint32_t slot=0) const override;
    void Unbind() const;
    void SetData(void* data, uint32_t size) override;

    const TextureSpecification& GetSpec() const { return m_Spec; }
    const std::string& GetPath() const { return m_Path; }
    void Resize(uint32_t w, uint32_t h);

    static std::shared_ptr<Texture2D> Create(const TextureSpecification& spec);
    static std::shared_ptr<Texture2D> Create(const std::string& path);

private:
    void Allocate();
    void SetParameters();

    TextureSpecification m_Spec;
    std::string m_Path;
    uint32_t m_RendererID=0;
    bool m_Loaded=false;
};

} // namespace
