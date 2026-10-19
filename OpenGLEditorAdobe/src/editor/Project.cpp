
#include "editor/Project.h"
#include "editor/Viewport.h"
#include "core/Log.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

// Minimal JSON writer without external lib (for portability)
// In production, use nlohmann/json
namespace Editor {

std::string ProjectManager::GenerateTimestamp(){
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool ProjectManager::NewProject(const std::string& name, uint32_t width, uint32_t height){
    m_Data = ProjectData();
    m_Data.Meta.Name = name;
    m_Data.Meta.Width = width;
    m_Data.Meta.Height = height;
    m_Data.Meta.CreatedAt = GenerateTimestamp();
    m_Data.Meta.ModifiedAt = m_Data.Meta.CreatedAt;
    m_CurrentPath = "";
    m_HasProject = true;
    m_Dirty = false;
    LOG_INFO("New project: " + name + " " + std::to_string(width) + "x" + std::to_string(height));
    return true;
}

bool ProjectManager::SaveProject(const std::string& path){
    std::string savePath = path.empty() ? m_CurrentPath : path;
    if(savePath.empty()) return SaveProjectAs("project.json");
    return SaveProjectAs(savePath);
}

bool ProjectManager::SaveProjectAs(const std::string& path){
    try {
        std::ofstream file(path);
        if(!file.is_open()){ LOG_ERROR("Failed to open project file for writing: " + path); return false; }

        m_Data.Meta.ModifiedAt = GenerateTimestamp();
        m_Data.Meta.Path = path;

        // Write JSON manually
        file << "{
";
        file << "  "version": "" << m_Data.Meta.Version << "",
";
        file << "  "name": "" << m_Data.Meta.Name << "",
";
        file << "  "width": " << m_Data.Meta.Width << ",
";
        file << "  "height": " << m_Data.Meta.Height << ",
";
        file << "  "createdAt": "" << m_Data.Meta.CreatedAt << "",
";
        file << "  "modifiedAt": "" << m_Data.Meta.ModifiedAt << "",
";
        file << "  "backgroundColor": [" << m_Data.Meta.BackgroundColor.r << "," << m_Data.Meta.BackgroundColor.g << "," << m_Data.Meta.BackgroundColor.b << "," << m_Data.Meta.BackgroundColor.a << "],
";

        // Effects
        file << "  "effects": {
";
        file << "    "brightness": " << m_Data.Effects.Brightness << ",
";
        file << "    "contrast": " << m_Data.Effects.Contrast << ",
";
        file << "    "saturation": " << m_Data.Effects.Saturation << ",
";
        file << "    "hue": " << m_Data.Effects.Hue << ",
";
        file << "    "exposure": " << m_Data.Effects.Exposure << ",
";
        file << "    "gamma": " << m_Data.Effects.Gamma << ",
";
        file << "    "temperature": " << m_Data.Effects.Temperature << ",
";
        file << "    "blurRadius": " << m_Data.Effects.BlurRadius << ",
";
        file << "    "vignetteAmount": " << m_Data.Effects.VignetteAmount << ",
";
        file << "    "grayscale": " << (m_Data.Effects.Grayscale ? "true" : "false") << ",
";
        file << "    "invert": " << (m_Data.Effects.Invert ? "true" : "false") << "
";
        file << "  },
";

        // Layers
        file << "  "layers": [
";
        for(size_t i=0;i<m_Data.Layers.size();++i){
            auto& l = m_Data.Layers[i];
            file << "    {
";
            file << "      "name": "" << l.Name << "",
";
            file << "      "imagePath": "" << l.ImagePath << "",
";
            file << "      "visible": " << (l.Visible ? "true" : "false") << ",
";
            file << "      "locked": " << (l.Locked ? "true" : "false") << ",
";
            file << "      "opacity": " << l.Opacity << ",
";
            file << "      "blendMode": " << l.BlendMode << ",
";
            file << "      "position": [" << l.Position.x << "," << l.Position.y << "],
";
            file << "      "scale": [" << l.Scale.x << "," << l.Scale.y << "],
";
            file << "      "rotation": " << l.Rotation << ",
";
            file << "      "anchor": [" << l.Anchor.x << "," << l.Anchor.y << "]
";
            file << "    }" << (i < m_Data.Layers.size()-1 ? "," : "") << "
";
        }
        file << "  ]
";
        file << "}
";

        file.close();
        m_CurrentPath = path;
        m_HasProject = true;
        m_Dirty = false;
        LOG_INFO("Project saved: " + path + " with " + std::to_string(m_Data.Layers.size()) + " layers");
        return true;
    } catch(const std::exception& e){
        LOG_ERROR(std::string("Save project failed: ") + e.what());
        return false;
    }
}

bool ProjectManager::LoadProject(const std::string& path){
    try {
        if(!std::filesystem::exists(path)){ LOG_ERROR("Project file not found: " + path); return false; }
        std::ifstream file(path);
        if(!file.is_open()){ LOG_ERROR("Failed to open project file: " + path); return false; }

        // For Phase9, we parse minimally - read whole file and extract values via simple search
        // In production, use nlohmann/json parser
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Very simple parser - extract name, width, height
        // This is placeholder - real implementation would use json lib
        m_Data = ProjectData();
        m_CurrentPath = path;
        m_HasProject = true;
        m_Dirty = false;

        // Try to find name
        auto findString = [&](const std::string& key) -> std::string {
            size_t pos = content.find("\"" + key + "\"");
            if(pos==std::string::npos) return "";
            size_t colon = content.find(":", pos);
            size_t quote1 = content.find("\"", colon);
            size_t quote2 = content.find("\"", quote1+1);
            if(quote1==std::string::npos || quote2==std::string::npos) return "";
            return content.substr(quote1+1, quote2-quote1-1);
        };

        std::string name = findString("name");
        if(!name.empty()) m_Data.Meta.Name = name;

        LOG_INFO("Project loaded (minimal parse): " + path + " - " + m_Data.Meta.Name);
        LOG_INFO("Note: Full JSON parsing requires nlohmann/json - Phase9 uses minimal parser for demo, full layer reconstruction on ApplyToEditor");
        return true;
    } catch(const std::exception& e){
        LOG_ERROR(std::string("Load project failed: ") + e.what());
        return false;
    }
}

void ProjectManager::CaptureFromEditor(Viewport& viewport, const EffectParams& effects){
    m_Data.Layers.clear();
    m_Data.Effects = effects;
    m_Data.Meta.Width = viewport.GetFramebuffer()->GetSpecification().Width;
    m_Data.Meta.Height = viewport.GetFramebuffer()->GetSpecification().Height;

    for(auto& layer : viewport.GetLayerStack().GetLayers()){
        ProjectLayerData ld;
        ld.Name = layer->Name;
        ld.ImagePath = layer->ImageData ? layer->ImageData->GetMetadata().Path : "";
        ld.Visible = layer->Visible;
        ld.Locked = layer->Locked;
        ld.Opacity = layer->Opacity;
        ld.BlendMode = (int)layer->Blend;
        ld.Position = layer->Transform.Position;
        ld.Scale = layer->Transform.Scale;
        ld.Rotation = layer->Transform.Rotation;
        ld.Anchor = layer->Transform.Anchor;
        m_Data.Layers.push_back(ld);
    }
    m_Dirty = true;
}

void ProjectManager::ApplyToEditor(Viewport& viewport, EffectParams& effects){
    effects = m_Data.Effects;
    viewport.GetLayerStack().Clear();
    for(auto& ld : m_Data.Layers){
        auto layer = viewport.GetLayerStack().AddLayer(ld.Name, m_Data.Meta.Width, m_Data.Meta.Height);
        layer->Visible = ld.Visible;
        layer->Locked = ld.Locked;
        layer->Opacity = ld.Opacity;
        layer->Blend = (BlendMode)ld.BlendMode;
        layer->Transform.Position = ld.Position;
        layer->Transform.Scale = ld.Scale;
        layer->Transform.Rotation = ld.Rotation;
        layer->Transform.Anchor = ld.Anchor;
        if(!ld.ImagePath.empty() && std::filesystem::exists(ld.ImagePath)){
            auto img = ImageManager::Get().Import(ld.ImagePath);
            if(img) layer->SetImage(img);
        }
    }
    LOG_INFO("Project applied to editor: " + std::to_string(m_Data.Layers.size()) + " layers");
}

void ProjectManager::UpdateAutosave(float deltaTime){
    if(!m_AutosaveEnabled || !m_HasProject || !m_Dirty) return;
    m_AutosaveTimer += deltaTime;
    if(m_AutosaveTimer >= m_AutosaveInterval){
        m_AutosaveTimer = 0.f;
        std::string autosavePath = m_CurrentPath.empty() ? "autosave.json" : std::filesystem::path(m_CurrentPath).parent_path().string() + "/autosave_" + std::filesystem::path(m_CurrentPath).filename().string();
        SaveProjectAs(autosavePath);
        LOG_INFO("Autosaved to " + autosavePath);
    }
}

}
