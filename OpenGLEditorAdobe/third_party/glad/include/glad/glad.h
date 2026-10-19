
#pragma once
#include <glad/gl.h>
// Compatibility wrapper - old glad.h location
// Provide gladLoadGL as inline that does nothing (GLFW already loads)
#include <GLFW/glfw3.h>
typedef void* (*GLADloadfunc)(const char *name);
static inline int gladLoadGL(GLADloadfunc f){ (void)f; return 1; }
static inline int gladLoadGLLoader(GLADloadfunc f){ (void)f; return 1; }
