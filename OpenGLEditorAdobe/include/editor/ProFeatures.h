
#pragma once
// Professional Adobe-like features header - defines all pro capabilities
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Editor {

// Professional color spaces
enum class ColorSpace { RGB, CMYK, LAB, Grayscale, HSB, XYZ };
enum class BitDepth { B8=8, B16=16, B32=32 };

// Adjustment layer types (non-destructive)
enum class AdjustmentType {
    Levels, Curves, HueSaturation, ColorBalance, BrightnessContrast,
    Exposure, Vibrance, BlackWhite, PhotoFilter, ChannelMixer,
    ColorLookup, Invert, Posterize, Threshold, GradientMap, SelectiveColor
};

// Layer styles (like Photoshop Layer Effects)
struct DropShadowStyle { bool Enabled=false; glm::vec2 Offset{5,5}; float Blur=10.f; float Spread=0.f; glm::vec4 Color{0,0,0,0.75f}; float Opacity=0.75f; float Angle=120.f; bool UseGlobalLight=true; };
struct InnerShadowStyle { bool Enabled=false; glm::vec2 Offset{5,5}; float Blur=10.f; float Choke=0.f; glm::vec4 Color{0,0,0,0.5f}; };
struct OuterGlowStyle { bool Enabled=false; float Size=10.f; float Spread=0.f; glm::vec4 Color{1,0.8f,0,1.f}; float Opacity=0.8f; };
struct InnerGlowStyle { bool Enabled=false; float Size=10.f; float Choke=0.f; glm::vec4 Color{1,1,1,0.8f}; };
struct BevelEmbossStyle { bool Enabled=false; enum Style {OuterBevel, InnerBevel, Emboss, PillowEmboss, StrokeEmboss} Style=InnerBevel; enum Technique {Smooth, ChiselHard, ChiselSoft} Technique=Smooth; float Depth=100.f; float Size=5.f; float Soften=0.f; float Angle=120.f; float Altitude=30.f; };
struct StrokeStyle { bool Enabled=false; float Size=3.f; glm::vec4 Color{1,0,0,1}; enum Position {Outside, Inside, Center} Position=Outside; };

struct LayerStyle {
    DropShadowStyle DropShadow;
    InnerShadowStyle InnerShadow;
    OuterGlowStyle OuterGlow;
    InnerGlowStyle InnerGlow;
    BevelEmbossStyle BevelEmboss;
    StrokeStyle Stroke;
    bool HasAnyEnabled() const { return DropShadow.Enabled || InnerShadow.Enabled || OuterGlow.Enabled || InnerGlow.Enabled || BevelEmboss.Enabled || Stroke.Enabled; }
};

// Smart object (non-destructive embedded file)
struct SmartObject {
    std::string SourcePath;
    std::string EmbeddedData; // base64 or path
    glm::ivec2 OriginalSize{0,0};
    bool IsLinked = false;
    void Update();
};

// Vector path - pen tool
struct BezierPoint {
    glm::vec2 Position;
    glm::vec2 HandleIn;
    glm::vec2 HandleOut;
    bool HasHandleIn=false, HasHandleOut=false;
    bool Corner=false; // sharp corner
};

struct VectorPath {
    std::string Name;
    std::vector<BezierPoint> Points;
    bool Closed=false;
    bool IsClippingPath=false;
    glm::vec4 StrokeColor{0,0,0,1};
    float StrokeWidth=2.f;
    glm::vec4 FillColor{1,0,0,0}; // transparent by default
    bool Fill=true;
    bool Stroke=false;

    void AddPoint(const glm::vec2& pos);
    glm::vec2 Evaluate(float t) const; // bezier evaluation
    std::vector<glm::vec2> Tessellate(int segmentsPerCurve=20) const;
};

// Advanced brush engine
struct BrushDynamics {
    bool PressureSize=true;
    bool PressureOpacity=true;
    bool TiltAngle=false;
    float SizeJitter=0.f;
    float OpacityJitter=0.f;
    float AngleJitter=0.f;
    float Scatter=0.f; // 0..1000%
    int Count=1; // dual brush count
    bool ColorDynamics=false;
    float HueJitter=0.f, SatJitter=0.f, BrightJitter=0.f;
};

struct ProfessionalBrush {
    std::string Name;
    std::string TipTexturePath;
    float Size=50.f;
    float Hardness=80.f;
    float Spacing=25.f; // %
    float Angle=0.f;
    float Roundness=100.f;
    bool FlipX=false, FlipY=false;
    BrushDynamics Dynamics;
    uint32_t TipTextureID=0;

    // Dual brush
    bool DualBrush=false;
    ProfessionalBrush* SecondaryBrush=nullptr;
    int BlendMode=0; // multiply etc
};

// Color management
struct ColorProfile {
    std::string Name;
    std::string Path; // ICC profile path
    ColorSpace Space=ColorSpace::RGB;
    std::string Description;
};

// Non-destructive adjustment layer
class AdjustmentLayer {
public:
    AdjustmentLayer(AdjustmentType type);
    virtual ~AdjustmentLayer() = default;
    AdjustmentType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }
    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool e) { m_Enabled=e; }

    virtual void Apply(uint32_t inputTexture, std::shared_ptr<class Framebuffer> target, const glm::vec4& params) = 0;
    virtual void DrawUI() = 0;

protected:
    AdjustmentType m_Type;
    std::string m_Name;
    bool m_Enabled=true;
};

// Specific adjustments
class LevelsAdjustment : public AdjustmentLayer {
public:
    LevelsAdjustment();
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> target, const glm::vec4& params) override;
    void DrawUI() override;
    float Shadows=0.f, Midtones=1.f, Highlights=255.f;
    float OutputShadows=0.f, OutputHighlights=255.f;
    std::shared_ptr<class Shader> m_Shader;
};

class CurvesAdjustment : public AdjustmentLayer {
public:
    CurvesAdjustment();
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> target, const glm::vec4& params) override;
    void DrawUI() override;
    std::vector<glm::vec2> Points{{0,0},{0.25f,0.25f},{0.5f,0.5f},{0.75f,0.75f},{1,1}}; // curve points
    std::shared_ptr<class Shader> m_Shader;
};

class HueSatAdjustment : public AdjustmentLayer {
public:
    HueSatAdjustment();
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> target, const glm::vec4& params) override;
    void DrawUI() override;
    float Hue=0.f, Saturation=0.f, Lightness=0.f;
    bool Colorize=false;
    glm::vec3 ColorizeColor{0,0,0};
    std::shared_ptr<class Shader> m_Shader;
};

// Workspace
enum class WorkspaceType { Essentials, Photography, Painting, Graphic, Motion, Custom };
struct Workspace {
    WorkspaceType Type=WorkspaceType::Essentials;
    std::string Name="Essentials";
    std::vector<std::string> VisiblePanels;
    std::string LayoutData; // imgui ini
};

} // namespace
