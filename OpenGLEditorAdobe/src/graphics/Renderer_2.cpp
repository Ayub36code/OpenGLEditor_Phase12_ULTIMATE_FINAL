
#include "graphics/Renderer.h"
#include "core/Log.h"
#include "core/Time.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Editor {

void RenderCommand::Clear(const glm::vec4& color){ glClearColor(color.r,color.g,color.b,color.a); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }
void RenderCommand::SetViewport(uint32_t x,uint32_t y,uint32_t w,uint32_t h){ glViewport(x,y,w,h); }
void RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray>& va, uint32_t count){
    uint32_t c = count ? count : va->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, nullptr);
}
void RenderCommand::SetLineWidth(float w){ glLineWidth(w); }

Renderer::Renderer() {}
Renderer::~Renderer(){ Shutdown(); }

void Renderer::Init(){
    if(m_Initialized) return;
    LOG_INFO("Renderer Phase4 Init");
    CreateQuadVA();
    const char* quadVS = R"(#version 460 core
    layout(location=0) in vec3 aPos; layout(location=1) in vec2 aTexCoord;
    uniform mat4 u_ViewProjection; uniform mat4 u_Transform; out vec2 v_TexCoord;
    void main(){ v_TexCoord=aTexCoord; gl_Position=u_ViewProjection*u_Transform*vec4(aPos,1.0); })";
    const char* quadFS = R"(#version 460 core
    layout(location=0) out vec4 color; in vec2 v_TexCoord; uniform vec4 u_Color=vec4(1.0); void main(){ color=u_Color; })";
    const char* texVS = R"(#version 460 core
    layout(location=0) in vec3 aPos; layout(location=1) in vec2 aTexCoord;
    uniform mat4 u_ViewProjection; uniform mat4 u_Transform; out vec2 v_TexCoord;
    void main(){ v_TexCoord=aTexCoord; gl_Position=u_ViewProjection*u_Transform*vec4(aPos,1.0); })";
    const char* texFS = R"(#version 460 core
    layout(location=0) out vec4 color; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Tiling=1.0; uniform vec4 u_Tint=vec4(1.0);
    void main(){ vec4 tex=texture(u_Texture, v_TexCoord*u_Tiling); color=tex*u_Tint; })";
    try {
        m_QuadShader = std::make_shared<Shader>("QuadShader", quadVS, quadFS);
        m_TextureShader = std::make_shared<Shader>("TextureShader", texVS, texFS);
    } catch(const std::exception& e){ LOG_ERROR(std::string("Shader init failed: ")+e.what()); throw; }

    TextureSpecification whiteSpec; whiteSpec.Width=1; whiteSpec.Height=1; whiteSpec.Filter=TextureFilter::Nearest;
    m_WhiteTexture = Texture2D::Create(whiteSpec); uint32_t whiteData=0xffffffff; m_WhiteTexture->SetData(&whiteData, sizeof(whiteData));
    TextureSpecification checkerSpec; checkerSpec.Width=2; checkerSpec.Height=2; checkerSpec.Filter=TextureFilter::Nearest; checkerSpec.Wrap=TextureWrap::Repeat;
    m_CheckerTexture = Texture2D::Create(checkerSpec); uint32_t checkerData[4]={0xff808080,0xff606060,0xff606060,0xff808080}; m_CheckerTexture->SetData(checkerData, sizeof(checkerData));
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glEnable(GL_DEPTH_TEST);
    m_Initialized=true; LOG_INFO("Renderer Phase4 ready");
}

void Renderer::Shutdown(){ m_QuadVA.reset(); m_QuadShader.reset(); m_TextureShader.reset(); m_WhiteTexture.reset(); m_CheckerTexture.reset(); m_Initialized=false; }

void Renderer::CreateQuadVA(){
    float vertices[4*5] = {-0.5f,-0.5f,0.f,0.f,0.f, 0.5f,-0.5f,0.f,1.f,0.f, 0.5f,0.5f,0.f,1.f,1.f, -0.5f,0.5f,0.f,0.f,1.f};
    uint32_t indices[6]={0,1,2,2,3,0};
    m_QuadVA = VertexArray::Create();
    auto vb = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
    BufferLayout layout = { {ShaderDataType::Float3, "aPos"}, {ShaderDataType::Float2, "aTexCoord"} };
    vb->SetLayout(layout); m_QuadVA->AddVertexBuffer(vb);
    auto ib = std::make_shared<IndexBuffer>(indices, 6); m_QuadVA->SetIndexBuffer(ib);
}

void Renderer::BeginFrame(const glm::vec4& clearColor){ RenderCommand::Clear(clearColor); m_Stats.DrawCalls=0; m_Stats.QuadCount=0; }
void Renderer::EndFrame(){ m_Stats.FPS=Time::FPS(); m_Stats.FrameTimeMs=Time::DeltaTime()*1000.f; m_Stats.FrameIndex=Time::FrameCount(); }
void Renderer::OnResize(int w,int h){ glViewport(0,0,w,h); }
void Renderer::BeginScene(const OrthographicCamera& camera){ m_SceneData.ViewProjection=camera.GetViewProjectionMatrix(); }
void Renderer::EndScene(){}

void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color){
    m_QuadShader->Bind(); m_QuadShader->SetMat4("u_ViewProjection", m_SceneData.ViewProjection);
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(pos,0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(size,1.f));
    m_QuadShader->SetMat4("u_Transform", transform); m_QuadShader->SetFloat4("u_Color", color);
    m_QuadVA->Bind(); RenderCommand::DrawIndexed(m_QuadVA); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}
void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tiling, const glm::vec4& tint){
    m_TextureShader->Bind(); m_TextureShader->SetMat4("u_ViewProjection", m_SceneData.ViewProjection);
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(pos,0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(size,1.f));
    m_TextureShader->SetMat4("u_Transform", transform); m_TextureShader->SetFloat4("u_Tint", tint); m_TextureShader->SetFloat("u_Tiling", tiling);
    texture->Bind(0); m_TextureShader->SetInt("u_Texture", 0);
    m_QuadVA->Bind(); RenderCommand::DrawIndexed(m_QuadVA); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}
void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tiling, const glm::vec4& tint){
    m_TextureShader->Bind(); m_TextureShader->SetMat4("u_ViewProjection", m_SceneData.ViewProjection);
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(pos,0.f)) * glm::rotate(glm::mat4(1.f), glm::radians(rotation), glm::vec3(0,0,1)) * glm::scale(glm::mat4(1.f), glm::vec3(size,1.f));
    m_TextureShader->SetMat4("u_Transform", transform); m_TextureShader->SetFloat4("u_Tint", tint); m_TextureShader->SetFloat("u_Tiling", tiling);
    texture->Bind(0); m_TextureShader->SetInt("u_Texture", 0);
    m_QuadVA->Bind(); RenderCommand::DrawIndexed(m_QuadVA); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}
void Renderer::DrawRotatedQuad(const glm::vec2& pos, const glm::vec2& size, float rotation, const glm::vec4& color){
    m_QuadShader->Bind(); m_QuadShader->SetMat4("u_ViewProjection", m_SceneData.ViewProjection);
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(pos,0.f)) * glm::rotate(glm::mat4(1.f), glm::radians(rotation), glm::vec3(0,0,1)) * glm::scale(glm::mat4(1.f), glm::vec3(size,1.f));
    m_QuadShader->SetMat4("u_Transform", transform); m_QuadShader->SetFloat4("u_Color", color);
    m_QuadVA->Bind(); RenderCommand::DrawIndexed(m_QuadVA); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}
void Renderer::DrawCheckerboard(const glm::vec2& pos, const glm::vec2& size){ DrawQuad(pos, size, m_CheckerTexture, 10.f, glm::vec4(1.f)); }

}
