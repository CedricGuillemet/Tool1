#include "imgui.h"
#include "msys_vector.h"
#include "msys_libc.h"
#include "msys_vector.h"
#include <math.h>
#include "ImGuizmo.h"
#include "msys_random.h"
#include "TextEdit.h"
#include "msys_glext.h"
#include "edit.h"
#include "msys_sound.h"

void ResetSaveTimer();
bool RenderFrame(int frame, FrameBuffer* contentViewFrameBuffer);
extern std::string shaderContent;
std::unique_ptr<TextEditor> editor;

std::string saveImageName = "image.tga";

void TextEdit::Init()
{
    m_trackball.radius = -1.f;//(characterRendering.m_max - characterRendering.m_min).length() * 0.75f;
    
    
    auto lang = TextEditor::LanguageDefinitionId::Glsl;
    editor = std::make_unique<TextEditor>();
    editor->SetLanguageDefinition(lang);
    editor->SetText(shaderContent);
}

bool TextEdit::ShowGUI()
{
    ImGui::Begin("Text");
    bool res = ImGui::IsWindowFocused();
    editor->Render("TextEditor");
    std::string newText = editor->GetText();
    if (newText != shaderContent)
    {
        shaderContent = newText;
        ResetSaveTimer();
    }
    ImGui::End();
    return res;
}

extern unsigned int compiledShader;
extern Mesh fullscreenMesh;

#define WIDTH 1920
#define HEIGHT 1080

void save_tga(const char* filename, const unsigned char* rgba) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return;
    }

    uint8_t header[18] = {0};
    header[2] = 2;               // Image type = uncompressed true-color
    header[12] = WIDTH & 0xFF;   // Width (low byte)
    header[13] = (WIDTH >> 8);   // Width (high byte)
    header[14] = HEIGHT & 0xFF;  // Height (low byte)
    header[15] = (HEIGHT >> 8);  // Height (high byte)
    header[16] = 32;             // Bits per pixel
    header[17] = 0x20;           // Image descriptor (top-left origin)

    fwrite(header, 1, 18, f);

    // Convert RGBA to BGRA and write
    for (int y = HEIGHT-1; y >= 0; y--) {
        for (int x = 0; x < WIDTH; ++x) {
            const unsigned char* pixel = &rgba[(y * WIDTH + x) * 4];
            uint8_t bgra[4] = { pixel[2], pixel[1], pixel[0], pixel[3] }; // B, G, R, A
            fwrite(bgra, 1, 4, f);
        }
    }

    fclose(f);
}

float data[4096 * 4096 * 2];
void TextEdit::ShowContent(FrameBuffer& contentViewFrameBuffer, unsigned int previousFrame, matrix view, matrix projection, const ImVec2 mouseNormalizedPosition)
{
    extern bool needRendering;

    bind_framebuffer(contentViewFrameBuffer);

    
    glViewport(0, 0, contentViewFrameBuffer.width, contentViewFrameBuffer.height);

    if (needRendering)
    {
        glDisable(GL_CULL_FACE);
        bind_shader(compiledShader);
        glRects(-1, -1, 1, 1);
        
        needRendering = false;
        
        bind_backbuffer();
        
        glBindTexture(GL_TEXTURE_2D, contentViewFrameBuffer.texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, data);

        
        
        
        msys_soundInit(data, sizeof(float) * 4096 * 4096 * 2);
        msys_soundStart();
    }
    /*
    if (saveImage)
    {
        bind_backbuffer();
        unsigned char* data = (unsigned char*)malloc(WIDTH * HEIGHT * 4);
        glBindTexture(GL_TEXTURE_2D, contentViewFrameBuffer.texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        save_tga(saveImageName.c_str(), data);
        free(data);
        saveImage = false;
    }*/
}
