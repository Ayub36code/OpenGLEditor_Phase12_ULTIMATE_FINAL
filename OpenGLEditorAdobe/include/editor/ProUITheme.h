
#pragma once
#include <imgui.h>

namespace Editor {

class ProUITheme {
public:
    static void ApplyAdobeDarkTheme();
    static void ApplyAdobeLightTheme();
    static void DrawToolIcon(const char* icon, bool active);
};

} // namespace
