
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace Editor {

enum class KeyframeType { Position, Scale, Rotation, Opacity, Brightness, Contrast, Saturation, Custom };
enum class InterpolationType { Linear, Bezier, Hold, EaseIn, EaseOut, EaseInOut };

struct Keyframe {
    float Time = 0.f; // seconds
    KeyframeType Type = KeyframeType::Position;
    InterpolationType Interp = InterpolationType::Linear;
    glm::vec4 Value{0}; // x,y for position, x for rotation/opacity etc, or vec4
    glm::vec2 HandleIn{0}, HandleOut{0}; // for Bezier

    // For bezier
    glm::vec4 Evaluate(const Keyframe& next, float t) const;
};

struct Clip {
    std::string Name = "Clip";
    float StartTime = 0.f;
    float Duration = 5.f; // seconds
    std::string LayerName; // which layer this clip affects
    bool Enabled = true;
    bool Loop = false;
    std::vector<Keyframe> Keyframes;

    float GetEndTime() const { return StartTime + Duration; }
    void AddKeyframe(const Keyframe& kf);
    void RemoveKeyframe(int index);
    Keyframe* GetKeyframeAt(float time, KeyframeType type);
    glm::vec4 GetValueAt(float time, KeyframeType type, const glm::vec4& defaultValue) const;
};

struct Track {
    std::string Name;
    std::string LayerName;
    bool Visible = true;
    bool Locked = false;
    bool Muted = false;
    glm::vec4 Color{0.3f,0.6f,1.f,1.f};
    std::vector<Clip> Clips;

    void AddClip(const Clip& clip);
    void RemoveClip(int index);
};

class Timeline {
public:
    Timeline(float duration=10.f, float fps=30.f);
    void Resize(float duration);

    // Playback
    void Play();
    void Pause();
    void Stop();
    void SetCurrentTime(float time);
    void Update(float deltaTime);

    float GetCurrentTime() const { return m_CurrentTime; }
    float GetDuration() const { return m_Duration; }
    float GetFPS() const { return m_FPS; }
    void SetFPS(float fps) { m_FPS = fps; }

    bool IsPlaying() const { return m_IsPlaying; }
    float GetPlayheadNormalized() const { return m_Duration>0 ? m_CurrentTime/m_Duration : 0; }

    // Tracks
    int AddTrack(const std::string& name, const std::string& layerName="");
    void RemoveTrack(int index);
    Track* GetTrack(int index);
    std::vector<Track>& GetTracks() { return m_Tracks; }

    // Clips
    Clip* GetClipAt(int trackIndex, float time);

    // Keyframes evaluation for a layer at current time
    glm::vec2 EvaluatePosition(const std::string& layerName, const glm::vec2& defaultPos) const;
    glm::vec2 EvaluateScale(const std::string& layerName, const glm::vec2& defaultScale) const;
    float EvaluateRotation(const std::string& layerName, float defaultRot) const;
    float EvaluateOpacity(const std::string& layerName, float defaultOpacity) const;

    // Export
    void SetLoop(bool loop) { m_Loop = loop; }
    bool IsLooping() const { return m_Loop; }

    // Callbacks for applying timeline values to layers
    std::function<void(const std::string& layerName, const glm::vec2& pos, const glm::vec2& scale, float rot, float opacity)> OnApplyLayerTransform;

private:
    float m_Duration = 10.f;
    float m_FPS = 30.f;
    float m_CurrentTime = 0.f;
    bool m_IsPlaying = false;
    bool m_Loop = false;
    std::vector<Track> m_Tracks;
};

} // namespace
