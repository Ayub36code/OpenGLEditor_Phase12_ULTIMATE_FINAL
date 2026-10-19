
#pragma once
#include "graphics/Buffer.h"
#include <memory>
#include <vector>

namespace Editor {

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb);
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib);

    const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
    uint32_t GetRendererID() const { return m_RendererID; }

    static std::shared_ptr<VertexArray> Create();

private:
    uint32_t m_RendererID=0;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

} // namespace
