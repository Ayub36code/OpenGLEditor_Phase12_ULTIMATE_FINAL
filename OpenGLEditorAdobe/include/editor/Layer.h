
#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "core/Transform.h"
#include "graphics/Texture.h"

namespace Editor {

enum class BlendMode { Normal=0, Multiply, Screen, Overlay, Add, Subtract, Difference };

struct Layer {
    std::string Name = "Layer";
    bool Visible = true;
    bool Locked = false;
    float Opacity = 1.f;
    BlendMode Blend = BlendMode::Normal;
    Transform TransformData; // FIXED: was Transform Transform; which shadows type
    std::shared_ptr<class Image> ImageData;
    std::shared_ptr<Texture2D> Texture;
    std::shared_ptr<class LayerMask> Mask;
    std::shared_ptr<struct LayerStyle> Style;
    int Index = 0;

    Layer(const std::string& name="Layer", uint32_t w=512, uint32_t h=512, const glm::vec4& color={0,0,0,0});
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    void SetImage(std::shared_ptr<Image> img);
    void SetVisible(bool v) { Visible=v; }
    void SetOpacity(float o) { Opacity=o; }
    void SetLocked(bool l) { Locked=l; }
};

// Compatibility aliases so old code Transform.Position still works via TransformData
// Provide getters
inline glm::vec2& GetLayerPos(Layer& l){ return l.TransformData.Position; }

} // namespace
