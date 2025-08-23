#pragma once
#include "msys_vector.h"
#include "msys_types.h"
#include "msys_glutils.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include <string>
#include "IconsFontAwesome5.h"

struct Layout;
float GetDPIFactor();

struct TrackballCamera
{
    enum class ControlMode
    {
        Around,
        Subjective,
    };
    float x{0.f}, y{0.f};
    float radius{1.f};
    vec3 target{0.f, 0.f, 0.f};
    ControlMode controlMode{ControlMode::Subjective };
    bool mbMoving{false};

    bool IsMoving() const
    {
        return mbMoving;
    }

    void Tick()
    {
        ImGuiIO& io = ImGui::GetIO();
        /*if (io.WantCaptureMouse)
        {
            return;
        }*/
        mbMoving = false;
        if (io.MouseDown[0])
        {
            // don't move camera when cursor is over gizmo
            if (!ImGuizmo::IsOver())
            {
                x += io.MouseDelta.x * 0.01f;
                y += io.MouseDelta.y * 0.01f * ((controlMode == ControlMode::Around) ? 1.f : -1.f);
                mbMoving = true;
            }
        }
        if (io.MouseDown[1])
        {
            if (controlMode == ControlMode::Subjective)
            {
                target += GetRight() * io.MouseDelta.x * 0.01f;
                target += GetUp() * io.MouseDelta.y * 0.01f;
            }
        }
        if (controlMode == ControlMode::Around)
        {
            radius *= 1.f + io.MouseWheel * 0.1f;
        }
        else
        {
            // subjective
            target -= GetDirection() * io.MouseWheel * 0.1f;
        }
        mbMoving |= fabsf(io.MouseWheel) > FLT_EPSILON;

        // movements
        if (controlMode == ControlMode::Subjective)
        {
            const float speed = 0.025f * (io.KeyShift ? 4.f : 1.f);
            if (io.KeyCtrl)
            {
                target -= GetUp() * speed;
                mbMoving = true;
            }
            else if (io.KeysDown[ImGuiKey_Space])
            {
                target += GetUp() * speed;
                mbMoving = true;
            }
            else if (io.KeysDown[ImGuiKey_UpArrow])
            {
                target += GetDirection() * speed;
                mbMoving = true;
            }
            else if (io.KeysDown[ImGuiKey_DownArrow])
            {
                target -= GetDirection() * speed;
                mbMoving = true;
            }
            else if (io.KeysDown[ImGuiKey_LeftArrow])
            {
                target += GetRight() * speed;
                mbMoving = true;
            }
            else if (io.KeysDown[ImGuiKey_RightArrow])
            {
                target -= GetRight() * speed;
                mbMoving = true;
            }
        }
    }
    matrix GetWorldToViewMatrix() const
    {
        matrix view;
        view.lookAtLH(GetEyePos(), GetTargetPos(), vec3(0.f, 1.f, 0.f));
        return view;
    }

    vec3 GetEyePos() const
    {
        if (controlMode == ControlMode::Around)
        {
            vec3 eyePos = target + GetDirection() * radius;
            return eyePos;
        }
        else
        {
            // subjective
            return target;
        }
    }
    vec3 GetRight() const
    {
        vec3 direction = GetDirection();
        vec3 right;
        right = Cross(direction, GetUp());
        return right;
    }

    vec3 GetUp() const
    {
        return vec3(0.f, 1.f, 0.f);
    }

    vec3 GetTargetPos() const
    {
        if (controlMode == ControlMode::Around)
        {
            return target;
        }
        else
        {
            // subjective
            vec3 targetPos = target + GetDirection();
            return targetPos;
        }
    }

    vec3 GetDirection() const
    {
        float csy = cosf(y);
        float sny = sinf(y);
        float csx = cosf(x) * csy;
        float snx = sinf(x) * csy;

        vec3 direction{csx, sny, snx};
        return direction;
    }

    void SetCameraWorldMatrix(const matrix& mat)
    {
        target = vec3(mat.m16[12], mat.m16[13], mat.m16[14]);

        vec3 expectedDir(mat.m16[8], mat.m16[9], mat.m16[10]);
        x = atan2f(mat.m16[10], mat.m16[8]);
        y = asinf(mat.m16[9]);
    }

    matrix GetCameraWorldMatrix() const
    {
        matrix res;
        res.lookAtRH(target, target - GetDirection(), vec3(0, 1, 0));
        res.inverse();
        return res;
    }

    void SwitchAround(vec3 pivot)
    {
        const float newRadius = (GetEyePos() - pivot).length();
        x *= -1.f;
        y *= -1.f;
        target = pivot;
        radius = newRadius;

        controlMode = ControlMode::Around;
    }

    void SwitchSubjective()
    {
        target = GetEyePos();
        controlMode = ControlMode::Subjective;
        x *= -1.f;
        y *= -1.f;
    }
};

class EditUtils
{
public:
    bool String(const char* label, std::string& str, int stringId = 0);
    /*
    void Init();
    void DebugSphere(matrix viewProjection, vec3 position, float radius, vec3 color);
    void DebugLink(matrix viewProjection, vec3 posA, vec3 posB, vec3 color);
    void DebugLinkInfluence(matrix viewProjection, vec3 posA, float radiusA, vec3 posB, float radiusB, vec3 color);
    void DebugAABB(matrix viewProjection, vec3 min, vec3 max, vec3 color);
    void DrawCharacterLinks(matrix viewProjection, Character* character, int selectedLink);
    void DrawLinks(matrix viewProjection, Character* character, int selectedNode, const vec3* positions);
    void DrawCharacterInstanceLinks(matrix viewProjection, Character* character, int characterInstance, int selectedNode);
    void NodesList(int characterIndex, int& currentNode);
    void LinksList(int characterIndex, int& currentLink);
    bool EditVec3(vec3& vector, const char* txt);
    void DrawInfluenceCone(const matrix& vp, Character* character, int currentLink);
    bool U8Slider(const char* label, uint8_t& value);
    void DrawGrid(const matrix& vp);
    void DrawWorldGrid(const matrix& vp);
    void DrawWorldDebugRocks(matrix vp, vec3 color);
    void ToggleButton(const char* str_id, bool* v);
    void RenderTrack(const Vec<TrackFrame>& trackFrame, const matrix& mvp, const vec3& color);
    void DrawFrustum(matrix frameViewProjection, matrix viewProjection);
    void DrawTripod(const matrix& vp, const matrix& mat, float scaling);
    Mesh m_wireSphere;
    Mesh m_cube;
    Mesh m_frustum;
    Mesh m_influenceSphere;
    Mesh m_grid;
    Mesh m_gridWorld;
    Mesh m_track;
    Mesh m_tripodMesh;
    unsigned int m_colorShader;
    unsigned int m_vertexColorShader;
    unsigned int m_influenceShader;
    unsigned int m_gridShader;

protected:
    Mesh GenerateGrid();
    Mesh GenerateGridWorld();
     */
};

extern EditUtils editUtils;

struct EditBase
{
    virtual const char* Name() const = 0;
    virtual bool ShowGUI() = 0;
    virtual bool Record(FrameBuffer& contentViewFrameBuffer, ImVec2 contentPosition, bool forceRecording)
    {
        return false;
    };
    virtual void ShowContent(FrameBuffer& contentViewFrameBuffer, unsigned int previousFrame, matrix view, matrix projection, const ImVec2 mouseNormalizedPosition) = 0;
    virtual void ContentBurn(ImVec2 contentSize, ImVec2 cursorPos);

    TrackballCamera m_trackball;

    virtual bool TrackballDisabled() const
    {
        return false;
    }
};

inline vec3 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax)
{
    vec3 tMin = (boxMin - rayOrigin);
    tMin.x /= rayDir.x;
    tMin.y /= rayDir.y;
    tMin.z /= rayDir.z;

    vec3 tMax = (boxMax - rayOrigin);
    tMax.x /= rayDir.x;
    tMax.y /= rayDir.y;
    tMax.z /= rayDir.z;

    vec3 t1(fminf(tMin.x, tMax.x), fminf(tMin.y, tMax.y), fminf(tMin.z, tMax.z));
    vec3 t2(fmaxf(tMin.x, tMax.x), fmaxf(tMin.y, tMax.y), fmaxf(tMin.z, tMax.z));
    float tNear = fmaxf(fmaxf(t1.x, t1.y), t1.z);
    float tFar = fminf(fminf(t2.x, t2.y), t2.z);
    return vec3(tNear, tFar, 0.f);
};

template<typename T>
ImTextureID GetTex(T value)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(value));
}

