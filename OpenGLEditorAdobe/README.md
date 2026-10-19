
# OpenGLEditor — Professional Desktop Editor (C++20 / OpenGL 4.6)

Educational Photoshop / Premiere-like editor built from scratch without game engines.

## Phase 1 - Foundation
- CMake 3.20+ build
- GLFW window + input
- GLAD OpenGL 4.6 Core Profile loader
- Dear ImGui Docking + dark professional theme
- Core logging, window abstraction, application loop
- Basic renderer abstraction
- Resizable dockspace with all panels scaffolded

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/OpenGLEditor
```

### Dependencies (auto-fetched via FetchContent)
- GLFW 3.4
- GLAD2 (gl:core=4.6)
- GLM 1.0.1
- Dear ImGui docking branch
- stb_image / stb_image_write (header-only)

## Architecture
See include/ for clean separation:
- core/ : Window, Application, Log, Time, Input
- graphics/ : OpenGLContext, Renderer, Shader (phase2), Texture, Framebuffer...
- editor/ : EditorUI, Panels
- media/ : (FFmpeg wrapper - phase 9)
- project/ : Project serialization (phase 8)
- utils/ : helpers

## Phases
1. ✅ Foundation (this)
2. Rendering abstraction
3. Image loading/display
4. Zoom/Pan/Crop/Transform
5. Layer system
6. GPU effects
7. Drawing tools
8. Project save/load
9. Video playback (FFmpeg)
10. Timeline
11. Export
12. Optimization

License: MIT - Educational use.
