
#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "graphics/Shader.h"
#include "graphics/Buffer.h"
#include "graphics/Texture.h"
#include "graphics/Framebuffer.h"
#include "graphics/Camera.h"

namespace Editor {

struct RendererStats {
    uint32_t DrawCalls = 0;
    uint32_t QuadCount = 0;
    float FPS = 0.f;
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    void Init();
    void Shutdown();
    void OnResize(uint32_t w, uint32_t h);

    void BeginFrame(const glm::vec4& clearColor);
    void BeginScene(const OrthographicCamera& camera);
    void EndScene();
    void EndFrame();

    void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    void DrawQuad(const glm::vec2& position, const glm::vec2& size, std::shared_ptr<Texture2D> texture, float tiling=1.f, const glm::vec4& tint={1,1,1,1});
    void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, std::shared_ptr<Texture2D> texture, float tiling=1.f, const glm::vec4& tint={1,1,1,1});
    void DrawCheckerboard(const glm::vec2& pos, const glm::vec2& size);

    RendererStats& GetStats() { return m_Stats; }
    std::shared_ptr<Shader> GetTextureShader() { return m_TextureShader; }

private:
    std::shared_ptr<Shader> m_QuadShader;
    std::shared_ptr<Shader> m_TextureShader;
    std::shared_ptr<VertexArray> m_QuadVA;
    std::shared_ptr<VertexBuffer> m_QuadVB;
    std::shared_ptr<IndexBuffer> m_QuadIB;
    std::shared_ptr<Texture2D> m_WhiteTexture;
    std::shared_ptr<Texture2D> m_CheckerTexture;
    OrthographicCamera* m_Camera = nullptr;
    RendererStats m_Stats;
    glm::mat4 m_ViewProjection{1.f};
};

} // namespace
