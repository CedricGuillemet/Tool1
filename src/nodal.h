#pragma once
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

class Nodal
{
public:
    void OnStart();
    void DoIt();
    void OnStop();

    ed::EditorContext* m_Context = nullptr;
};