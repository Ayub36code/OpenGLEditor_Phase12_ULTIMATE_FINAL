
#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "graphics/Shader.h"
#include "graphics/VertexArray.h"
#include "graphics/Texture.h"
#include "graphics/Framebuffer.h"
#include "graphics/Camera.h"

namespace Editor {

struct RendererStats { float FPS=0, FrameTimeMs=0; int DrawCalls=0; int TextureCount=0; int FramebufferCount=0; uint64_t FrameIndex=0; uint32_t QuadCount=0; };
struct RenderCommand {
    static void Clear(const glm::vec4& color);
    static void SetViewport(uint32_t x,uint32_t y,uint32_t w,uint32_t h);
    static void DrawIndexed(const std::shared_ptr<VertexArray>& va, uint32_t count=0);
    static void SetLineWidth(float width);
};

class Renderer {
public:
    Renderer(); ~Renderer();
    void Init(); void Shutdown(); void BeginFrame(const glm::vec4& clearColor); void EndFrame(); void OnResize(int w,int h);
    RendererStats& GetStats() { return m_Stats; }
    void BeginScene(const OrthographicCamera& camera); void EndScene();
    void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tiling=1.f, const glm::vec4& tint={1,1,1,1});
    void DrawQuad(const glm::vec2& pos, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tiling=1.f, const glm::vec4& tint={1,1,1,1});
    void DrawRotatedQuad(const glm::vec2& pos, const glm::vec2& size, float rotation, const glm::vec4& color);
    void DrawCheckerboard(const glm::vec2& pos, const glm::vec2& size);
    std::shared_ptr<Shader> GetQuadShader() { return m_QuadShader; }
    std::shared_ptr<Shader> GetTextureShader() { return m_TextureShader; }
    std::shared_ptr<VertexArray> GetQuadVA() { return m_QuadVA; }
private:
    void CreateQuadVA();
    RendererStats m_Stats; bool m_Initialized=false;
    std::shared_ptr<VertexArray> m_QuadVA;
    std::shared_ptr<Shader> m_QuadShader;
    std::shared_ptr<Shader> m_TextureShader;
    std::shared_ptr<Texture2D> m_WhiteTexture;
    std::shared_ptr<Texture2D> m_CheckerTexture;
    struct SceneData { glm::mat4 ViewProjection; } m_SceneData;
};

} // namespace
