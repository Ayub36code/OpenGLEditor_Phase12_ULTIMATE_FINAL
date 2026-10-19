
#pragma once
#include <chrono>

namespace Editor {
class Time {
public:
    static void Update() {
        auto now = std::chrono::high_resolution_clock::now();
        if(s_First) { s_Last = now; s_First = false; }
        s_Delta = std::chrono::duration<float>(now - s_Last).count();
        s_Last = now;
        s_Total += s_Delta;
        s_FrameCount++;
        // FPS averaging
        s_Accum += s_Delta;
        s_AccumFrames++;
        if(s_Accum >= 0.5f) {
            s_FPS = s_AccumFrames / s_Accum;
            s_Accum = 0; s_AccumFrames = 0;
        }
    }
    static float DeltaTime() { return s_Delta; }
    static float TotalTime() { return s_Total; }
    static float FPS() { return s_FPS; }
    static uint64_t FrameCount() { return s_FrameCount; }
private:
    inline static bool s_First = true;
    inline static std::chrono::high_resolution_clock::time_point s_Last;
    inline static float s_Delta = 0.f;
    inline static float s_Total = 0.f;
    inline static float s_FPS = 0.f;
    inline static float s_Accum = 0.f;
    inline static int s_AccumFrames = 0;
    inline static uint64_t s_FrameCount = 0;
};
}
