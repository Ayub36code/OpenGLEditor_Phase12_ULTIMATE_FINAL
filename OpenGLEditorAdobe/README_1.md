
# OpenGLEditor - Phase 2 Rendering Abstraction

## Phase 2 Complete:
- Shader class: GLSL compile/link error handling, uniform location cache, #type preprocessing
- Buffer: VertexBuffer, IndexBuffer, BufferLayout with ShaderDataType
- VertexArray: VAO abstraction handling multiple VBs + IB
- Texture2D: stb_image -> GPU, mipmaps, filter/wrap, Resize, Bind(slot)
- Framebuffer: FBO with multiple color attachments, depth, resize, completeness check, pooling
- Camera: OrthographicCamera with zoom, screen->world, ViewProjection
- Renderer: BeginScene/EndScene, DrawQuad, DrawRotatedQuad, DrawCheckerboard
- RenderCommand: Clear, SetViewport, DrawIndexed abstraction
- RenderTarget: FBO + Camera combo for non-destructive pipeline

## Pipeline Now:
Image -> Texture2D -> Framebuffer -> Shader Effects -> Layer Composition -> Viewport -> Export (Phase11)

## Build same as Phase1
cmake -B build && cmake --build build -j && ./build/OpenGLEditor

## What's next Phase3:
Image loading/display, zoom/pan, viewport FBO texture display in ImGui.
