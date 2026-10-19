
#include "graphics/Shader.h"
#include "core/Log.h"
#include "utils/Utils.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace Editor {

static uint32_t ShaderTypeFromString(const std::string& type) {
    if(type=="vertex") return GL_VERTEX_SHADER;
    if(type=="fragment" || type=="pixel") return GL_FRAGMENT_SHADER;
    if(type=="geometry") return GL_GEOMETRY_SHADER;
    if(type=="compute") return GL_COMPUTE_SHADER;
    return 0;
}

Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    : m_Name(name) {
    CreateProgram(vertexSrc, fragmentSrc);
}

Shader::Shader(const std::string& filepath) {
    std::string source = ReadFile(filepath);
    auto shaderSources = PreProcess(source);
    // Expect vertex + fragment
    if(shaderSources.find("vertex")==shaderSources.end() || shaderSources.find("fragment")==shaderSources.end()) {
        LOG_ERROR("Shader file " + filepath + " missing #type vertex/fragment");
        throw std::runtime_error("Invalid shader file");
    }
    m_Name = std::filesystem::path(filepath).stem().string();
    CreateProgram(shaderSources["vertex"], shaderSources["fragment"]);
}

Shader::Shader(Shader&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Name(std::move(other.m_Name)), m_UniformCache(std::move(other.m_UniformCache)) {
    other.m_RendererID=0;
}
Shader& Shader::operator=(Shader&& other) noexcept {
    if(this!=&other){ if(m_RendererID) glDeleteProgram(m_RendererID);
        m_RendererID=other.m_RendererID; m_Name=std::move(other.m_Name); m_UniformCache=std::move(other.m_UniformCache);
        other.m_RendererID=0; } return *this;
}

Shader::~Shader() { if(m_RendererID) glDeleteProgram(m_RendererID); }

uint32_t Shader::CompileShader(uint32_t type, const std::string& source) {
    uint32_t id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int result; glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result==GL_FALSE) {
        int length; glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::string msg(length, ' '); glGetShaderInfoLog(id, length, &length, msg.data());
        LOG_ERROR("Shader compile failed (" + m_Name + "):\n" + msg);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

void Shader::CreateProgram(const std::string& vs, const std::string& fs) {
    uint32_t program = glCreateProgram();
    uint32_t vsID = CompileShader(GL_VERTEX_SHADER, vs);
    uint32_t fsID = CompileShader(GL_FRAGMENT_SHADER, fs);
    if(!vsID || !fsID) { glDeleteProgram(program); throw std::runtime_error("Shader compilation failed"); }

    glAttachShader(program, vsID);
    glAttachShader(program, fsID);
    glLinkProgram(program);
    int linked; glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(linked==GL_FALSE) {
        int len; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::string msg(len,' '); glGetProgramInfoLog(program, len, &len, msg.data());
        LOG_ERROR("Program link failed (" + m_Name + "):\n" + msg);
        glDeleteProgram(program); glDeleteShader(vsID); glDeleteShader(fsID);
        throw std::runtime_error("Program link failed");
    }
    glDetachShader(program, vsID); glDetachShader(program, fsID);
    glDeleteShader(vsID); glDeleteShader(fsID);
    m_RendererID = program;
    LOG_INFO("Shader compiled: " + m_Name + " ID=" + std::to_string(m_RendererID));
}

void Shader::Bind() const { glUseProgram(m_RendererID); }
void Shader::Unbind() const { glUseProgram(0); }

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_UniformCache.find(name);
    if(it!=m_UniformCache.end()) return it->second;
    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if(loc==-1) {
        // Not error - uniform may be optimized out
        // LOG_WARNING("Uniform not found: " + name + " in shader " + m_Name);
    }
    m_UniformCache[name]=loc;
    return loc;
}

void Shader::SetInt(const std::string& name, int v){ Bind(); glUniform1i(GetUniformLocation(name), v); }
void Shader::SetIntArray(const std::string& name, int* values, uint32_t count){ Bind(); glUniform1iv(GetUniformLocation(name), count, values); }
void Shader::SetFloat(const std::string& name, float v){ Bind(); glUniform1f(GetUniformLocation(name), v); }
void Shader::SetFloat2(const std::string& name, const glm::vec2& v){ Bind(); glUniform2f(GetUniformLocation(name), v.x, v.y); }
void Shader::SetFloat3(const std::string& name, const glm::vec3& v){ Bind(); glUniform3f(GetUniformLocation(name), v.x, v.y, v.z); }
void Shader::SetFloat4(const std::string& name, const glm::vec4& v){ Bind(); glUniform4f(GetUniformLocation(name), v.x, v.y, v.z, v.w); }
void Shader::SetMat3(const std::string& name, const glm::mat3& m){ Bind(); glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::SetMat4(const std::string& name, const glm::mat4& m){ Bind(); glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m)); }

std::string Shader::ReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if(!in) { LOG_ERROR("Could not open shader file: "+path); return ""; }
    std::stringstream ss; ss << in.rdbuf(); return ss.str();
}

std::unordered_map<std::string,std::string> Shader::PreProcess(const std::string& source) {
    std::unordered_map<std::string,std::string> out;
    const char* typeToken="#type";
    size_t pos = source.find(typeToken,0);
    while(pos!=std::string::npos) {
        size_t eol = source.find_first_of("\r\n", pos);
        size_t begin = pos + strlen(typeToken)+1;
        std::string type = source.substr(begin, eol-begin);
        // trim
        type.erase(0,type.find_first_not_of(" \t")); type.erase(type.find_last_not_of(" \t\r\n")+1);
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        pos = source.find(typeToken, nextLinePos);
        std::string shaderCode = (pos==std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos-nextLinePos);
        out[type]=shaderCode;
    }
    return out;
}

std::shared_ptr<Shader> Shader::FromFiles(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    std::string vs = Utils::ReadFile(vertPath);
    std::string fs = Utils::ReadFile(fragPath);
    if(vs.empty()||fs.empty()) { LOG_ERROR("Failed to load shader files"); return nullptr; }
    return std::make_shared<Shader>(name, vs, fs);
}
std::shared_ptr<Shader> Shader::FromSource(const std::string& name, const std::string& vs, const std::string& fs) {
    return std::make_shared<Shader>(name, vs, fs);
}

}
