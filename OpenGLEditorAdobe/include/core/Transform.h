
#pragma once
#include <glm/glm.hpp>

namespace Editor {

struct Transform {
    glm::vec2 Position{0.f,0.f};
    glm::vec2 Scale{1.f,1.f};
    float Rotation=0.f; // degrees
    glm::vec2 Anchor{0.5f,0.5f};
    glm::vec2 Origin{0.f,0.f};

    glm::mat4 GetMatrix() const;
    glm::mat4 GetMatrix(const glm::vec2& size) const;
};

} // namespace
