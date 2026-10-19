
#include "utils/Utils.h"
#include <fstream>

namespace Editor::Utils {
    std::string ReadFile(const std::filesystem::path& path) {
        std::ifstream f(path, std::ios::in | std::ios::binary);
        if(!f) return "";
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }
    bool FileExists(const std::filesystem::path& path) { return std::filesystem::exists(path); }
    std::string GetExecutableDir() { return std::filesystem::current_path().string(); }
}
