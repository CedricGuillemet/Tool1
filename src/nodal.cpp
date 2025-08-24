#include "nodal.h"



void Nodal::OnStart()
{
    ed::Config config;
    config.SettingsFile = "Simple.json";
    m_Context = ed::CreateEditor(&config);
}

void Nodal::OnStop()
{
    ed::DestroyEditor(m_Context);
}

void Nodal::DoIt()
{
    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));

    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ed::EndNode();

    ed::End();
    ed::SetCurrentEditor(nullptr);
}