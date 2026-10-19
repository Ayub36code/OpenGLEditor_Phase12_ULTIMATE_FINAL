
#include "editor/ExportManager.h"
#include "graphics/Framebuffer.h"
#include "core/Log.h"
#include <glad/gl.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <filesystem>

namespace Editor {

std::string ExportManager::FormatToExtension(ExportFormat fmt){
    switch(fmt){ case ExportFormat::PNG: return ".png"; case ExportFormat::JPEG: return ".jpg"; case ExportFormat::BMP: return ".bmp"; default: return ".png"; }
}
std::string ExportManager::FormatToString(ExportFormat fmt){
    switch(fmt){ case ExportFormat::PNG: return "PNG"; case ExportFormat::JPEG: return "JPEG"; case ExportFormat::BMP: return "BMP"; default: return "PNG"; }
}

bool ExportManager::ExportTexture(uint32_t textureID, uint32_t width, uint32_t height, const ExportSettings& settings){
    std::vector<unsigned char> pixels(width*height*4);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Flip vertically (OpenGL bottom-left to top-left)
    std::vector<unsigned char> flipped(width*height*4);
    for(uint32_t y=0;y<height;y++) memcpy(flipped.data()+y*width*4, pixels.data()+(height-1-y)*width*4, width*4);

    std::string path = settings.Path;
    if(std::filesystem::path(path).extension().empty()) path += FormatToExtension(settings.Format);

    bool ok=false;
    if(settings.Format==ExportFormat::PNG) ok = stbi_write_png(path.c_str(), width, height, 4, flipped.data(), width*4);
    else if(settings.Format==ExportFormat::JPEG) ok = stbi_write_jpg(path.c_str(), width, height, 4, flipped.data(), settings.Quality);
    else if(settings.Format==ExportFormat::BMP) ok = stbi_write_bmp(path.c_str(), width, height, 4, flipped.data());

    if(ok) LOG_INFO("Exported " + path + " " + std::to_string(width) + "x" + std::to_string(height));
    else LOG_ERROR("Export failed " + path);
    return ok;
}

bool ExportManager::ExportFramebuffer(std::shared_ptr<Framebuffer> fbo, const ExportSettings& settings){
    if(!fbo) return false;
    return ExportTexture(fbo->GetColorAttachmentRendererID(0), fbo->GetSpecification().Width, fbo->GetSpecification().Height, settings);
}

}
