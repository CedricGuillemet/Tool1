#include "EditUtils.h"
#include "msys_glext.h"
#include "imgui_internal.h"
#include <string>

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct ColoredVertex
{
    float x, y, z;
    uint32_t color;
};

EditUtils editUtils;

#if 0
void EditUtils::Init()
{
    const char* fragment = R"(
                             uniform vec3 color;
                             void main()
                             {
                                 gl_FragColor = vec4(color, 1.);
                             }
                             )";
    
    const char* vertexColorFragment = R"(
                             void main()
                             {
                                 gl_FragColor = vColor;
                             }
                             )";

    const char* vertex = R"(
                           void main()
                           {
                                gl_Position = mvpMtx * vec4(Position.xyz,1);
                                vColor = Color;
                           })";

    const char* influenceVertex = R"(
                           uniform mat4 modelA;
                           uniform mat4 modelB;
                            uniform mat4 vp;
                           void main()
                           {
                                float t = step(Position.z, 0.);
                            mat4 mat = mat4(
                                mix(modelA[0], modelB[0], t),
                               mix(modelA[1], modelB[1], t),
                               mix(modelA[2], modelB[2], t),
                               mix(modelA[3], modelB[3], t));
                           vec4 p0 = mat * vec4(Position.xyz,1);
                                gl_Position = vp * p0;
                           })";

    m_colorShader = compile_shader(vertex, fragment, "colorShader", false);
    m_vertexColorShader = compile_shader(vertex, vertexColorFragment, "vertexColorShader", false);
    m_wireSphere = build_wire_sphere_mesh();
    m_cube = build_unit_cube_mesh();
    m_frustum = build_frustum_mesh();
    m_influenceSphere = build_wire_sphere_mesh(45);//build_unit_sphere_mesh();
    m_influenceShader = compile_shader(influenceVertex, fragment, "InfluenceColor", false);

    static const ColoredVertex tripodVertices[6] = {
        {0.f, 0.f, 0.f, 0xFF0000FF},
        {1.f, 0.f, 0.f, 0xFF0000FF},
        
        {0.f, 0.f, 0.f, 0xFF00FF00},
        {0.f, 1.f, 0.f, 0xFF00FF00},
        
        {0.f, 0.f, 0.f, 0xFFFF0000},
        {0.f, 0.f, 1.f, 0xFFFF0000},
    };
    static const uint16_t tripodIndices[6] = {0, 1, 2, 3, 4, 5};
    m_tripodMesh = build_mesh(tripodVertices, ATTRIB_POSITION | ATTRIB_COLOR, 6, tripodIndices, 6, false, GL_LINES);
    
    static const char* gridVertex = R"(
        void main()
        {
            vTexCoord = TexCoord0;
            vNormal = Normal;
            gl_Position = mvpMtx * vec4(Position.xyz,1);
        }
    )";
    static const char* gridFragment = R"(
        void main()
        {
            vec3 lightDir = vec3(0.2, -0.6, 0.5);
            float illum = dot(normalize(-lightDir), normalize(vNormal)) * 0.5 + 0.8;
            
            float g = grid(vTexCoord.xy * 8., 0.05) * 0.4 + 0.6;
            g = min(g, grid(vTexCoord.xy * 40. + 0.5, 0.1) * 0.2 + 0.8);
            g *= 0.3;
            g -= 0.05;
            
            g *= illum;
        
            gl_FragColor = vec4(g, g, g, 1.0);
        }
    )";
    m_gridShader = compile_shader(gridVertex, gridFragment, "grid", false);
    m_grid = GenerateGrid();
    m_gridWorld = GenerateGridWorld();
}

void EditUtils::DrawGrid(const matrix& vp)
{
    bind_shader(m_gridShader);
    bind_matrix(shd_mvpMtx, vp.m16);
    draw(m_grid);
}

void EditUtils::DrawWorldGrid(const matrix& vp)
{
    bind_shader(m_gridShader);
    bind_matrix(shd_mvpMtx, vp.m16);
    draw(m_gridWorld);
}

void EditUtils::DrawTripod(const matrix& vp, const matrix& mat, float scaling)
{
    glDisable(GL_DEPTH_TEST);
    bind_shader(m_vertexColorShader);
    matrix scale;
    scale.scale(scaling);
    matrix model = scale * mat;
    matrix mvp = model * vp;
    bind_matrix(shd_modelMtx, model.m16);
    bind_matrix(shd_vpMtx, vp.m16);
    bind_matrix(shd_mvpMtx, mvp.m16);
    draw(m_tripodMesh);
    glEnable(GL_DEPTH_TEST);
}

Mesh EditUtils::GenerateGrid()
{
    Vec<uint16_t> indices;
    Vec<Vertex> vertices;

    const int tesselationSegments = 16;
    const float halfWidth = 20.f;
    const float halfDepth = 20.f;
    const float quarterCircleLength = 0.5f * PI;
    const float offsetZ = 2.f;
    const uint32_t indexCount = tesselationSegments * 6;
    const uint32_t vertexCount = tesselationSegments * 2 + 2;
    indices.reserve(indexCount);
    vertices.reserve(vertexCount);

    for (uint32_t i = 0; i < tesselationSegments; i++)
    {
        indices.push_back(i * 2 + 0);
        indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 1);

        indices.push_back(i * 2 + 1);
        indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 3);
    }

    for (uint32_t i = 0; i < vertexCount / 2; i++)
    {
        const float t = float(i) / float(tesselationSegments);
        const float ng = t * PI * 0.5f + PI * 1.5f;
        vec3 n{0.f, -sinf(ng), -cosf(ng)};
        vertices.push_back({-halfWidth, sinf(ng) + 1.f, cosf(ng) + offsetZ, n.x, n.y, n.z, -halfWidth, t * quarterCircleLength});
        vertices.push_back({halfWidth, sinf(ng) + 1.f, cosf(ng) + offsetZ, n.x, n.y, n.z, halfWidth, t * quarterCircleLength});
    }
    // ground
    static const int planIndices[] = {0, 2, 1, 1, 2, 3};
    for (auto index : planIndices)
    {
        indices.push_back(static_cast<uint16_t>(index + vertices.size()));
    }
    vertices.push_back({-halfWidth, 0.f, 0.0f + offsetZ, 0.f, 1.f, 0.f, -halfWidth, 0.0f});
    vertices.push_back({halfWidth, 0.f, 0.0f + offsetZ, 0.f, 1.f, 0.f, halfWidth, 0.0f});
    vertices.push_back({-halfWidth, 0.f, -halfDepth + offsetZ, 0.f, 1.f, 0.f, -halfWidth, -halfDepth});
    vertices.push_back({halfWidth, 0.f, -halfDepth + offsetZ, 0.f, 1.f, 0.f, halfWidth, -halfDepth});

    for (auto index : planIndices)
    {
        indices.push_back(static_cast<uint16_t>(index + vertices.size()));
    }

    vertices.push_back({-halfWidth, 1.f, 1.0f + offsetZ, 0.f, 0.f, -1.f, -halfWidth, quarterCircleLength});
    vertices.push_back({halfWidth, 1.f, 1.0f + offsetZ, 0.f, 0.f, -1.f, halfWidth, quarterCircleLength});
    vertices.push_back({-halfWidth, 1.f + halfDepth, 1.0f + offsetZ, 0.f, 0.f, -1.f, -halfWidth, quarterCircleLength + halfDepth});
    vertices.push_back({halfWidth, 1.f + halfDepth, 1.0f + offsetZ, 0.f, 0.f, -1.f, halfWidth, quarterCircleLength + halfDepth});

    // rotate 180 around Y axis
    for (auto& vt : vertices)
    {
        vt.x = -vt.x;
        vt.z = -vt.z;
    }

    return build_mesh(vertices.data(), ATTRIB_TEXCOORD0 | ATTRIB_POSITION | ATTRIB_NORMAL, static_cast<unsigned int>(vertices.size()), indices.data(), static_cast<unsigned int>(indices.size()), false);
}

Mesh EditUtils::GenerateGridWorld()
{
    Vec<uint16_t> indices;
    Vec<Vertex> vertices;

    const float halfWidth = 64.f;
    const float offsetZ = 0.f;
    const float halfDepth = halfWidth;

    for (uint32_t i = 0; i < 1; i++)
    {
        indices.push_back(i * 2 + 0);
        indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 1);

        indices.push_back(i * 2 + 1);
        indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 3);
    }

    vertices.push_back({-halfWidth, 0.f, halfDepth + offsetZ, 0.f, 1.f, 0.f, -halfWidth, halfDepth});
    vertices.push_back({halfWidth, 0.f, halfDepth + offsetZ, 0.f, 1.f, 0.f, halfWidth, halfDepth});
    vertices.push_back({-halfWidth, 0.f, -halfDepth + offsetZ, 0.f, 1.f, 0.f, -halfWidth, -halfDepth});
    vertices.push_back({halfWidth, 0.f, -halfDepth + offsetZ, 0.f, 1.f, 0.f, halfWidth, -halfDepth});
    return build_mesh(vertices.data(), ATTRIB_TEXCOORD0 | ATTRIB_POSITION | ATTRIB_NORMAL, static_cast<unsigned int>(vertices.size()), indices.data(), static_cast<unsigned int>(indices.size()), false);
}


void EditUtils::DebugAABB(matrix viewProjection, vec3 min, vec3 max, vec3 color)
{
    matrix model;
    vec3 sz = (max - min) * 0.5f;
    vec3 center = (max + min) * 0.5f;
    model.scale(sz.x, sz.y, sz.z);
    model.m16[12] = center.x;
    model.m16[13] = center.x;
    model.m16[14] = center.x;
    enable_wireframe(true);
    matrix mvp = model * viewProjection;
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    bind_vec3(shd_color, color);
    draw(m_cube);
    enable_wireframe(false);
}

void EditUtils::DebugSphere(matrix viewProjection, vec3 position, float radius, vec3 color)
{
    matrix model;
    model.translation(position.x, position.y, position.z);
    model.m16[0] = model.m16[5] = model.m16[10] = radius;
    const matrix mvp = model * viewProjection;
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    bind_vec3(shd_color, color);
    draw(m_wireSphere);
}

void EditUtils::DebugLink(matrix viewProjection, vec3 posA, vec3 posB, vec3 color)
{
    matrix orientation;
    vec3 mid = (posA + posB) * 0.5f;
    vec3 dir = posB - posA;

    orientation.LookAt(mid, posB, perpStark(dir));
    matrix scale;
    scale.scale(FLT_EPSILON, FLT_EPSILON, dir.length() * 0.5f);
    const matrix model = scale * orientation;
    const matrix mvp = model * viewProjection;

    glDisable(GL_DEPTH_TEST);
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    bind_vec3(shd_color, color);
    draw(m_cube);
    glEnable(GL_DEPTH_TEST);
}

void EditUtils::DrawFrustum(matrix frameViewProjection, matrix viewProjection)
{
    const matrix mvp = frameViewProjection * viewProjection;
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    vec3 color(0.5f, 0.5f, 0.5f);
    bind_vec3(shd_color, color);
    draw(m_frustum);
}

void EditUtils::DebugLinkInfluence(matrix viewProjection, vec3 posA, float radiusA, vec3 posB, float radiusB, vec3 color)
{
    matrix orientationA, orientationB;
    vec3 mid = (posA + posB) * 0.5f;
    vec3 dir = posB - posA;

    vec3 up = perpStark(dir);
    orientationA.LookAt(posA, posA + dir, up);
    orientationB.LookAt(posB, posB + dir, up);

    matrix scaleA;
    scaleA.scale(radiusA, radiusA, radiusA);

    matrix scaleB;
    scaleB.scale(radiusB, radiusB, radiusB);

    matrix modelA = scaleA * orientationA;
    matrix modelB = scaleB * orientationB;

    glDisable(GL_DEPTH_TEST);
    bind_shader(m_influenceShader);
    bind_matrix("modelA", modelA.m16);
    bind_matrix("modelB", modelB.m16);
    bind_matrix("vp", viewProjection.m16);
    bind_vec3(shd_color, color);
    draw(m_influenceSphere);
    glEnable(GL_DEPTH_TEST);
}

void EditUtils::DrawCharacterLinks(matrix viewProjection, Character* character, int selectedNode)
{
    DrawLinks(viewProjection, character, selectedNode, character->mNodes.data());
}

void EditUtils::DrawLinks(matrix viewProjection, Character* character, int selectedNode, const vec3* positions)
{
    enable_wireframe(true);
    int linkIndex = 0;
    for (auto& link : character->mLinks)
    {
        const auto& a = positions[link.mIndex1];
        const auto& b = positions[link.mIndex2];
        bool selected = selectedNode == linkIndex;
        DebugLink(viewProjection, a, b, selected ? vec3{1.f, 0.5f, 0.1f} : (link.mFixed ? vec3{0.5f, 0.5f, 0.5f} : vec3{1.f, 1.f, 1.f}));
        glDisable(GL_DEPTH_TEST);
        DebugSphere(viewProjection, a, 0.01f, selected ? vec3{1.f, 0.5f, 0.1f} : vec3{0.5f, 0.5f, 0.5f});
        DebugSphere(viewProjection, b, 0.01f, selected ? vec3{1.f, 0.5f, 0.1f} : vec3{0.5f, 0.5f, 0.5f});
        glEnable(GL_DEPTH_TEST);
        linkIndex++;
    }
    enable_wireframe(false);
}

void EditUtils::DrawCharacterInstanceLinks(matrix viewProjection, Character* character, int characterInstance, int selectedNode)
{
    DrawLinks(viewProjection, character, selectedNode, GetCharacterInstanceNodes(characterInstance));
}

static const int MAX_COMBO_LENGTH = 256;
void EditUtils::NodesList(int characterIndex, int& currentNode)
{
    static char** items = nullptr;
    const int bc = GetNodeCount(characterIndex);
    assert(MAX_COMBO_LENGTH > bc);
    if (!items)
    {
        items = new char*[MAX_COMBO_LENGTH];
        for (int i = 0; i < MAX_COMBO_LENGTH; i++)
        {
            items[i] = new char[20];
            sprintf(items[i], "Node %d", i + 1);
        }
    }

    ImGui::ListBox("Nodes", &currentNode, items, bc, 20);
}

void EditUtils::LinksList(int characterIndex, int& currentLink)
{
    static char** items = nullptr;
    const int bc = GetLinkCount(characterIndex);
    assert(MAX_COMBO_LENGTH > bc);
    if (!items)
    {
        items = new char*[MAX_COMBO_LENGTH];
        for (int i = 0; i < MAX_COMBO_LENGTH; i++)
        {
            items[i] = new char[20];
            sprintf(items[i], "Link %d", i + 1);
        }
    }

    ImGui::ListBox("Links", &currentLink, items, bc, 20);
}

bool EditUtils::EditVec3(vec3& vector, const char* txt)
{
    return ImGui::InputFloat3(txt, &vector.x);
}

void EditUtils::DrawInfluenceCone(const matrix& vp, Character* character, int currentLink)
{
    auto& link = character->mLinks[currentLink];
    vec3 pointA = character->mNodes[link.mIndex1];
    vec3 pointB = character->mNodes[link.mIndex2];

    glDisable(GL_DEPTH_TEST);
    const vec3 influenceColor(1.f, 0.5f, 0.1f);
    if (link.mRadius1 >= 0.f)
    {
        editUtils.DebugLinkInfluence(vp, pointA, link.mRadius1, pointB, link.mRadius2, influenceColor);
    }
    else
    {
        editUtils.DebugSphere(vp, pointA, -link.mRadius1, influenceColor);
    }
    glEnable(GL_DEPTH_TEST);
}

bool EditUtils::U8Slider(const char* label, uint8_t& value)
{
    int u8value = value;
    bool dirty = ImGui::SliderInt(label, &u8value, 0, 255);
    value = u8value;
    return dirty;
}
#endif
bool EditUtils::String(const char* label, std::string& str, int stringId)
{
    bool res = false;
    //static int guiStringId = 47414;
    ImGui::PushID(stringId ? stringId : (int)(int64_t)&str);
    char eventStr[512];
    strcpy(eventStr, str.c_str());
    if (ImGui::InputText(label, eventStr, 512))
    {
        str = eventStr;
        res = true;
    }

    ImGui::PopID();
    return res;
}
#if 0
void EditUtils::ToggleButton(const char* str_id, bool* v)
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;
    const ImGuiContext& gg = *ImGui::GetCurrentContext();//*GImGui;
    float ANIM_SPEED = 0.085f;
    if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
        float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
    if (ImGui::IsItemHovered())
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
    else
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

void EditUtils::RenderTrack(const Vec<TrackFrame>& trackFrame, const matrix& mvp, const vec3& color)
{
    if (trackFrame.size() < 2)
    {
        return;
    }
    destroy_mesh(m_track);
    static const int MAX_FRAMES = 1024;
    static vec3 vertices[MAX_FRAMES * 2];
    static unsigned short indices[MAX_FRAMES * 6];
    int vertexCount = static_cast<int>(trackFrame.size() * 2);
    int indexCount = static_cast<int>(trackFrame.size() - 1) * 4 + 2;
    int vertexAdvance = 0;
    int indexAdvance = 0;
    for (unsigned int i = 0; i < trackFrame.size(); i++)
    {
        const auto& mat = trackFrame[i].mat;
        const auto matPosition = vec3(mat.m16[12], mat.m16[13], mat.m16[14]);
        const auto matRight = vec3(mat.m16[0], mat.m16[1], mat.m16[2]);
        const float rightScale = 0.02f;
        const auto right = matPosition + matRight * rightScale;
        vertices[vertexAdvance++] = matPosition;
        vertices[vertexAdvance++] = right;
    }
    for (unsigned int i = 0; i < trackFrame.size() - 1; i++)
    {
        indices[indexAdvance++] = i * 2;
        indices[indexAdvance++] = i * 2 + 1;
        indices[indexAdvance++] = i * 2;
        indices[indexAdvance++] = i * 2 + 2;
    }
    indices[indexAdvance++] = (unsigned short)(trackFrame.size() - 1) * 2;
    indices[indexAdvance++] = (unsigned short)(trackFrame.size() - 1) * 2 + 1;

    m_track = build_mesh((void*)vertices, int(ATTRIB_POSITION), vertexCount, (void*)indices, indexCount, false, GL_LINES);

    glLineWidth(4.f);
    glDepthMask(false);
    glDepthFunc(GL_GREATER);
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    vec3 halfColor(0.f, 0.f, 0.f);// = color * 0.33f;
    bind_vec3(shd_color, halfColor);
    draw(m_track);
    glDepthFunc(GL_LEQUAL);
    bind_shader(m_colorShader);
    bind_matrix(shd_mvpMtx, mvp.m16);
    bind_vec3(shd_color, color);
    draw(m_track);
    glDepthMask(true);
    glLineWidth(1.f);
}
#endif
void EditBase::ContentBurn(ImVec2 contentSize, ImVec2 cursorPos)
{
    // view gizmo
    matrix viewMat = m_trackball.GetWorldToViewMatrix();
    auto& io = ImGui::GetIO();
    ImGuizmo::ViewManipulate(viewMat.m16, m_trackball.radius, ImVec2(contentSize.x - 128 - 10, cursorPos.y + 10), ImVec2(128, 128), 0x10101010);
}
