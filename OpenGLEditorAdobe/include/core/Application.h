
#pragma once
#include <memory>
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "editor/EditorUI.h"

namespace Editor {

class Application {
public:
    Application();
    ~Application();
    void Run();
    static Application& Get() { return *s_Instance; }
    Window& GetWindow() { return *m_Window; }
private:
    void Init();
    void Shutdown();
    bool m_Running = true;
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<EditorUI> m_EditorUI;
    static Application* s_Instance;
};

} // namespace
