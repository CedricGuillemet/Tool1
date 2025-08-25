#include "nodal.h"

static int uniqueId = 1;

void Nodal::OnStart()
{
    ed::Config config;
    config.SettingsFile = "Simple.json";
    m_Context = ed::CreateEditor(&config);
    
    nodes.push_back(std::make_unique<Kick>());
    nodes.push_back(std::make_unique<Time>());
}

void Nodal::OnStop()
{
    ed::DestroyEditor(m_Context);
}

void Nodal::DoIt()
{
    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));

    uniqueId = 1;
    // Start drawing nodes.
    for (auto& node : nodes)
    {
        ed::BeginNode(uniqueId++);
        node->Gui();
        ed::EndNode();
    }
    
    ed::End();
    ed::SetCurrentEditor(nullptr);
}

std::string Nodal::GenerateShader()
{
    std::string res;
    res = R"(
#define linearstep(a,b,x) saturate(((x)-(a))/((b)-(a)))
#define saturate(i) clamp(i,0.,1.)
)";
    for (auto& node : nodes)
    {
        res += node->GenerateShader();
    }
    return res;
}


// --------------------------------

void Kick::Gui()
{
    ImGui::Text("Kick");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("Time");
    ed::EndPin();
}

std::string Kick::GenerateShader()
{
    return R"(
             {
               float env=linearstep(0.4,0.15,t);

               // { // hi pass like
               //   env*=exp(-50.*t);
               // }

               dest+=.6*env*tanh(2.*sin(
                 300.*t-20.*exp(-40.*t)
                 -5.*exp(-400.*t)
               ));
             }
             )";
}


void Time::Gui()
{
    ImGui::Text("Time");
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out");
    ed::EndPin();
}
std::string Time::GenerateShader()
{
    return {};
}
