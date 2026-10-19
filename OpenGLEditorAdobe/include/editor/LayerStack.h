
#pragma once
#include "editor/Layer.h"
#include "graphics/Framebuffer.h"
#include "graphics/Shader.h"
#include <vector>
#include <memory>

namespace Editor {

class Renderer;
class OrthographicCamera;

class LayerStack {
public:
    LayerStack();
    ~LayerStack() = default;

    // Layer management
    std::shared_ptr<Layer> AddLayer(const std::string& name = "", uint32_t w=800, uint32_t h=600, const glm::vec4& color={0,0,0,0});
    std::shared_ptr<Layer> AddImageLayer(std::shared_ptr<Image> img, const std::string& name="");
    std::shared_ptr<Layer> AddSolidColorLayer(const std::string& name, const glm::vec4& color, uint32_t w, uint32_t h);
    void DeleteLayer(int index);
    void DeleteLayer(std::shared_ptr<Layer> layer);
    std::shared_ptr<Layer> DuplicateLayer(int index);
    void MoveLayer(int fromIndex, int toIndex);
    void MoveUp(int index);
    void MoveDown(int index);

    void Clear();
    size_t Size() const { return m_Layers.size(); }
    std::vector<std::shared_ptr<Layer>>& GetLayers() { return m_Layers; }
    const std::vector<std::shared_ptr<Layer>>& GetLayers() const { return m_Layers; }

    int GetActiveIndex() const { return m_ActiveIndex; }
    void SetActiveIndex(int idx);
    std::shared_ptr<Layer> GetActive() const;
    std::shared_ptr<Layer> GetLayer(int idx) const;

    // Compositing - renders stack to target framebuffer using Renderer
    void Composite(Renderer& renderer, OrthographicCamera& camera, std::shared_ptr<Framebuffer> targetFBO);

    // FBOs for compositing
    void EnsureCompositeFBO(uint32_t w, uint32_t h);
    uint32_t GetCompositeTextureID() const;

private:
    void InitBlendShaders();

    std::vector<std::shared_ptr<Layer>> m_Layers;
    int m_ActiveIndex = -1;

    // Two FBOs ping-pong for blend modes that need background sampling
    std::shared_ptr<Framebuffer> m_CompositeFBO[2];
    int m_CurrentFBO = 0;

    // Shaders for each blend mode
    std::shared_ptr<Shader> m_BlendShaders[(int)BlendMode::COUNT];

    // For Normal blending we can use simple texture shader with alpha
    // For other modes we need shader that samples base + blend
};

} // namespace
