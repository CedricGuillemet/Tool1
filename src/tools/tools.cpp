#include "tools.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "EditUtils.h"
#include "edit.h"
static Edit edit;

void tools_init()
{
    ImGui::StyleColorsDark();
    //editUtils.Init();
    edit.Init();
}

void tools_do()
{
    ImGuiIO& io = ImGui::GetIO();
    
    int width, height;
    get_display_size(width, height);
    float ratio = 16.f / 9.f;//float(width) / float(height);
    
    matrix view, projection;
    projection.glhPerspectivef2Rad(1., ratio, 0.01f, 1000.f, false);
    vec3 eyePos(3.f, 3.f, 3.f);
    view.lookAtLH(eyePos, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

    edit.ShowGUI();
    edit.ShowContent(view, projection);
}

bool tools_record(bool forceRecording)
{
    return edit.Record(forceRecording);
}

void tools_end()
{
}

