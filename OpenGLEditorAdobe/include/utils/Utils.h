
#pragma once
#include <string>
#include <filesystem>

namespace Editor::Utils {
    std::string ReadFile(const std::filesystem::path& path);
    bool FileExists(const std::filesystem::path& path);
    std::string GetExecutableDir();
}
