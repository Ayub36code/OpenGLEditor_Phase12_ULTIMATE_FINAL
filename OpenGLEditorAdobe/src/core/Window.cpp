
#include "core/Window.h"
#include "core/Log.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Editor {

Window::Window(const WindowProps& props) : m_Props(props){
    if(!glfwInit()){ LOG_ERROR("GLFW init failed"); return; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), nullptr, nullptr);
    if(!m_Window){ LOG_ERROR("Window creation failed"); glfwTerminate(); return; }
    glfwMakeContextCurrent(m_Window);
    int version = gladLoadGL(glfwGetProcAddress);
    if(version==0){ LOG_ERROR("Failed to load GLAD"); return; }
    LOG_INFO("GLAD loaded OpenGL 4.6");
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* win, int w, int h){
        auto* self = (Window*)glfwGetWindowUserPointer(win);
        self->m_Props.Width=w; self->m_Props.Height=h;
        glViewport(0,0,w,h);
        if(self->m_ResizeCallback) self->m_ResizeCallback(w,h);
    });
    if(props.VSync) glfwSwapInterval(1); else glfwSwapInterval(0);
}

Window::~Window(){ if(m_Window){ glfwDestroyWindow(m_Window); } glfwTerminate(); }
void Window::Update(){ glfwPollEvents(); glfwSwapBuffers(m_Window); }
bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

}
