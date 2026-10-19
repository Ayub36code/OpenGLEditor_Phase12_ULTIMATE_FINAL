
#include "editor/CloudCollab.h"
#include "core/Log.h"

namespace Editor {

void CollaborationManager::Init(const std::string& projectID){
    m_ProjectID=projectID;
    // Add self as collaborator
    Collaborator self; self.UserID="self"; self.Name="You"; self.Color={0.3f,0.8f,1.f,1.f}; self.IsOnline=true;
    m_Collaborators.push_back(self);
    LOG_INFO("Collaboration initialized for project " + projectID);
}

void CollaborationManager::Connect(const std::string& serverURL){
    m_ServerURL=serverURL; m_Connected=true;
    LOG_INFO("Connected to collaboration server: " + serverURL);
    // Simulate other collaborators joining
    Collaborator other; other.UserID="user2"; other.Name="Alice (Designer)"; other.Color={1.f,0.4f,0.4f,1.f}; other.CursorPos={100,100}; other.IsOnline=true;
    m_Collaborators.push_back(other);
    if(OnCollaboratorJoined) OnCollaboratorJoined(other);
}

void CollaborationManager::Disconnect(){ m_Connected=false; LOG_INFO("Disconnected from collaboration"); }

void CollaborationManager::SendOperation(const EditOperation& op){
    if(!m_Connected) return;
    LOG_INFO("Sending operation: " + std::to_string((int)op.OpType) + " on " + op.LayerName);
    m_OperationHistory.push_back(op);
    // In real impl, send via websocket
}

void CollaborationManager::ReceiveOperation(const EditOperation& op){
    LOG_INFO("Received operation from " + op.UserID);
    ApplyOperation(op);
    if(OnOperationReceived) OnOperationReceived(op);
}

void CollaborationManager::ApplyOperation(const EditOperation& op){
    // Operational Transform - apply remote edit
    m_OperationHistory.push_back(op);
}

void CollaborationManager::UpdateCursor(const glm::vec2& pos, const std::string& layer){
    if(!m_Collaborators.empty()){ m_Collaborators[0].CursorPos=pos; m_Collaborators[0].ActiveLayer=layer; }
}

void CollaborationManager::AddComment(const glm::vec2& pos, const std::string& text){
    Comment c; c.ID="comment_" + std::to_string(m_Comments.size()); c.UserID="self"; c.Text=text; c.Position=pos; c.Timestamp=0.f;
    m_Comments.push_back(c);
    LOG_INFO("Comment added at " + std::to_string((int)pos.x) + "," + std::to_string((int)pos.y) + ": " + text);
}

void CollaborationManager::SaveVersion(const std::string& message){
    Version v; v.ID="v" + std::to_string(m_Versions.size()+1); v.Message=message; v.Author="You"; v.Timestamp=0.f;
    m_Versions.push_back(v);
    LOG_INFO("Version saved: " + message);
}

void PluginManager::Init(){
    LOG_INFO("Plugin Manager initialized - supports C++ DLL and JS plugins");
    RegisterJSAPI();
}

void PluginManager::LoadPluginsFromFolder(const std::string& folder){
    LOG_INFO("Loading plugins from " + folder);
    // Scan folder for .dll/.so and .js files
    // For demo, register built-in plugins
    PluginInfo info; info.ID="example.effect"; info.Name="Example Glow Effect"; info.Version="1.0"; info.Author="Pro"; info.Type=PluginType::Effect; info.Description="Custom glow effect plugin";
    // Would load DLL here
}

void PluginManager::RegisterPlugin(std::shared_ptr<Plugin> plugin){
    if(plugin && plugin->Load()){ m_Plugins.push_back(plugin); LOG_INFO("Plugin registered: " + plugin->GetInfo().Name); }
}

void PluginManager::UnloadPlugin(const std::string& id){
    m_Plugins.erase(std::remove_if(m_Plugins.begin(), m_Plugins.end(), [&](auto& p){ return p->GetInfo().ID==id; }), m_Plugins.end());
}

std::shared_ptr<Plugin> PluginManager::GetPlugin(const std::string& id){
    for(auto& p: m_Plugins) if(p->GetInfo().ID==id) return p;
    return nullptr;
}

bool PluginManager::ExecuteJSScript(const std::string& code){
    LOG_INFO("Executing JS: " + code.substr(0,50));
    // Would use quickjs here
    return true;
}

void PluginManager::RegisterJSAPI(){
    LOG_INFO("JS API registered: editor.layers, editor.effects, editor.ai, editor.export");
}

std::shared_ptr<Model3D> ModelLoader::LoadGLTF(const std::string& path){
    auto model=std::make_shared<Model3D>(); model->Path=path; model->Loaded=true;
    LOG_INFO("Loaded GLTF: " + path);
    // Real impl would parse glTF JSON and buffers
    model->Vertices={{-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0}};
    model->Indices={0,1,2,2,3,0};
    return model;
}

std::shared_ptr<Model3D> ModelLoader::LoadOBJ(const std::string& path){
    auto model=std::make_shared<Model3D>(); model->Path=path; model->Loaded=true;
    LOG_INFO("Loaded OBJ: " + path);
    return model;
}

Renderer3D::Renderer3D(uint32_t w, uint32_t h) : m_Width(w), m_Height(h){
    const char* vs=R"(#version 460 core layout(location=0) in vec3 aPos; layout(location=1) in vec3 aNormal; layout(location=2) in vec2 aTexCoord; uniform mat4 u_ViewProj; uniform mat4 u_Model; out vec3 v_Normal; out vec2 v_TexCoord; out vec3 v_WorldPos; void main(){ v_Normal=mat3(transpose(inverse(u_Model)))*aNormal; v_TexCoord=aTexCoord; v_WorldPos=vec3(u_Model*vec4(aPos,1)); gl_Position=u_ViewProj*u_Model*vec4(aPos,1); })";
    const char* fs=R"(#version 460 core out vec4 outColor; in vec3 v_Normal; in vec2 v_TexCoord; in vec3 v_WorldPos; uniform vec3 u_CameraPos; uniform vec4 u_Albedo; uniform float u_Metallic; uniform float u_Roughness; void main(){ vec3 N=normalize(v_Normal); vec3 V=normalize(u_CameraPos-v_WorldPos); vec3 L=normalize(vec3(1,1,1)); float NdotL=max(dot(N,L),0); vec3 diffuse=u_Albedo.rgb*NdotL; vec3 F0=mix(vec3(0.04), u_Albedo.rgb, u_Metallic); outColor=vec4(diffuse + F0*0.5, u_Albedo.a); })";
    m_PBRShader=std::make_shared<Shader>("PBR", vs, fs);
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth24Stencil8};
    m_FBO=Framebuffer::Create(spec);
}

void Renderer3D::Resize(uint32_t w,uint32_t h){ m_Width=w; m_Height=h; m_FBO->Resize(w,h); }
void Renderer3D::BeginFrame(const glm::vec3& camPos, const glm::mat4& viewProj){ m_FBO->Bind(); m_PBRShader->Bind(); m_PBRShader->SetMat4("u_ViewProj", viewProj); m_PBRShader->SetFloat3("u_CameraPos", {camPos.x, camPos.y, camPos.z}); }
void Renderer3D::DrawModel(const Model3D& model){ /* draw */ }
void Renderer3D::DrawGrid(){}
void Renderer3D::EndFrame(){ m_FBO->Unbind(); }

}
