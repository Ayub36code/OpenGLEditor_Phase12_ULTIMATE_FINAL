
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace Editor {

// Phase 12: Cloud Collaboration - Figma-like multiplayer editing

struct Collaborator {
    std::string UserID;
    std::string Name;
    std::string AvatarURL;
    glm::vec4 Color{0.3f,0.6f,1.f,1.f}; // Cursor color
    glm::vec2 CursorPos{0,0};
    std::string ActiveLayer;
    bool IsOnline=true;
    float LastSeen=0.f;
};

struct EditOperation {
    enum Type { LayerAdd, LayerDelete, LayerMove, LayerTransform, LayerOpacity, ImageImport, EffectChange, CommentAdd } OpType;
    std::string UserID;
    std::string LayerName;
    float Timestamp=0.f;
    std::string Data; // JSON data
    std::string OperationID; // UUID for OT
};

class CollaborationManager {
public:
    static CollaborationManager& Get() { static CollaborationManager instance; return instance; }

    void Init(const std::string& projectID);
    void Connect(const std::string& serverURL="wss://collab.opengleditor.com");
    void Disconnect();

    // Real-time sync (Operational Transform like Google Docs / Figma)
    void SendOperation(const EditOperation& op);
    void ReceiveOperation(const EditOperation& op);
    void ApplyOperation(const EditOperation& op);

    // Presence
    void UpdateCursor(const glm::vec2& pos, const std::string& layerName="");
    std::vector<Collaborator>& GetCollaborators() { return m_Collaborators; }

    // Comments (Figma-like)
    struct Comment {
        std::string ID;
        std::string UserID;
        std::string Text;
        glm::vec2 Position;
        std::string LayerName;
        float Timestamp;
        bool Resolved=false;
        std::vector<Comment> Replies;
    };
    void AddComment(const glm::vec2& pos, const std::string& text);
    std::vector<Comment>& GetComments() { return m_Comments; }

    // Version history (like Figma)
    struct Version {
        std::string ID;
        std::string Message;
        std::string Author;
        float Timestamp;
        std::string ThumbnailPath;
    };
    void SaveVersion(const std::string& message);
    std::vector<Version>& GetVersions() { return m_Versions; }

    bool IsConnected() const { return m_Connected; }
    bool IsOnline() const { return m_Connected; }

    std::function<void(const EditOperation&)> OnOperationReceived;
    std::function<void(const Collaborator&)> OnCollaboratorJoined;

private:
    bool m_Connected=false;
    std::string m_ProjectID;
    std::string m_ServerURL;
    std::vector<Collaborator> m_Collaborators;
    std::vector<Comment> m_Comments;
    std::vector<Version> m_Versions;
    std::vector<EditOperation> m_OperationHistory;
};

// Plugin SDK - C++ plugins + JS scripting
enum class PluginType { Effect, Tool, Importer, Exporter, Panel, AIFilter };

struct PluginInfo {
    std::string ID;
    std::string Name;
    std::string Version;
    std::string Author;
    std::string Description;
    PluginType Type;
    std::string EntryPoint; // DLL/so path or JS file
    bool Enabled=true;
};

class Plugin {
public:
    Plugin(const PluginInfo& info) : m_Info(info) {}
    virtual ~Plugin() = default;
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnDrawUI() {}
    const PluginInfo& GetInfo() const { return m_Info; }
    bool IsLoaded() const { return m_Loaded; }

protected:
    PluginInfo m_Info;
    bool m_Loaded=false;
};

class PluginManager {
public:
    static PluginManager& Get() { static PluginManager instance; return instance; }
    void Init();
    void LoadPluginsFromFolder(const std::string& folder="plugins/");
    void RegisterPlugin(std::shared_ptr<Plugin> plugin);
    void UnloadPlugin(const std::string& id);
    std::vector<std::shared_ptr<Plugin>>& GetPlugins() { return m_Plugins; }
    std::shared_ptr<Plugin> GetPlugin(const std::string& id);

    // JS scripting (via quickjs or similar - stub)
    bool ExecuteJSScript(const std::string& code);
    void RegisterJSAPI();

private:
    std::vector<std::shared_ptr<Plugin>> m_Plugins;
};

// 3D Layers - PBR materials, model import
struct MaterialPBR {
    glm::vec4 Albedo{1,1,1,1};
    float Metallic=0.f;
    float Roughness=0.5f;
    float AO=1.f;
    uint32_t AlbedoMap=0;
    uint32_t NormalMap=0;
    uint32_t MetallicMap=0;
    uint32_t RoughnessMap=0;
};

struct Model3D {
    std::string Path;
    std::vector<glm::vec3> Vertices;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;
    std::vector<uint32_t> Indices;
    MaterialPBR Material;
    glm::mat4 Transform{1.f};
    bool Loaded=false;
};

class ModelLoader {
public:
    static std::shared_ptr<Model3D> LoadGLTF(const std::string& path);
    static std::shared_ptr<Model3D> LoadOBJ(const std::string& path);
};

class Renderer3D {
public:
    Renderer3D(uint32_t w=1280, uint32_t h=720);
    void Resize(uint32_t w, uint32_t h);
    void BeginFrame(const glm::vec3& cameraPos, const glm::mat4& viewProj);
    void DrawModel(const Model3D& model);
    void DrawGrid();
    void EndFrame();

private:
    std::shared_ptr<class Shader> m_PBRShader;
    std::shared_ptr<class Framebuffer> m_FBO;
    uint32_t m_Width, m_Height;
};

} // namespace
