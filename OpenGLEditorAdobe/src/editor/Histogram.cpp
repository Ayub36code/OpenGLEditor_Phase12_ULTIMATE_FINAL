
#include "editor/Histogram.h"
#include <glad/gl.h>
#include <cmath>

namespace Editor {

void Histogram::CalculateFromPixels(const std::vector<unsigned char>& pixels, int channels){
    m_Data.Red.assign(256,0); m_Data.Green.assign(256,0); m_Data.Blue.assign(256,0); m_Data.Luminance.assign(256,0);
    if(pixels.empty() || channels<3){ m_Valid=false; return; }
    int pixelCount = pixels.size() / channels;
    long long sumR=0,sumG=0,sumB=0;
    for(int i=0;i<pixelCount;i++){
        int idx=i*channels;
        unsigned char r=pixels[idx], g=channels>1?pixels[idx+1]:r, b=channels>2?pixels[idx+2]:r;
        m_Data.Red[r]++; m_Data.Green[g]++; m_Data.Blue[b]++;
        int lum = (int)(0.299*r + 0.587*g + 0.114*b);
        m_Data.Luminance[glm::clamp(lum,0,255)]++;
        sumR+=r; sumG+=g; sumB+=b;
    }
    m_AvgColor = glm::vec3(sumR/(float)pixelCount/255.f, sumG/(float)pixelCount/255.f, sumB/(float)pixelCount/255.f);
    m_Data.MaxCount=1;
    for(int i=0;i<256;i++){ m_Data.MaxCount=glm::max(m_Data.MaxCount, m_Data.Red[i]); m_Data.MaxCount=glm::max(m_Data.MaxCount, m_Data.Green[i]); m_Data.MaxCount=glm::max(m_Data.MaxCount, m_Data.Blue[i]); }
    m_Valid=true;
}

void Histogram::CalculateFromTexture(uint32_t textureID, uint32_t width, uint32_t height){
    // Read back texture for histogram - for Phase8 we do CPU readback for stats
    // In production, use compute shader
    std::vector<unsigned char> pixels(width*height*4);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    CalculateFromPixels(pixels, 4);
}

float Histogram::GetMean() const {
    if(!m_Valid) return 0;
    long long sum=0, count=0;
    for(int i=0;i<256;i++){ sum+= (long long)m_Data.Luminance[i]*i; count+=m_Data.Luminance[i]; }
    return count>0 ? (float)sum/count : 0;
}

float Histogram::GetStdDev() const {
    float mean=GetMean(); double var=0; long long count=0;
    for(int i=0;i<256;i++){ var+= m_Data.Luminance[i] * (i-mean)*(i-mean); count+=m_Data.Luminance[i]; }
    return count>0 ? sqrtf((float)(var/count)) : 0;
}

}
