/*
*
* "Orchestration of consent"
*

TOOL
- windows: button subjective/arc rotate
- save/load camera position
- set/load/save defines

- accumulative path tracing checkbox
- combo box resolution(1/2, 1/4, 1/8)

 */

#include "msys.h"
#include "intro.h"
#include "msys_debug.h"
#include <stdio.h>
#include <string>
#include "edit.h"

void ClearLog();
/*
static const char* shaderFileName = "shader.glsl";

inline std::string ReadFile(const char* filePath)
{
    FILE* fp = fopen(filePath, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        auto bufSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        //char *buf = new char[bufSize];
        std::string str;
        str.resize(bufSize);
        fread(str.data(), bufSize, 1, fp);
        fclose(fp);
        return str;
    }
    assert(false);
    return "";
}
*/
std::string shaderContent;
/*
std::string shaderContent = R"(
#define TWO_PI 6.28318530718
varying vec4 parameters;
void main() {
vec2 tc = vec2(gl_FragCoord.xy);
float t = (tc.y * 4096. + tc.x) / 48000. ;
vec2 dest = vec2(0.);//vec2(sin(time * TWO_PI * 440.));

gl_FragColor = vec4(dest.xy, 0., 0.);
}
)";
/*
void SaveFile(const char* filePath)
{
    FILE* fp = fopen(filePath, "wb");
    if (fp)
    {
        fwrite(shaderContent.data(), shaderContent.length(), 1, fp);
        fclose(fp);
    }
}
*/
unsigned int compiledShader = 0;

const char* fsVertex = R"(
layout(location = 0) in vec3 Position;

void main() {
    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
)";


void UpdateShader()
{
    ClearLog();
    
    extern Edit edit;
    
    shaderContent = R"(
    #define TWO_PI 6.28318530718
    in vec4 parameters;        // Replaces 'varying' from vertex shader
    out vec4 FragColor;        // Replaces gl_FragColor

    void main() {
    vec2 tc = vec2(gl_FragCoord.xy);
    float t = (tc.y * 4096. + tc.x) / 48000. ;
    vec2 dest = vec2(0.);//vec2(sin(time * TWO_PI * 440.));
    )";

    shaderContent += edit.GenerateShader();

    shaderContent += R"(
    FragColor = vec4(dest.xy, 0.0, 0.0);
    })";
    
    
    shaderContent = R"(
    out vec4 FragColor;        // Replaces gl_FragColor

    void main() {
    FragColor = vec4(1., 0.666, 0.0, 0.);
    }
)";

    unsigned int newCompiledShader = compile_shader(fsVertex, shaderContent.c_str(), "demoShader", false);
    
    if (shader_compiled(newCompiledShader))
    {
        if (compiledShader > 0)
        {
            destroy_shader(compiledShader);
        }
        compiledShader = newCompiledShader;
    }
}

int intro_init()
{
    FILE *fp = fopen("fa-solid-900.ttf", "rt");
    if (!fp) {
        exit(0);
    }
    fclose(fp);
    /*shaderContent = ReadFile(shaderFileName);
    */
    return 1;
}

//int frame = 0;
int displayWidth, displayHeight;
Mesh fullscreenMesh;
int frameBeforeSaving = -1;
bool needRendering = true;

void ResetSaveTimer()
{
    frameBeforeSaving = 60;
}

int intro_do(void)
{
    if (needRendering)
    {
        glViewport(0, 0, 1920, 1080);
        glDepthMask(true);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClearDepth(1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        init_glutils();
        get_display_size(displayWidth, displayHeight);
        
        static bool oneTimeInit = false;
        if (!oneTimeInit)
        {
            fullscreenMesh = build_fullscreen_triangle();
            fullscreenMesh = build_fullscreen_triangle();
            fullscreenMesh = build_fullscreen_triangle();
            fullscreenMesh = build_fullscreen_triangle();
            UpdateShader();
            oneTimeInit = true;
        }
#ifndef TOOLS
        if (!RenderFrame(frame, nullptr))
        {
#ifdef WIN32
            extern int done;
            done = 1;
#else
            exit(0);
#endif
        }
        frame++;
#endif
        
    }
    // save and compile
    frameBeforeSaving --;
    if (!frameBeforeSaving)
    {
        //SaveFile(shaderFileName);
        UpdateShader();
        needRendering = true;
    }
    return 0;
}

bool RenderFrame(int frame, FrameBuffer* contentViewFrameBuffer)
{

    
    return true;
}
