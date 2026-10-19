
#include "editor/History.h"
#include "editor/Viewport.h"
#include "editor/LayerStack.h"
#include "core/Log.h"

namespace Editor {

// HistoryManager

void HistoryManager::Execute(std::unique_ptr<Command> command){
    if(!command) return;
    // Remove redo history
    if(m_CurrentIndex < (int)m_Commands.size()-1){
        m_Commands.erase(m_Commands.begin()+m_CurrentIndex+1, m_Commands.end());
    }

    command->Execute();
    m_Commands.push_back(std::move(command));

    // Limit size
    if(m_Commands.size() > m_MaxSize){
        m_Commands.erase(m_Commands.begin());
    } else {
        m_CurrentIndex++;
    }

    LOG_INFO("History execute, undo count: " + std::to_string(GetUndoCount()));
}

void HistoryManager::Undo(){
    if(!CanUndo()) return;
    m_Commands[m_CurrentIndex]->Undo();
    m_CurrentIndex--;
    LOG_INFO("Undo, remaining: " + std::to_string(GetUndoCount()));
}

void HistoryManager::Redo(){
    if(!CanRedo()) return;
    m_CurrentIndex++;
    m_Commands[m_CurrentIndex]->Redo();
    LOG_INFO("Redo, undo count: " + std::to_string(GetUndoCount()));
}

void HistoryManager::Clear(){
    m_Commands.clear();
    m_CurrentIndex=-1;
}

const std::vector<std::string> HistoryManager::GetHistoryList() const {
    std::vector<std::string> list;
    for(auto& cmd : m_Commands) list.push_back(cmd->GetName());
    return list;
}

// Commands

TransformCommand::TransformCommand(const std::string& layerName, const glm::vec2& oldPos, const glm::vec2& newPos, const glm::vec2& oldScale, const glm::vec2& newScale, float oldRot, float newRot)
    : Command("Transform " + layerName), m_LayerName(layerName), m_OldPos(oldPos), m_NewPos(newPos), m_OldScale(oldScale), m_NewScale(newScale), m_OldRot(oldRot), m_NewRot(newRot) {}

void TransformCommand::Execute(){
    // Find layer and apply new transform - would need viewport access via singleton or manager
    // For Phase9, log only - actual implementation would access LayerStack via Viewport
    LOG_INFO("Execute Transform " + m_LayerName);
}
void TransformCommand::Undo(){ LOG_INFO("Undo Transform " + m_LayerName); }

LayerOpacityCommand::LayerOpacityCommand(const std::string& layerName, float oldOpacity, float newOpacity)
    : Command("Opacity " + layerName), m_LayerName(layerName), m_OldOpacity(oldOpacity), m_NewOpacity(newOpacity) {}
void LayerOpacityCommand::Execute(){ LOG_INFO("Opacity " + m_LayerName + " to " + std::to_string(m_NewOpacity)); }
void LayerOpacityCommand::Undo(){ LOG_INFO("Undo Opacity " + m_LayerName + " to " + std::to_string(m_OldOpacity)); }

AddLayerCommand::AddLayerCommand(const std::string& layerName) : Command("Add Layer " + layerName), m_LayerName(layerName) {}
void AddLayerCommand::Execute(){ LOG_INFO("Add layer " + m_LayerName); }
void AddLayerCommand::Undo(){ LOG_INFO("Undo add layer " + m_LayerName); }

DeleteLayerCommand::DeleteLayerCommand(const std::string& layerName, int index) : Command("Delete Layer " + layerName), m_LayerName(layerName), m_Index(index) {}
void DeleteLayerCommand::Execute(){ LOG_INFO("Delete layer " + m_LayerName); }
void DeleteLayerCommand::Undo(){ LOG_INFO("Undo delete layer " + m_LayerName); }

EffectParamCommand::EffectParamCommand(const std::string& paramName, float oldValue, float newValue, std::function<void(float)> setter)
    : Command("Effect " + paramName), m_ParamName(paramName), m_OldValue(oldValue), m_NewValue(newValue), m_Setter(setter) {}
void EffectParamCommand::Execute(){ if(m_Setter) m_Setter(m_NewValue); }
void EffectParamCommand::Undo(){ if(m_Setter) m_Setter(m_OldValue); }

}
