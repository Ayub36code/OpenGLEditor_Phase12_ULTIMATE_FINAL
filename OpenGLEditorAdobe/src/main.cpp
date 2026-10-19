
#include "core/Application.h"
#include "core/Log.h"

int main(int argc, char** argv) {
    LOG_INFO("OpenGLEditor starting - Phase 1");
    try {
        Editor::Application app;
        app.Run();
    } catch(const std::exception& e) {
        LOG_ERROR(std::string("Unhandled exception: ") + e.what());
        return -1;
    }
    LOG_INFO("OpenGLEditor shutdown cleanly");
    return 0;
}
