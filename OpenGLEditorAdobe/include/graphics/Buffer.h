
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace Editor {

enum class ShaderDataType {
    None=0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch(type) {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Float2: return 8;
        case ShaderDataType::Float3: return 12;
        case ShaderDataType::Float4: return 16;
        case ShaderDataType::Mat3:   return 36;
        case ShaderDataType::Mat4:   return 64;
        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Int2:   return 8;
        case ShaderDataType::Int3:   return 12;
        case ShaderDataType::Int4:   return 16;
        case ShaderDataType::Bool:   return 1;
        default: return 0;
    }
}

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    uint32_t Size;
    size_t Offset;
    bool Normalized;
    uint32_t ComponentCount;

    BufferElement() = default;
    BufferElement(ShaderDataType type, const std::string& name, bool normalized=false)
        : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {
        switch(type) {
            case ShaderDataType::Float: ComponentCount=1; break;
            case ShaderDataType::Float2: ComponentCount=2; break;
            case ShaderDataType::Float3: ComponentCount=3; break;
            case ShaderDataType::Float4: ComponentCount=4; break;
            case ShaderDataType::Mat3: ComponentCount=9; break;
            case ShaderDataType::Mat4: ComponentCount=16; break;
            default: ComponentCount=1; break;
        }
    }
    uint32_t GetComponentCount() const { return ComponentCount; }
};

class BufferLayout {
public:
    BufferLayout() {}
    BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) { CalculateOffsetsAndStride(); }

    uint32_t GetStride() const { return m_Stride; }
    const std::vector<BufferElement>& GetElements() const { return m_Elements; }

    std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
    std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

private:
    void CalculateOffsetsAndStride() {
        size_t offset=0; m_Stride=0;
        for(auto& e: m_Elements) { e.Offset=offset; offset+=e.Size; m_Stride+=e.Size; }
    }
    std::vector<BufferElement> m_Elements;
    uint32_t m_Stride=0;
};

class VertexBuffer {
public:
    VertexBuffer(uint32_t size);
    VertexBuffer(float* vertices, uint32_t size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, uint32_t size);
    void SetLayout(const BufferLayout& layout) { m_Layout=layout; }
    const BufferLayout& GetLayout() const { return m_Layout; }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID=0;
    BufferLayout m_Layout;
};

class IndexBuffer {
public:
    IndexBuffer(uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    uint32_t GetCount() const { return m_Count; }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID=0;
    uint32_t m_Count=0;
};

} // namespace
