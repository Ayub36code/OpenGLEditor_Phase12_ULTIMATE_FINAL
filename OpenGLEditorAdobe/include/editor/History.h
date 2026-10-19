
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace Editor {

// Command pattern for undo/redo

class Command {
public:
    Command(const std::string& name) : m_Name(name) {}
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() { Execute(); }
    const std::string& GetName() const { return m_Name; }
    virtual bool CanMergeWith(const Command* other) const { return false; }
    virtual void Merge(std::unique_ptr<Command> other) {}

protected:
    std::string m_Name;
};

class HistoryManager {
public:
    static HistoryManager& Get() { static HistoryManager instance; return instance; }

    void Execute(std::unique_ptr<Command> command);
    bool CanUndo() const { return m_CurrentIndex >= 0; }
    bool CanRedo() const { return m_CurrentIndex < (int)m_Commands.size()-1; }

    void Undo();
    void Redo();
    void Clear();

    int GetUndoCount() const { return m_CurrentIndex+1; }
    int GetRedoCount() const { return (int)m_Commands.size() - m_CurrentIndex -1; }

    const std::vector<std::string> GetHistoryList() const;

    void SetMaxSize(size_t maxSize) { m_MaxSize = maxSize; }

private:
    HistoryManager() = default;
    std::vector<std::unique_ptr<Command>> m_Commands;
    int m_CurrentIndex = -1;
    size_t m_MaxSize = 100;
};

// Specific commands

class TransformCommand : public Command {
public:
    TransformCommand(const std::string& layerName, const glm::vec2& oldPos, const glm::vec2& newPos, const glm::vec2& oldScale, const glm::vec2& newScale, float oldRot, float newRot);
    void Execute() override;
    void Undo() override;
private:
    std::string m_LayerName;
    glm::vec2 m_OldPos, m_NewPos;
    glm::vec2 m_OldScale, m_NewScale;
    float m_OldRot, m_NewRot;
};

class LayerOpacityCommand : public Command {
public:
    LayerOpacityCommand(const std::string& layerName, float oldOpacity, float newOpacity);
    void Execute() override;
    void Undo() override;
private:
    std::string m_LayerName;
    float m_OldOpacity, m_NewOpacity;
};

class AddLayerCommand : public Command {
public:
    AddLayerCommand(const std::string& layerName);
    void Execute() override;
    void Undo() override;
private:
    std::string m_LayerName;
};

class DeleteLayerCommand : public Command {
public:
    DeleteLayerCommand(const std::string& layerName, int index);
    void Execute() override;
    void Undo() override;
private:
    std::string m_LayerName;
    int m_Index;
};

class EffectParamCommand : public Command {
public:
    EffectParamCommand(const std::string& paramName, float oldValue, float newValue, std::function<void(float)> setter);
    void Execute() override;
    void Undo() override;
private:
    std::string m_ParamName;
    float m_OldValue, m_NewValue;
    std::function<void(float)> m_Setter;
};

} // namespace
