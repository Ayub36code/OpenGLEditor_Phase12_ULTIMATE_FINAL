
#pragma once
#include <string>

namespace Editor {
struct GLInfo {
    std::string Vendor;
    std::string Renderer;
    std::string Version;
    std::string GLSLVersion;
    int Major = 0, Minor = 0;
};

class OpenGLContext {
public:
    static bool Init();
    static GLInfo GetInfo();
    static void SetDebugOutput(bool enabled);
    static void PrintInfo();
private:
    static void APIENTRY DebugCallback(unsigned int source, unsigned int type,
        unsigned int id, unsigned int severity, int length,
        const char* message, const void* userParam);
};
}
