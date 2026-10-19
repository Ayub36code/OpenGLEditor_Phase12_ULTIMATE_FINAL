
#include "editor/Timeline.h"
#include "core/Log.h"
#include <glm/gtx/compatibility.hpp>
#include <algorithm>

namespace Editor {

glm::vec4 Keyframe::Evaluate(const Keyframe& next, float t) const {
    float dt = next.Time - Time;
    if(dt <= 0) return Value;
    float alpha = (t - Time) / dt;
    alpha = glm::clamp(alpha, 0.f, 1.f);

    switch(Interp){
        case InterpolationType::Linear:
            return glm::mix(Value, next.Value, alpha);
        case InterpolationType::Hold:
            return Value;
        case InterpolationType::EaseIn:
            alpha = alpha*alpha;
            return glm::mix(Value, next.Value, alpha);
        case InterpolationType::EaseOut:
            alpha = 1.f - (1.f-alpha)*(1.f-alpha);
            return glm::mix(Value, next.Value, alpha);
        case InterpolationType::EaseInOut:
            alpha = alpha<0.5f ? 2.f*alpha*alpha : 1.f - powf(-2.f*alpha+2.f,2.f)/2.f;
            return glm::mix(Value, next.Value, alpha);
        case InterpolationType::Bezier: {
            // Cubic bezier with handles
            float t2 = alpha;
            // Simplified bezier
            return glm::mix(Value, next.Value, t2);
        }
        default:
            return glm::mix(Value, next.Value, alpha);
    }
}

void Clip::AddKeyframe(const Keyframe& kf){
    Keyframes.push_back(kf);
    std::sort(Keyframes.begin(), Keyframes.end(), [](const Keyframe& a, const Keyframe& b){ return a.Time < b.Time; });
}
void Clip::RemoveKeyframe(int index){ if(index>=0 && index < (int)Keyframes.size()) Keyframes.erase(Keyframes.begin()+index); }

Keyframe* Clip::GetKeyframeAt(float time, KeyframeType type){
    for(auto& kf : Keyframes) if(kf.Type==type && glm::abs(kf.Time - time) < 0.01f) return &kf;
    return nullptr;
}

glm::vec4 Clip::GetValueAt(float time, KeyframeType type, const glm::vec4& defaultValue) const {
    std::vector<const Keyframe*> filtered;
    for(auto& kf : Keyframes) if(kf.Type==type) filtered.push_back(&kf);
    if(filtered.empty()) return defaultValue;
    if(filtered.size()==1) return filtered[0]->Value;
    if(time <= filtered.front()->Time) return filtered.front()->Value;
    if(time >= filtered.back()->Time) return filtered.back()->Value;
    for(size_t i=0;i<filtered.size()-1;i++){
        if(time >= filtered[i]->Time && time <= filtered[i+1]->Time){
            return filtered[i]->Evaluate(*filtered[i+1], time);
        }
    }
    return defaultValue;
}

void Track::AddClip(const Clip& clip){ Clips.push_back(clip); }
void Track::RemoveClip(int index){ if(index>=0 && index < (int)Clips.size()) Clips.erase(Clips.begin()+index); }

Timeline::Timeline(float duration, float fps) : m_Duration(duration), m_FPS(fps) {}

void Timeline::Resize(float duration){ m_Duration=duration; m_CurrentTime=glm::clamp(m_CurrentTime, 0.f, m_Duration); }

void Timeline::Play(){ m_IsPlaying=true; LOG_INFO("Timeline Play at " + std::to_string(m_CurrentTime)); }
void Timeline::Pause(){ m_IsPlaying=false; LOG_INFO("Timeline Pause at " + std::to_string(m_CurrentTime)); }
void Timeline::Stop(){ m_IsPlaying=false; m_CurrentTime=0.f; }

void Timeline::SetCurrentTime(float time){
    m_CurrentTime = glm::clamp(time, 0.f, m_Duration);
    // Apply transforms to layers via callback
    if(OnApplyLayerTransform){
        // Group by layer name
        std::map<std::string, bool> processed;
        for(auto& track : m_Tracks){
            if(track.LayerName.empty() || processed[track.LayerName]) continue;
            glm::vec2 pos = EvaluatePosition(track.LayerName, {0,0});
            glm::vec2 scale = EvaluateScale(track.LayerName, {1,1});
            float rot = EvaluateRotation(track.LayerName, 0.f);
            float opacity = EvaluateOpacity(track.LayerName, 1.f);
            OnApplyLayerTransform(track.LayerName, pos, scale, rot, opacity);
            processed[track.LayerName]=true;
        }
    }
}

void Timeline::Update(float deltaTime){
    if(!m_IsPlaying) return;
    m_CurrentTime += deltaTime;
    if(m_CurrentTime >= m_Duration){
        if(m_Loop) m_CurrentTime = fmodf(m_CurrentTime, m_Duration);
        else { m_CurrentTime = m_Duration; m_IsPlaying=false; LOG_INFO("Timeline finished"); }
    }
    SetCurrentTime(m_CurrentTime);
}

int Timeline::AddTrack(const std::string& name, const std::string& layerName){
    Track track; track.Name=name; track.LayerName=layerName.empty()?name:layerName;
    track.Color = glm::vec4((rand()%100)/100.f*0.5f+0.3f, (rand()%100)/100.f*0.5f+0.3f, (rand()%100)/100.f*0.5f+0.5f, 1.f);
    m_Tracks.push_back(track);
    LOG_INFO("Track added: " + name + " for layer " + track.LayerName);
    return (int)m_Tracks.size()-1;
}

void Timeline::RemoveTrack(int index){ if(index>=0 && index < (int)m_Tracks.size()) m_Tracks.erase(m_Tracks.begin()+index); }
Track* Timeline::GetTrack(int index){ if(index>=0 && index < (int)m_Tracks.size()) return &m_Tracks[index]; return nullptr; }

Clip* Timeline::GetClipAt(int trackIndex, float time){
    if(trackIndex<0 || trackIndex >= (int)m_Tracks.size()) return nullptr;
    for(auto& clip : m_Tracks[trackIndex].Clips){
        if(time >= clip.StartTime && time <= clip.GetEndTime()) return &clip;
    }
    return nullptr;
}

glm::vec2 Timeline::EvaluatePosition(const std::string& layerName, const glm::vec2& defaultPos) const {
    glm::vec4 result(defaultPos,0,0);
    bool found=false;
    for(auto& track : m_Tracks){
        if(track.LayerName==layerName){
            for(auto& clip : track.Clips){
                if(m_CurrentTime >= clip.StartTime && m_CurrentTime <= clip.GetEndTime()){
                    float localTime = m_CurrentTime - clip.StartTime;
                    glm::vec4 val = clip.GetValueAt(localTime, KeyframeType::Position, result);
                    if(!found || clip.StartTime <= m_CurrentTime) { result=val; found=true; }
                }
            }
        }
    }
    return found ? glm::vec2(result.x, result.y) : defaultPos;
}

glm::vec2 Timeline::EvaluateScale(const std::string& layerName, const glm::vec2& defaultScale) const {
    glm::vec4 result(defaultScale,0,0);
    bool found=false;
    for(auto& track : m_Tracks){
        if(track.LayerName==layerName){
            for(auto& clip : track.Clips){
                if(m_CurrentTime >= clip.StartTime && m_CurrentTime <= clip.GetEndTime()){
                    float localTime = m_CurrentTime - clip.StartTime;
                    glm::vec4 val = clip.GetValueAt(localTime, KeyframeType::Scale, result);
                    if(!found){ result=val; found=true; }
                }
            }
        }
    }
    return found ? glm::vec2(result.x, result.y) : defaultScale;
}

float Timeline::EvaluateRotation(const std::string& layerName, float defaultRot) const {
    glm::vec4 result(defaultRot,0,0,0);
    bool found=false;
    for(auto& track : m_Tracks){
        if(track.LayerName==layerName){
            for(auto& clip : track.Clips){
                if(m_CurrentTime >= clip.StartTime && m_CurrentTime <= clip.GetEndTime()){
                    float localTime = m_CurrentTime - clip.StartTime;
                    glm::vec4 val = clip.GetValueAt(localTime, KeyframeType::Rotation, result);
                    found=true; result=val;
                }
            }
        }
    }
    return found ? result.x : defaultRot;
}

float Timeline::EvaluateOpacity(const std::string& layerName, float defaultOpacity) const {
    glm::vec4 result(defaultOpacity,0,0,0);
    bool found=false;
    for(auto& track : m_Tracks){
        if(track.LayerName==layerName){
            for(auto& clip : track.Clips){
                if(m_CurrentTime >= clip.StartTime && m_CurrentTime <= clip.GetEndTime()){
                    float localTime = m_CurrentTime - clip.StartTime;
                    glm::vec4 val = clip.GetValueAt(localTime, KeyframeType::Opacity, result);
                    found=true; result=val;
                }
            }
        }
    }
    return found ? result.x : defaultOpacity;
}

}
