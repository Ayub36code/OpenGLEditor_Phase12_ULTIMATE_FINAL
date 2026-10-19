
#include "editor/Layer.h"
#include "media/Image.h"
#include "graphics/Texture.h"

namespace Editor {

Layer::Layer(const std::string& name, uint32_t w, uint32_t h, const glm::vec4& color) : Name(name) {
    TextureSpecification spec; spec.Width=w; spec.Height=h; spec.Format=TextureFormat::RGBA8;
    Texture=Texture2D::Create(spec);
    std::vector<unsigned char> data(w*h*4);
    for(uint32_t i=0;i<w*h;i++){ data[i*4+0]=(unsigned char)(color.r*255); data[i*4+1]=(unsigned char)(color.g*255); data[i*4+2]=(unsigned char)(color.b*255); data[i*4+3]=(unsigned char)(color.a*255); }
    Texture->SetData(data.data(), data.size());
}
uint32_t Layer::GetWidth() const { return Texture ? Texture->GetSpecification().Width : 512; }
uint32_t Layer::GetHeight() const { return Texture ? Texture->GetSpecification().Height : 512; }
void Layer::SetImage(std::shared_ptr<Image> img){
    ImageData=img;
    if(img && img->GetTexture()) Texture=img->GetTexture();
}

}
