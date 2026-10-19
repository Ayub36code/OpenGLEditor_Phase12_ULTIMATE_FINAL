
#include "graphics/VertexArray.h"
#include <glad/gl.h>

namespace Editor {

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
    switch(type){
        case ShaderDataType::Float: case ShaderDataType::Float2: case ShaderDataType::Float3: case ShaderDataType::Float4:
        case ShaderDataType::Mat3: case ShaderDataType::Mat4: return GL_FLOAT;
        case ShaderDataType::Int: case ShaderDataType::Int2: case ShaderDataType::Int3: case ShaderDataType::Int4: return GL_INT;
        case ShaderDataType::Bool: return GL_BOOL;
        default: return 0;
    }
}

VertexArray::VertexArray() { glCreateVertexArrays(1, &m_RendererID); }
VertexArray::~VertexArray() { glDeleteVertexArrays(1, &m_RendererID); }
void VertexArray::Bind() const { glBindVertexArray(m_RendererID); }
void VertexArray::Unbind() const { glBindVertexArray(0); }

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb) {
    Bind(); vb->Bind();
    const auto& layout = vb->GetLayout();
    uint32_t index=0;
    // handle existing VBs offset
    for(auto& existing: m_VertexBuffers) index += existing->GetLayout().GetElements().size();

    for(const auto& element: layout) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index,
            element.GetComponentCount(),
            ShaderDataTypeToOpenGLBaseType(element.Type),
            element.Normalized ? GL_TRUE : GL_FALSE,
            layout.GetStride(),
            (const void*)element.Offset);
        index++;
    }
    m_VertexBuffers.push_back(vb);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib) {
    Bind(); ib->Bind(); m_IndexBuffer=ib;
}

std::shared_ptr<VertexArray> VertexArray::Create() { return std::make_shared<VertexArray>(); }

}
