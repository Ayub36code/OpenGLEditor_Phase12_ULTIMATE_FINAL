
#include "media/Image.h"
#include "core/Log.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"
#include <filesystem>

namespace Editor {

std::shared_ptr<Image> Image::LoadFromFile(const std::string& path){
    int w,h,channels;
    unsigned char* data = stbi_load(path.c_str(), &w,&h,&channels,4);
    if(!data){ LOG_ERROR("Failed to load image: " + path + " - " + stbi_failure_reason()); return nullptr; }
    auto img = CreateFromPixels(w,h,data,4, std::filesystem::path(path).filename().string());
    img->m_Meta.Path = path;
    stbi_image_free(data);
    LOG_INFO("Loaded image: " + path + " " + std::to_string(w)+"x"+std::to_string(h));
    return img;
}

std::shared_ptr<Image> Image::CreateFromPixels(uint32_t w, uint32_t h, const unsigned char* data, int channels, const std::string& name){
    auto img = std::make_shared<Image>();
    img->m_Meta.Width=w; img->m_Meta.Height=h; img->m_Meta.Channels=4; img->m_Meta.Name=name.empty()?"Image":name;
    img->m_Pixels.assign(data, data + w*h*4);
    TextureSpecification spec; spec.Width=w; spec.Height=h; spec.Format=TextureFormat::RGBA8;
    img->m_Texture = Texture2D::Create(spec);
    img->m_Texture->SetData(data, w*h*4);
    return img;
}

void Image::UpdateTexture(){
    if(!m_Texture || m_Pixels.empty()) return;
    TextureSpecification spec; spec.Width=m_Meta.Width; spec.Height=m_Meta.Height; spec.Format=TextureFormat::RGBA8;
    m_Texture = Texture2D::Create(spec);
    m_Texture->SetData(m_Pixels.data(), m_Pixels.size());
}

bool Image::Resize(uint32_t newW, uint32_t newH){
    if(newW==0||newH==0||m_Pixels.empty()) return false;
    std::vector<unsigned char> newPixels(newW*newH*4);
    if(!stbir_resize_uint8_linear(m_Pixels.data(), m_Meta.Width, m_Meta.Height, 0, newPixels.data(), newW, newH, 0, STBIR_RGBA)){ LOG_ERROR("Resize failed"); return false; }
    m_Pixels = std::move(newPixels);
    m_Meta.Width=newW; m_Meta.Height=newH;
    UpdateTexture();
    LOG_INFO("Resized image to " + std::to_string(newW)+"x"+std::to_string(newH));
    return true;
}

void Image::FlipHorizontal(){
    if(m_Pixels.empty()) return;
    uint32_t w=m_Meta.Width, h=m_Meta.Height;
    for(uint32_t y=0;y<h;y++){
        for(uint32_t x=0;x<w/2;x++){
            uint32_t l=(y*w+x)*4, r=(y*w+(w-1-x))*4;
            for(int c=0;c<4;c++) std::swap(m_Pixels[l+c], m_Pixels[r+c]);
        }
    }
    UpdateTexture();
}

void Image::FlipVertical(){
    if(m_Pixels.empty()) return;
    uint32_t w=m_Meta.Width, h=m_Meta.Height;
    std::vector<unsigned char> row(w*4);
    for(uint32_t y=0;y<h/2;y++){
        uint32_t top=(y*w)*4, bottom=((h-1-y)*w)*4;
        memcpy(row.data(), m_Pixels.data()+top, w*4);
        memcpy(m_Pixels.data()+top, m_Pixels.data()+bottom, w*4);
        memcpy(m_Pixels.data()+bottom, row.data(), w*4);
    }
    UpdateTexture();
}

void Image::Rotate90(bool clockwise){
    if(m_Pixels.empty()) return;
    uint32_t w=m_Meta.Width, h=m_Meta.Height;
    uint32_t newW=h, newH=w;
    std::vector<unsigned char> newPixels(newW*newH*4);
    for(uint32_t y=0;y<h;y++){
        for(uint32_t x=0;x<w;x++){
            uint32_t src=(y*w+x)*4;
            uint32_t dst;
            if(clockwise) dst=(x*newW + (newW-1-y))*4;
            else dst=((newH-1-x)*newW + y)*4;
            for(int c=0;c<4;c++) newPixels[dst+c]=m_Pixels[src+c];
        }
    }
    m_Pixels=std::move(newPixels);
    m_Meta.Width=newW; m_Meta.Height=newH;
    UpdateTexture();
}

void ImageManager::Add(std::shared_ptr<Image> img){ if(img){ m_Images.push_back(img); m_Active=img; } }
std::shared_ptr<Image> ImageManager::Import(const std::string& path){ auto img=Image::LoadFromFile(path); if(img) Add(img); return img; }

}
