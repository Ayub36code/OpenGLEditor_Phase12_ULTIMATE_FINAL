
#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "graphics/Shader.h"
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

namespace Editor {

// Base GPU effect - all processing happens in fragment shader
// CPU -> GPU: texture stays on GPU, only uniform updates

enum class EffectType {
    None,
    Brightness,
    Contrast,
    Saturation,
    Hue,
    Exposure,
    Gamma,
    ColorTemperature,
    Blur,
    Sharpen,
    Grayscale,
    Invert,
    Threshold,
    Vignette,
    ColorAdjustment // unified
};

struct EffectParams {
    float Brightness = 0.0f; // -1..1
    float Contrast = 1.0f;   // 0..3
    float Saturation = 1.0f; // 0..3
    float Hue = 0.0f;        // -180..180 degrees
    float Exposure = 0.0f;   // -3..3
    float Gamma = 1.0f;      // 0.1..3
    float Temperature = 0.0f; // -1..1 warm..cool
    float BlurRadius = 0.0f; // 0..20
    float SharpenAmount = 0.0f; // 0..2
    float Threshold = 0.5f;  // 0..1
    float VignetteAmount = 0.0f; // 0..1
    float VignetteSoftness = 0.5f;
    bool Grayscale = false;
    bool Invert = false;
};

class GPUEffect {
public:
    GPUEffect(EffectType type, const std::string& name);
    virtual ~GPUEffect() = default;

    virtual void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params);
    virtual void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }
    std::shared_ptr<Shader> GetShader() { return m_Shader; }
    EffectType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }
    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool e) { m_Enabled = e; }

protected:
    EffectType m_Type;
    std::string m_Name;
    std::shared_ptr<Shader> m_Shader;
    bool m_Enabled = true;
};

// Unified color adjustment effect (most efficient - single pass)
class ColorAdjustmentEffect : public GPUEffect {
public:
    ColorAdjustmentEffect();
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override;
};

// Individual effects (for learning / modular pipeline)
class BrightnessEffect : public GPUEffect { public: BrightnessEffect(); void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override; };
class ContrastEffect : public GPUEffect { public: ContrastEffect(); void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override; };
class BlurEffect : public GPUEffect {
public:
    BlurEffect();
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override;
private:
    std::shared_ptr<Shader> m_HorizontalShader;
    std::shared_ptr<Shader> m_VerticalShader;
    std::shared_ptr<Framebuffer> m_IntermediateFBO;
};
class VignetteEffect : public GPUEffect { public: VignetteEffect(); void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override; };
class GrayscaleEffect : public GPUEffect { public: GrayscaleEffect(); void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override; };
class InvertEffect : public GPUEffect { public: InvertEffect(); void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params) override; };

// Effect chain - ping-pong FBOs
class EffectChain {
public:
    EffectChain(uint32_t width=1280, uint32_t height=720);
    void Resize(uint32_t w, uint32_t h);
    void AddEffect(std::shared_ptr<GPUEffect> effect);
    void ClearEffects();
    std::vector<std::shared_ptr<GPUEffect>>& GetEffects() { return m_Effects; }

    // Process input texture through all enabled effects, output to targetFBO
    void Process(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO, const EffectParams& params);

    EffectParams& GetParams() { return m_Params; }
    void ResetParams() { m_Params = EffectParams(); }

    // Get final texture after processing
    uint32_t GetOutputTexture() const { return m_OutputTexture; }

private:
    std::vector<std::shared_ptr<GPUEffect>> m_Effects;
    EffectParams m_Params;
    std::shared_ptr<Framebuffer> m_FBO[2];
    uint32_t m_Width, m_Height;
    uint32_t m_OutputTexture = 0;
    int m_CurrentFBO = 0;

    std::shared_ptr<Shader> m_CopyShader;
};

} // namespace
