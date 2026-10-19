
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "graphics/Framebuffer.h"
#include "graphics/Shader.h"

namespace Editor {

// Phase 11: AI Features - Adobe Firefly level

enum class AIModelType {
    BackgroundRemoval,  // U2Net / BiRefNet
    SuperResolution,    // ESRGAN
    AutoEnhance,        // Lighting + color auto
    StyleTransfer,      // Neural style
    Inpainting,         // LaMa / Stable Diffusion inpaint
    Outpainting,        // Expand image
    TextToImage,        // Stable Diffusion txt2img
    Denoise,            // AI denoise
    FaceEnhance,        // GFPGAN
    Colorize            // DeOldify
};

struct AIRequest {
    AIModelType Type;
    std::string Prompt = ""; // for text2img / inpaint
    std::string NegativePrompt = "";
    float Strength = 0.75f;
    int Steps = 20;
    float GuidanceScale = 7.5f;
    glm::ivec4 MaskBounds{0,0,0,0}; // for inpainting
    std::string InputImagePath;
    std::string MaskImagePath;
};

struct AIResult {
    bool Success = false;
    std::string OutputPath;
    uint32_t TextureID = 0;
    float ProcessingTime = 0.f;
    std::string ErrorMessage;
    std::vector<std::string> Logs;
};

// AI Model interface (can be ONNX Runtime, or external API)
class AIModel {
public:
    AIModel(AIModelType type, const std::string& name) : m_Type(type), m_Name(name) {}
    virtual ~AIModel() = default;
    virtual bool Load(const std::string& modelPath) = 0;
    virtual AIResult Process(const AIRequest& request) = 0;
    virtual bool IsLoaded() const { return m_Loaded; }

    AIModelType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }

protected:
    AIModelType m_Type;
    std::string m_Name;
    bool m_Loaded = false;
    std::shared_ptr<Shader> m_Shader; // for GPU accelerated models
};

// GPU-based AI effects (run 100% in shader for real-time preview)

class BackgroundRemovalAI : public AIModel {
public:
    BackgroundRemovalAI();
    bool Load(const std::string& modelPath) override;
    AIResult Process(const AIRequest& request) override;

private:
    std::shared_ptr<Shader> m_MattingShader;
    std::shared_ptr<Framebuffer> m_TempFBO;
};

class SuperResolutionAI : public AIModel {
public:
    SuperResolutionAI();
    bool Load(const std::string& modelPath) override;
    AIResult Process(const AIRequest& request) override;
    void SetScale(int scale) { m_Scale = scale; }
private:
    int m_Scale = 4; // 2x,4x,8x
    std::shared_ptr<Shader> m_UpscaleShader;
};

class AutoEnhanceAI : public AIModel {
public:
    AutoEnhanceAI();
    bool Load(const std::string& modelPath) override;
    AIResult Process(const AIRequest& request) override;
private:
    std::shared_ptr<Shader> m_EnhanceShader;
};

class InpaintingAI : public AIModel {
public:
    InpaintingAI();
    bool Load(const std::string& modelPath) override;
    AIResult Process(const AIRequest& request) override;
private:
    std::shared_ptr<Shader> m_InpaintShader;
};

// AI Manager - handles all models, queue, GPU fallback
class AIManager {
public:
    static AIManager& Get() { static AIManager instance; return instance; }

    void Init();
    void RegisterModel(std::shared_ptr<AIModel> model);
    std::shared_ptr<AIModel> GetModel(AIModelType type);

    // Async processing
    void QueueRequest(const AIRequest& request, std::function<void(const AIResult&)> callback);
    void Update(float deltaTime); // process queue

    bool IsProcessing() const { return m_IsProcessing; }
    float GetProgress() const { return m_Progress; }

    // Quick GPU effects (real-time)
    uint32_t ApplyBackgroundRemoval(uint32_t inputTexture, uint32_t width, uint32_t height);
    uint32_t ApplySuperResolution(uint32_t inputTexture, uint32_t width, uint32_t height, int scale=4);
    uint32_t ApplyAutoEnhance(uint32_t inputTexture, uint32_t width, uint32_t height);

private:
    std::vector<std::shared_ptr<AIModel>> m_Models;
    std::vector<std::pair<AIRequest, std::function<void(const AIResult&)>>> m_Queue;
    bool m_IsProcessing = false;
    float m_Progress = 0.f;
    std::shared_ptr<Framebuffer> m_TempFBO;
};

} // namespace
