
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "editor/Layer.h"
#include "graphics/Effects.h"

namespace Editor {

struct ProjectMetadata {
    std::string Name = "Untitled";
    std::string Path = "";
    std::string Version = "1.0.0";
    std::string CreatedAt = "";
    std::string ModifiedAt = "";
    uint32_t Width = 1920;
    uint32_t Height = 1080;
    glm::vec4 BackgroundColor{0.18f,0.18f,0.19f,1.f};
};

struct ProjectLayerData {
    std::string Name;
    std::string ImagePath; // original image path if any
    bool Visible = true;
    bool Locked = false;
    float Opacity = 1.f;
    int BlendMode = 0;
    glm::vec2 Position{0,0};
    glm::vec2 Scale{1,1};
    float Rotation = 0.f;
    glm::vec2 Anchor{0.5f,0.5f};
};

struct ProjectData {
    ProjectMetadata Meta;
    std::vector<ProjectLayerData> Layers;
    EffectParams Effects;
    std::string ThumbnailPath = "";
};

class ProjectManager {
public:
    static ProjectManager& Get() { static ProjectManager instance; return instance; }

    bool NewProject(const std::string& name, uint32_t width=1920, uint32_t height=1080);
    bool SaveProject(const std::string& path="");
    bool SaveProjectAs(const std::string& path);
    bool LoadProject(const std::string& path);
    bool HasProject() const { return m_HasProject; }

    ProjectData& GetData() { return m_Data; }
    const ProjectData& GetData() const { return m_Data; }
    const std::string& GetCurrentPath() const { return m_CurrentPath; }
    bool IsDirty() const { return m_Dirty; }
    void SetDirty(bool d) { m_Dirty = d; }

    // Convert current editor state to project data
    void CaptureFromEditor(class Viewport& viewport, const EffectParams& effects);
    // Apply project data to editor
    void ApplyToEditor(class Viewport& viewport, EffectParams& effects);

    // Autosave
    void SetAutosave(bool enabled, float intervalSec=300.f) { m_AutosaveEnabled=enabled; m_AutosaveInterval=intervalSec; }
    void UpdateAutosave(float deltaTime);

private:
    ProjectManager() = default;
    std::string GenerateTimestamp();

    ProjectData m_Data;
    std::string m_CurrentPath;
    bool m_HasProject = false;
    bool m_Dirty = false;
    bool m_AutosaveEnabled = false;
    float m_AutosaveInterval = 300.f;
    float m_AutosaveTimer = 0.f;
};

} // namespace
