
#include "graphics/OpenGLContext.h"
#include "core/Log.h"
#include <glad/gl.h>

namespace Editor {

bool OpenGLContext::Init() {
    // Already initialized via GLAD in Window, this validates
    return glGetString(GL_VERSION) != nullptr;
}

GLInfo OpenGLContext::GetInfo() {
    GLInfo info;
    info.Vendor = (const char*)glGetString(GL_VENDOR);
    info.Renderer = (const char*)glGetString(GL_RENDERER);
    info.Version = (const char*)glGetString(GL_VERSION);
    info.GLSLVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    glGetIntegerv(GL_MAJOR_VERSION, &info.Major);
    glGetIntegerv(GL_MINOR_VERSION, &info.Minor);
    return info;
}

void OpenGLContext::PrintInfo() {
    auto i = GetInfo();
    LOG_INFO("=== OpenGL Context ===");
    LOG_INFO("Vendor: " + i.Vendor);
    LOG_INFO("Renderer: " + i.Renderer);
    LOG_INFO("Version: " + i.Version + " (" + std::to_string(i.Major) + "." + std::to_string(i.Minor) + ")");
    LOG_INFO("GLSL: " + i.GLSLVersion);
    if(i.Major < 4 || (i.Major==4 && i.Minor <6)) {
        LOG_WARNING("Requested GL 4.6 but got lower. Some features may be unavailable.");
    }
}

void APIENTRY OpenGLContext::DebugCallback(unsigned int source, unsigned int type,
    unsigned int id, unsigned int severity, int length, const char* message, const void* userParam) {
    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    std::string srcStr;
    switch(source) {
        case GL_DEBUG_SOURCE_API: srcStr="API"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr="SHADER"; break;
        default: srcStr="OTHER"; break;
    }
    std::string msg = "[" + srcStr + "] " + std::string(message, length);
    if(type == GL_DEBUG_TYPE_ERROR) LOG_GL_ERROR(msg);
    else LOG_WARNING(msg);
}

void OpenGLContext::SetDebugOutput(bool enabled) {
    if(!enabled) { glDisable(GL_DEBUG_OUTPUT); return; }
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    LOG_INFO("GL Debug Output enabled");
}

}
