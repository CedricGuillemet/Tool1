#pragma once
#include <imgui_node_editor.h>
#include <string>
#include <vector>
#include <memory>

namespace ed = ax::NodeEditor;

class Node
{
public:
    virtual void Gui() = 0;
    virtual std::string GenerateShader() = 0;
};

class Kick : public Node
{
public:
    void Gui() override;
    std::string GenerateShader() override;
};

class Time : public Node
{
public:
    void Gui() override;
    std::string GenerateShader() override;
};

class Nodal
{
public:
    void OnStart();
    void DoIt();
    void OnStop();

    ed::EditorContext* m_Context = nullptr;
    
    std::vector<std::unique_ptr<Node>> nodes;
    
    std::string GenerateShader();
};



