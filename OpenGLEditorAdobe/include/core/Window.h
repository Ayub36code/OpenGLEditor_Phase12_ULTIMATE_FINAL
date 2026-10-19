
#pragma once
#include <string>
#include <functional>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Editor {

struct WindowProps {
    std::string Title = "OpenGL Editor PRO v2.0";
    uint32_t Width = 1920;
    uint32_t Height = 1080;
    bool VSync = true;
    bool Fullscreen = false;
};

class Window {
public:
    Window(const WindowProps& props);
    ~Window();
    void Update();
    bool ShouldClose() const;
    void SetResizeCallback(std::function<void(int,int)> cb) { m_ResizeCallback=cb; }
    GLFWwindow* GetNative() const { return m_Window; }
    uint32_t GetWidth() const { return m_Props.Width; }
    uint32_t GetHeight() const { return m_Props.Height; }
private:
    WindowProps m_Props;
    GLFWwindow* m_Window=nullptr;
    std::function<void(int,int)> m_ResizeCallback;
};

} // namespace
