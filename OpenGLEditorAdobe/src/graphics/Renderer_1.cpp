
#include "graphics/Renderer.h"
#include "core/Log.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Editor {

Renderer::Renderer() {}
Renderer::~Renderer(){}

void Renderer::Init(){
    // White texture 1x1
    TextureSpecification whiteSpec; whiteSpec.Width=1; whiteSpec.Height=1;
    m_WhiteTexture=Texture2D::Create(whiteSpec);
    uint32_t white=0xffffffff; m_WhiteTexture->SetData(&white, sizeof(white));

    // Checker texture
    TextureSpecification checkerSpec; checkerSpec.Width=16; checkerSpec.Height=16; checkerSpec.Filter=TextureFilter::Nearest; checkerSpec.Wrap=TextureWrap::Repeat;
    m_CheckerTexture=Texture2D::Create(checkerSpec);
    uint32_t checkerData[256]; for(int y=0;y<16;y++) for(int x=0;x<16;x++){ bool even=((x/4+y/4)%2)==0; checkerData[y*16+x]=even?0xff999999:0xff666666; }
    m_CheckerTexture->SetData(checkerData, sizeof(checkerData));

    // Quad vertices: pos(3) + texcoord(2)
    float vertices[] = { -0.5f,-0.5f,0.f, 0.f,0.f,  0.5f,-0.5f,0.f, 1.f,0.f,  0.5f,0.5f,0.f, 1.f,1.f,  -0.5f,0.5f,0.f, 0.f,1.f };
    uint32_t indices[] = {0,1,2, 2,3,0};
    m_QuadVA = VertexArray::Create();
    m_QuadVB = VertexBuffer::Create(vertices, sizeof(vertices));
    BufferLayout layout = { {ShaderDataType::Float3, "aPos"}, {ShaderDataType::Float2, "aTexCoord"} };
    m_QuadVB->SetLayout(layout);
    m_QuadVA->AddVertexBuffer(m_QuadVB);
    m_QuadIB = IndexBuffer::Create(indices, 6);
    m_QuadVA->SetIndexBuffer(m_QuadIB);

    const char* vs = R"(#version 460 core
    layout(location=0) in vec3 aPos; layout(location=1) in vec2 aTexCoord;
    uniform mat4 u_ViewProjection; uniform mat4 u_Transform;
    out vec2 v_TexCoord;
    void main(){ v_TexCoord=aTexCoord; gl_Position=u_ViewProjection*u_Transform*vec4(aPos,1.0); })";

    const char* fsQuad = R"(#version 460 core
    layout(location=0) out vec4 outColor; uniform vec4 u_Color; void main(){ outColor=u_Color; })";

    const char* fsTex = R"(#version 460 core
    layout(location=0) out vec4 outColor; in vec2 v_TexCoord;
    uniform sampler2D u_Texture; uniform vec4 u_Tint; uniform float u_Tiling=1.0;
    void main(){ vec4 tex=texture(u_Texture, v_TexCoord*u_Tiling); outColor=tex*u_Tint; })";

    m_QuadShader = std::make_shared<Shader>("Quad", vs, fsQuad);
    m_TextureShader = std::make_shared<Shader>("Texture", vs, fsTex);
    LOG_INFO("Renderer initialized - Quad VAO + Shaders");
}

void Renderer::Shutdown(){
    m_QuadVA.reset(); m_QuadVB.reset(); m_QuadIB.reset();
    m_QuadShader.reset(); m_TextureShader.reset();
    m_WhiteTexture.reset(); m_CheckerTexture.reset();
}

void Renderer::OnResize(uint32_t w, uint32_t h){ (void)w; (void)h; }

void Renderer::BeginFrame(const glm::vec4& clearColor){
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_Stats.DrawCalls=0; m_Stats.QuadCount=0;
}

void Renderer::BeginScene(const OrthographicCamera& camera){
    m_Camera = const_cast<OrthographicCamera*>(&camera);
    m_ViewProjection = camera.GetViewProjectionMatrix();
}

void Renderer::EndScene(){ m_Camera=nullptr; }
void Renderer::EndFrame(){}

void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color){
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(position,0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(size,1.f));
    m_QuadShader->Bind(); m_QuadShader->SetMat4("u_ViewProjection", m_ViewProjection); m_QuadShader->SetMat4("u_Transform", transform); m_QuadShader->SetFloat4("u_Color", color);
    m_QuadVA->Bind(); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}

void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, std::shared_ptr<Texture2D> texture, float tiling, const glm::vec4& tint){
    DrawQuad(position, size, 0.f, texture, tiling, tint);
}

void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, std::shared_ptr<Texture2D> texture, float tiling, const glm::vec4& tint){
    if(!texture) texture=m_WhiteTexture;
    glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(position,0.f));
    transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0,0,1));
    transform = glm::scale(transform, glm::vec3(size,1.f));
    m_TextureShader->Bind(); m_TextureShader->SetMat4("u_ViewProjection", m_ViewProjection); m_TextureShader->SetMat4("u_Transform", transform);
    m_TextureShader->SetFloat4("u_Tint", tint); m_TextureShader->SetFloat("u_Tiling", tiling);
    texture->Bind(0); m_TextureShader->SetInt("u_Texture", 0);
    m_QuadVA->Bind(); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); m_Stats.DrawCalls++; m_Stats.QuadCount++;
}

void Renderer::DrawCheckerboard(const glm::vec2& pos, const glm::vec2& size){
    DrawQuad(pos, size, m_CheckerTexture, size.x/16.f, {1,1,1,1});
}

}
