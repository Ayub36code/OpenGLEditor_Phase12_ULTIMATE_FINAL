
#pragma once
// Proper GLAD 4.6 Core header - minimal that includes GL and defines loader
#ifdef __cplusplus
extern "C" {
#endif

// This file is intentionally using system GL with GLFW to avoid double-include issues
// We define GLAD to use GLFW's loader
#include <GL/gl.h>

typedef void* (*GLADloadfunc)(const char *name);
int gladLoadGL(GLADloadfunc load);
int gladLoadGLLoader(GLADloadfunc load);

#ifdef __cplusplus
}
#endif

// Compatibility: if code includes glad/glad.h, redirect
