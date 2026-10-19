
cmake_minimum_required(VERSION 3.20)
project(OpenGLEditor VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.9)

FetchContent_MakeAvailable(glfw glm imgui)

# Glad2
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v2.0.8)
FetchContent_MakeAvailable(glad)
add_subdirectory(${glad_SOURCE_DIR}/cmake ${glad_BINARY_DIR})
glad_add_library(glad REPRODUCIBLE API gl:core=4.6)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE EDITOR_SOURCES
    src/*.cpp
    src/*/*.cpp
)

# stb headers - download if missing
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h)
    file(DOWNLOAD https://raw.githubusercontent.com/nothings/stb/master/stb_image.h ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image_write.h)
    file(DOWNLOAD https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image_write.h)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image_resize2.h)
    file(DOWNLOAD https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize2.h ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image_resize2.h)
endif()

add_executable(OpenGLEditor ${EDITOR_SOURCES} ${IMGUI_SOURCES})

target_include_directories(OpenGLEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glad_SOURCE_DIR}/include
)

target_link_libraries(OpenGLEditor PRIVATE glfw glm glad)

if(UNIX)
    target_link_libraries(OpenGLEditor PRIVATE dl pthread)
endif()

# Copy assets and shaders
add_custom_command(TARGET OpenGLEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:OpenGLEditor>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:OpenGLEditor>/assets
)

