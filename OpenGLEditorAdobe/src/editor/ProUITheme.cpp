
#include "editor/ProUITheme.h"

namespace Editor {

void ProUITheme::ApplyAdobeDarkTheme(){
    ImGuiStyle& style=ImGui::GetStyle();
    style.WindowRounding=0.f; style.FrameRounding=2.f; style.ScrollbarRounding=0.f; style.GrabRounding=2.f;
    style.WindowPadding=ImVec2(6,6); style.FramePadding=ImVec2(6,4); style.ItemSpacing=ImVec2(6,4);
    style.WindowBorderSize=1.f; style.FrameBorderSize=0.f;

    ImVec4* colors=style.Colors;
    colors[ImGuiCol_Text]=ImVec4(0.90f,0.90f,0.90f,1.f);
    colors[ImGuiCol_TextDisabled]=ImVec4(0.50f,0.50f,0.50f,1.f);
    colors[ImGuiCol_WindowBg]=ImVec4(0.13f,0.13f,0.13f,1.f);
    colors[ImGuiCol_ChildBg]=ImVec4(0.15f,0.15f,0.15f,1.f);
    colors[ImGuiCol_PopupBg]=ImVec4(0.18f,0.18f,0.18f,0.98f);
    colors[ImGuiCol_Border]=ImVec4(0.25f,0.25f,0.25f,0.5f);
    colors[ImGuiCol_FrameBg]=ImVec4(0.20f,0.20f,0.20f,1.f);
    colors[ImGuiCol_FrameBgHovered]=ImVec4(0.30f,0.30f,0.30f,1.f);
    colors[ImGuiCol_FrameBgActive]=ImVec4(0.35f,0.35f,0.35f,1.f);
    colors[ImGuiCol_TitleBg]=ImVec4(0.10f,0.10f,0.10f,1.f);
    colors[ImGuiCol_TitleBgActive]=ImVec4(0.15f,0.15f,0.15f,1.f);
    colors[ImGuiCol_MenuBarBg]=ImVec4(0.15f,0.15f,0.15f,1.f);
    colors[ImGuiCol_ScrollbarBg]=ImVec4(0.10f,0.10f,0.10f,1.f);
    colors[ImGuiCol_ScrollbarGrab]=ImVec4(0.35f,0.35f,0.35f,1.f);
    colors[ImGuiCol_CheckMark]=ImVec4(0.50f,0.70f,1.f,1.f);
    colors[ImGuiCol_SliderGrab]=ImVec4(0.50f,0.70f,1.f,1.f);
    colors[ImGuiCol_Button]=ImVec4(0.25f,0.25f,0.25f,1.f);
    colors[ImGuiCol_ButtonHovered]=ImVec4(0.35f,0.35f,0.35f,1.f);
    colors[ImGuiCol_ButtonActive]=ImVec4(0.45f,0.45f,0.45f,1.f);
    colors[ImGuiCol_Header]=ImVec4(0.25f,0.25f,0.25f,1.f);
    colors[ImGuiCol_HeaderHovered]=ImVec4(0.35f,0.35f,0.35f,1.f);
    colors[ImGuiCol_HeaderActive]=ImVec4(0.45f,0.45f,0.45f,1.f);
    colors[ImGuiCol_Tab]=ImVec4(0.18f,0.18f,0.18f,1.f);
    colors[ImGuiCol_TabHovered]=ImVec4(0.35f,0.35f,0.35f,1.f);
    colors[ImGuiCol_TabActive]=ImVec4(0.25f,0.25f,0.25f,1.f);
    colors[ImGuiCol_TabUnfocused]=ImVec4(0.18f,0.18f,0.18f,1.f);
    colors[ImGuiCol_DockingPreview]=ImVec4(0.50f,0.70f,1.f,0.5f);
}

void ProUITheme::ApplyAdobeLightTheme(){ ApplyAdobeDarkTheme(); }

}
