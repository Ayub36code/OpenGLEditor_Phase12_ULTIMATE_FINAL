
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "core/Transform.h"
#include "graphics/Texture.h"

namespace Editor {

struct ImageMetadata {
    std::string Name = "Untitled";
    std::string Path = "";
    uint32_t Width = 0;
    uint32_t Height = 0;
    int Channels = 4;
    std::string Format = "RGBA";
};

class Image {
public:
    Image() = default;
    ~Image() = default;

    static std::shared_ptr<Image> LoadFromFile(const std::string& path);
    static std::shared_ptr<Image> CreateFromPixels(uint32_t w, uint32_t h, const unsigned char* data, int channels=4, const std::string& name="");

    bool Resize(uint32_t newW, uint32_t newH);
    void FlipHorizontal();
    void FlipVertical();
    void Rotate90(bool clockwise=true);

    uint32_t GetWidth() const { return m_Meta.Width; }
    uint32_t GetHeight() const { return m_Meta.Height; }
    const ImageMetadata& GetMetadata() const { return m_Meta; }
    const std::vector<unsigned char>& GetPixels() const { return m_Pixels; }
    std::shared_ptr<Texture2D> GetTexture() const { return m_Texture; }

    Transform& GetTransform() { return m_Transform; }
    const Transform& GetTransform() const { return m_Transform; }

    void UpdateTexture();

private:
    ImageMetadata m_Meta;
    std::vector<unsigned char> m_Pixels;
    std::shared_ptr<Texture2D> m_Texture;
    Transform m_Transform;
};

class ImageManager {
public:
    static ImageManager& Get() { static ImageManager instance; return instance; }
    std::shared_ptr<Image> Import(const std::string& path);
    std::shared_ptr<Image> GetActive() const { return m_Active; }
    void SetActive(std::shared_ptr<Image> img) { m_Active=img; }
    std::vector<std::shared_ptr<Image>> GetAll() const { return m_Images; }
    void Add(std::shared_ptr<Image> img);
private:
    std::vector<std::shared_ptr<Image>> m_Images;
    std::shared_ptr<Image> m_Active;
};

} // namespace
