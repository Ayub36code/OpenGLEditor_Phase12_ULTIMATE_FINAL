
#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Editor {

enum class ShaderType { Vertex, Fragment, Geometry, Compute };

class Shader {
public:
    Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    Shader(const std::string& filepath); // loads .vert/.frag pair by base path OR single file with #type
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;
    void Unbind() const;
    uint32_t GetRendererID() const { return m_RendererID; }
    const std::string& GetName() const { return m_Name; }

    // Uniforms - cached locations
    void SetInt(const std::string& name, int value);
    void SetIntArray(const std::string& name, int* values, uint32_t count);
    void SetFloat(const std::string& name, float value);
    void SetFloat2(const std::string& name, const glm::vec2& value);
    void SetFloat3(const std::string& name, const glm::vec3& value);
    void SetFloat4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& mat);
    void SetMat4(const std::string& name, const glm::mat4& mat);

    static std::shared_ptr<Shader> FromFiles(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    static std::shared_ptr<Shader> FromSource(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);

private:
    uint32_t CompileShader(uint32_t type, const std::string& source);
    void CreateProgram(const std::string& vs, const std::string& fs);
    int GetUniformLocation(const std::string& name);
    std::string ReadFile(const std::string& path);
    std::unordered_map<std::string, std::string> PreProcess(const std::string& source); // #type parsing

    uint32_t m_RendererID = 0;
    std::string m_Name;
    mutable std::unordered_map<std::string, int> m_UniformCache;
};

} // namespace
