#include "msys.h"
#include "glext.h"
#ifdef TOOLS
#include <stdio.h>
#include <vector>
#endif

const char* GlslVersionString = "#version 120\n";
unsigned int currentShader;

#define VAR_Bones "Bones"
#define VAR_Color "Color"
#define VAR_Influences "Influences"
#define VAR_Normal "Normal"
#define VAR_Position "Position"
#define VAR_Tangent "Tangent"
#define VAR_TexCoord0 "TexCoord0"
#define VAR_TexCoord1 "TexCoord1"


const char* shd_Bones = VAR_Bones;
const char* shd_Color = VAR_Color;
const char* shd_Influences = VAR_Influences;
const char* shd_Normal = VAR_Normal;
const char* shd_Position = VAR_Position;
const char* shd_Tangent = VAR_Tangent;
const char* shd_TexCoord0 = VAR_TexCoord0;
const char* shd_TexCoord1 = VAR_TexCoord1;

#ifdef __APPLE__
#define glBindVertexArray glBindVertexArrayAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE

#define oglGenBuffers glGenBuffers
#define oglGenVertexArrays glGenVertexArrays
#define oglBindBuffer glBindBuffer
#define oglEnableVertexAttribArray glEnableVertexAttribArray
#define oglGetAttribLocation glGetAttribLocation
#define oglShaderSource glShaderSource
#define oglBindVertexArray glBindVertexArray
#define oglVertexAttribPointer glVertexAttribPointer
#define oglBufferData glBufferData
#define oglCreateShader glCreateShader
#define oglCompileShader glCompileShader
#define oglAttachShader glAttachShader
#define oglLinkProgram glLinkProgram
#define oglCreateProgram glCreateProgram
#define oglUseProgram glUseProgram
#define oglGetShaderiv glGetShaderiv
#define oglGetShaderInfoLog glGetShaderInfoLog
#define oglUniformMatrix4fv glUniformMatrix4fv
#define oglGetUniformLocation glGetUniformLocation
#define oglGenFramebuffers glGenFramebuffers
#define oglBindFramebuffer glBindFramebuffer
#define oglFramebufferTexture2D glFramebufferTexture2D
#define oglBindFramebuffer glBindFramebuffer
#define oglActiveTextureARB glActiveTextureARB
#define oglUniform3f glUniform3f
#define oglUniform4fv glUniform4fv
#define oglDeleteBuffers glDeleteBuffers
#define oglUniform4f glUniform4f
#define oglUniform1f glUniform1f
#define oglUniform1i glUniform1i
#define oglGenerateMipmapEXT glGenerateMipmapEXT
#define oglDeleteBuffers glDeleteBuffers

#define oglBindRenderbuffer glBindRenderbuffer
#define oglRenderbufferStorage glRenderbufferStorage
#define oglFramebufferRenderbuffer glFramebufferRenderbuffer
#define oglGenRenderbuffers glGenRenderbuffers

#define oglCheckFramebufferStatusEXT glCheckFramebufferStatusEXT

#define oglVertexAttribDivisor glVertexAttribDivisor
#define oglDrawElementsInstanced glDrawElementsInstanced

#define oglDeleteShader glDeleteShader
#define oglGetProgramInfoLog glGetProgramInfoLog
#define oglGetProgramiv glGetProgramiv

#endif

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#define GL_RENDERBUFFER               0x8D41
#define GL_DEPTH_ATTACHMENT           0x8D00

#define oglFramebufferRenderbuffer oglFramebufferRenderbufferEXT
#define oglRenderbufferStorage oglRenderbufferStorageEXT
#define oglBindRenderbuffer oglBindRenderbufferEXT

#endif
static GLint viewport[4];
static GLint windowViewport[4];
void init_glutils()
{
    glGetIntegerv(GL_VIEWPORT, viewport);
    msys_memcpy(windowViewport, viewport, sizeof(int) * 4);
}

void get_display_size(int& width, int& height)
{
    width = viewport[2];
    height = viewport[3];
}

void reset_display_size()
{
    msys_memcpy(viewport, windowViewport, sizeof(int) * 4);
}

void set_display_pos(int x, int y)
{
    viewport[0] = x;
    viewport[1] = y;
}

void get_display_pos(int& x, int& y)
{
    x = viewport[0];
    y = viewport[1];
}

void set_display_size(int width, int height)
{
    viewport[2] = width;
    viewport[3] = height;
}

unsigned int GetSize(int format)
{
    return ((format&ATTRIB_POSITION) ? 12 : 0) +
    ((format&ATTRIB_NORMAL) ? 12 : 0) +
    ((format&ATTRIB_TANGENT) ? 12 : 0) +
    ((format&ATTRIB_TEXCOORD0) ? 8 : 0) +
    ((format&ATTRIB_TEXCOORD1) ? 8 : 0) +
    ((format&ATTRIB_BONES) ? 4 : 0) +
    ((format&ATTRIB_INFLUENCES) ? 16 : 0) +
    ((format&ATTRIB_COLOR) ? 4 : 0);
}

static const unsigned short cubeIndices[] = {
    0, 2, 3, 0, 3, 1,
    2, 6, 7, 2, 7, 3,
    6, 4, 5, 6, 5, 7,
    4, 0, 1, 4, 1, 5,
    0, 4, 6, 0, 6, 2,
    //1, 5, 7, 1, 7, 3,
    3, 7, 1, 7,5,1
  };

static const float cubeVertices[] = {
  -1.0f,1.0f,1.0f,
  -1.0f,-1.0f,1.0f,
  1.0f,1.0f,1.0f,
  1.0f,-1.0f,1.0f,
  -1.0f,1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  1.0f,1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
};

static const unsigned short planIndices[] = {
    0, 2, 3, 0, 3, 1,
  };

static const unsigned short frustumIndices[] = {
    1, 0, 0, 2, 2, 3, 3, 1,
    5, 4, 4, 6, 6, 7, 7, 5,
    0, 4, 1, 5, 2, 6, 3, 7,
  };

static const float planVertices[] = {
  -1.0f, 0.f, 1.0f,       0.f, 1.f,
  -1.0f, 0.f, -1.0f,      0.f, 0.f,
  1.0f, 0.f, 1.0f,        1.f, 1.f,
  1.0f,0.f, -1.0f,        1.f, 0.f,
};

const float* get_cube_positions() { return cubeVertices; }
const unsigned short* get_cube_indices() { return cubeIndices; }

Mesh build_unit_cube_mesh()
{
    return build_mesh((void*)cubeVertices, int(ATTRIB_POSITION), 8, (void*)cubeIndices, 36, false);
}

Mesh build_frustum_mesh()
{
    return build_mesh((void*)cubeVertices, int(ATTRIB_POSITION), 8, (void*)frustumIndices, 24, false, GL_LINES);
}

int gridIndices[256 * 256 * 6];
struct gridVertex
{
    vec3 position;
    float u,v;
};
gridVertex gridVertices[256 * 256];
Mesh build_unit_grid(int widthSteps, int heightSteps, float width, float height, const vec3& translation)
{
    vec3 offset(float(widthSteps - 1) / 2.f, 0.f, float(heightSteps - 1) / 2.f);
    int vertexIndex = 0;
    for (int y = 0; y < heightSteps; y++)
    {
        for (int x = 0; x < widthSteps; x++)
        {
            auto& g = gridVertices[vertexIndex];
            g.u = float(x) / float(widthSteps-1);
            g.v = float(y) / float(heightSteps-1);
            
            gridVertices[vertexIndex].position = vec3(g.u * width - width * 0.5f, 0.f, g.v * height - height * 0.5f) + translation;
            
            vertexIndex++;
        }
    }
    int index = 0;
    for (int y = 0; y < heightSteps - 1; y++)
    {
        for (int x = 0; x < widthSteps - 1; x++)
        {
            int locals[4] = {
                y * widthSteps + x,
                y * widthSteps + x + 1,
                (y + 1) * widthSteps + x,
                (y + 1) * widthSteps + x + 1,
            };
            for (int i = 0;i<4;i++)
            {
                assert(locals[i] < vertexIndex);
            }
            gridIndices[index++] = locals[0];
            gridIndices[index++] = locals[1];
            gridIndices[index++] = locals[2];
            
            gridIndices[index++] = locals[1];
            gridIndices[index++] = locals[3];
            gridIndices[index++] = locals[2];
        }
    }
    return build_mesh((void*)gridVertices, int(ATTRIB_POSITION|ATTRIB_TEXCOORD0), vertexIndex, (void*)gridIndices, index, true);
}


Mesh build_plan_mesh()
{
    return build_mesh((void*)planVertices, int(ATTRIB_POSITION + ATTRIB_TEXCOORD0), 4, (void*)planIndices, 6, false);
}

Mesh build_fullscreen_triangle()
{
    static const unsigned short indices[] = {
        0, 1, 2
      };
    
    static const float vertices[] = {
        -1,-1, 0,
        3,-1, 0,
        -1, 3, 0
    };
    
    return build_mesh((void*)vertices, int(ATTRIB_POSITION), 3, (void*)indices, 3, false);
}

#ifdef TOOLS
Mesh build_wire_sphere_mesh(const int steps)
{
    int vertexCount = (steps + 1) * 3;
    int indexCount = steps * 3 * 2;
    vec3* vertices = new vec3 [vertexCount];
    uint16* indices = new uint16 [indexCount];
    for (int i = 0;i < steps + 1 ; i++)
    {
        float ng = ((PI * 2.f) / float(steps)) * i;
        float cs = msys_cosf(ng);
        float sn = msys_sinf(ng);
        vertices[i] = vec3(cs, 0.f, sn);
        vertices[i + steps + 1] = vec3(cs, sn, 0.f);
        vertices[i + (steps + 1) * 2] = vec3(0.f, cs, sn);
    }
    int index = 0;
    for (int j = 0; j < 3; j++)
    {
        int offsetIndex = j * (steps + 1);
        for (int i = 0; i < steps; i++)
        {
            indices[index++] = i + offsetIndex;
            indices[index++] = i + offsetIndex + 1;
        }
    }
    
    return build_mesh((void*)vertices, int(ATTRIB_POSITION), vertexCount, (void*)indices, indexCount, false, GL_LINES);
}
#endif
struct PositionUV
{
    vec3 position;
    float uvs[2];
};
Mesh build_sphere_mesh(const int steps, float radius)
{
    static const int maxSteps = 45;
    assert(steps <= maxSteps);
    int vertexCount = steps * steps;
    int indexCount = (steps-1)*(steps-1) * 3 * 2;
    //PositionUV* vertices = new PositionUV [vertexCount];
    //uint16* indices = new uint16 [indexCount];
    static PositionUV vertices[maxSteps * maxSteps];
    static uint16 indices[(maxSteps-1)*(maxSteps-1)*3*2];
    int index = 0;
    for (int y = 0; y < steps ; y++)
    {
        float ngy = (PI / float(steps-1)) * y;
        for (int x = 0; x < steps ; x++)
        {
            float ngx = ((PI * 2.f) / float(steps-1)) * x;
            
            float csx = msys_cosf(ngx);
            float snx = msys_sinf(ngx);
            float csy = msys_cosf(ngy);
            float sny = msys_sinf(ngy);

            vertices[index].position = vec3(csx * sny, csy, snx * sny) * radius;
            vertices[index].uvs[0] = (1.f / float(steps-1)) * x;
            vertices[index].uvs[1] = (1.f / float(steps-1)) * y;
            
            index ++;
        }
    }
    index = 0;
    for (int y = 0; y < steps - 1 ; y++)
    {
        for (int x = 0; x < steps - 1; x++)
        {
            indices[index++] = y * steps + x;
            indices[index++] = y * steps + x + 1;
            indices[index++] = (y + 1) * steps + x + 1;
            
            indices[index++] = y * steps + x;
            indices[index++] = (y + 1) * steps + x + 1;
            indices[index++] = (y + 1) * steps + x;
        }
    }
    
    return build_mesh((void*)vertices, int(ATTRIB_POSITION | ATTRIB_TEXCOORD0), vertexCount, (void*)indices, indexCount, false);
}

Mesh build_mesh(const void* vertices, int format, unsigned int vertexCount, const void* indices, unsigned int indexCount, bool use32Bits, int renderingMode)
{
    unsigned int    VboHandle, ElementsHandle;
    
    oglGenBuffers(1, &VboHandle);
    oglGenBuffers(1, &ElementsHandle);

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    oglBindBuffer(GL_ARRAY_BUFFER, VboHandle);
    oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementsHandle);
    
    auto size = GetSize(format);
    oglBufferData(GL_ARRAY_BUFFER, size * vertexCount, vertices, GL_STATIC_DRAW);
    
    auto indexSize = use32Bits ? 4 : 2;
    if (indices)
    {
        oglBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, indices, GL_STATIC_DRAW);
    }
    
    return {VboHandle, ElementsHandle, indexCount, format, use32Bits, renderingMode};
}

unsigned int build_instances(const void* data, int size)
{
    unsigned int instancesHandle;

    oglGenBuffers(1, &instancesHandle);

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    oglBindBuffer(GL_ARRAY_BUFFER, instancesHandle);

    //auto size = GetSize(format);
    oglBufferData(GL_ARRAY_BUFFER, size /* * vertexCount*/, data, GL_STATIC_DRAW);

    return instancesHandle;
}

void destroy_mesh(Mesh& mesh)
{
#ifdef TOOLS
    oglDeleteBuffers(1, &mesh.VboHandle);
    oglDeleteBuffers(1, &mesh.ElementsHandle);
    msys_memset(&mesh, 0, sizeof(Mesh));
#endif
}

void texture2D_minmax_filter(int minFilter, int maxFilter)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxFilter);
}

void texture2D_wrap(int wrapR, int wrapS, int wrapT)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

unsigned int load_texture(const void* data, unsigned int width, unsigned int height, int format, int componentType, int internalFormat)
{
    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    texture2D_minmax_filter(GL_LINEAR, GL_LINEAR);
    texture2D_wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
/*#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
    GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#endif*/
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, componentType, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texId;
}
#if 0
unsigned int load_texture3D(void* data, unsigned int width, unsigned int height, unsigned int depth, int format, int componentType, int internalFormat)
{
    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_3D, texId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
/*#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
    GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#endif*/
    glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, componentType, data);
    glBindTexture(GL_TEXTURE_3D, 0);
    return texId;
}
#endif
#ifdef TOOLS
void Log(const char* fmt, ...);
#endif
static void CheckShader(GLuint shader, const char* shaderType, const char* shaderName)
{
    GLint success = 0;
    oglGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logSize = 0;
        oglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        if (logSize)
        {
            static const int MAX_LEN = 8192;
            char txt[MAX_LEN];
            GLsizei loglength;
            oglGetShaderInfoLog(shader, MAX_LEN, &loglength, txt);
#ifdef TOOLS
            
            Log("%s %s %s\n", shaderName, shaderType, txt);
#ifdef WIN32
            OutputDebugStringA(shaderName);
            OutputDebugStringA(shaderType);
            OutputDebugStringA(txt);
#endif
#endif
            //exit(-1);
        } else {
            //printf("Error compiling %s (%s).\n", shaderName, shaderType);
        }
    }
}

void generate_mipmaps(unsigned int texture, int textureMode)
{
    glBindTexture(textureMode/*GL_TEXTURE_CUBE_MAP*/, texture);
    oglGenerateMipmapEXT(textureMode);
    glBindTexture(textureMode, 0);
}

FrameBuffer build_framebuffer_cubemap(unsigned int width, int format, int componentType, int internalFormat)
{
    FrameBuffer res;
    res.width = width;
    res.height = width;


    oglGenFramebuffers(1, &res.fbo);
    oglBindFramebuffer(GL_FRAMEBUFFER, res.fbo);

    glGenTextures(1, &res.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, res.texture);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, width, 0, format, componentType, (void*)nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    //oglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP, res.texture, 0);

    //glDrawBuffer(GL_COLOR_ATTACHMENT0);
    //assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    oglBindFramebuffer(GL_FRAMEBUFFER, 0);
    return res;
}

#ifdef TOOLS
void CheckErr(const char* str)
{
    auto lasterr = glGetError();
    if (lasterr)
    {
        char tmps[512];
        sprintf(tmps, "%s err %d\n", str, int(lasterr));
#ifdef WIN32
        OutputDebugStringA(tmps);
#else
        printf(tmps);
#endif
    }
    assert(lasterr == GL_NO_ERROR);
}
#endif
FrameBuffer build_framebuffer(unsigned int width, unsigned int height, int format, int componentType, int internalFormat, bool depthBuffer)
{
    FrameBuffer res;
    res.width = width;
    res.height = height;
    res.depthTexture = 0;
    glGenTextures(1, &res.texture);
    glBindTexture(GL_TEXTURE_2D, res.texture);
      
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, componentType/*GL_UNSIGNED_BYTE*/, NULL);

    texture2D_minmax_filter(GL_LINEAR, GL_LINEAR);
    oglGenFramebuffers(1, &res.fbo);
    oglBindFramebuffer(GL_FRAMEBUFFER, res.fbo);
    oglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, res.texture, 0);
    if (depthBuffer)
    {
        // Create and attach a depth buffer
        /*GLuint depthAttachment;
        oglGenRenderbuffers(1, &depthAttachment);
        oglBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
        oglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        oglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
        */
        
        glGenTextures(1, &res.depthTexture);
        glBindTexture(GL_TEXTURE_2D, res.depthTexture);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        texture2D_minmax_filter(GL_NEAREST, GL_NEAREST);
        texture2D_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        //oglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, res.depthTexture, 0);
        oglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, res.depthTexture, 0);
    }
    assert(oglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE_EXT);

    oglBindFramebuffer(GL_FRAMEBUFFER, 0);
    return res;
}

void destroy_framebuffer(FrameBuffer framebuffer)
{
    oglDeleteBuffers(1, &framebuffer.fbo);
}

void bind_backbuffer()
{
    oglBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void checkProgramLinking(GLuint program, const char* shaderName) {
    // Step 1: Retrieve the link status
    GLint linked;
    oglGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (linked == GL_FALSE) {
        // Step 2: Retrieve the length of the linking log
        GLint logLength = 0;
        oglGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        // Step 3: Allocate a vector to hold the log
        std::vector<GLchar> log(logLength);

        // Step 4: Retrieve the actual log
        oglGetProgramInfoLog(program, logLength, &logLength, log.data());

        // Step 5: Display the log, highlighting the errors
        //std::cerr << "Shader program linking failed with the following errors:\n" << log.data() << std::endl;
    }
    else {
        //std::cout << "Shader program linked successfully!" << std::endl;
    }
}

void bind_framebuffer(FrameBuffer framebuffer, int faceIndex)
{
    oglBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    if (faceIndex>=0)
    {
        oglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+faceIndex, framebuffer.texture, 0);
    }
    glViewport(0, 0, framebuffer.width, framebuffer.height);
    //assert(glGetError() == GL_NO_ERROR);
}
static const GLchar* PIScript = "\nfloat PI = 3.141592;\n";
static const GLchar* VaryingScript = "\nuniform mat4 cameraMtx;uniform sampler2D previousFrame; uniform float frameIndex; uniform float iterative; uniform float viewFraction;\n";

unsigned int compile_shader(const char* vertex, const char* fragment, const char* shaderName, bool incrementStep)
{
    const GLchar* vertex_shader_with_version[] = {GlslVersionString, PIScript, VaryingScript, /*INC_SHD(libraryScript), INC_SHD(libraryScript2), INC_SHD(libraryVertex), INC_SHD(libraryVarying), INC_SHD(libraryUniforms),*/ vertex};
    GLuint vert_handle = oglCreateShader(GL_VERTEX_SHADER);
    oglShaderSource(vert_handle, sizeof(vertex_shader_with_version)/sizeof(char*), vertex_shader_with_version, nullptr);
    oglCompileShader(vert_handle);
#ifdef TOOLS
    CheckShader(vert_handle, "vertex shader", shaderName);
#endif
    const GLchar* fragment_shader_with_version[] = {GlslVersionString, PIScript, VaryingScript, /*INC_SHD(libraryScript), INC_SHD(libraryScript2), INC_SHD(libraryVarying), INC_SHD(libraryUniforms),*/ fragment};
    GLuint frag_handle = oglCreateShader(GL_FRAGMENT_SHADER);
    oglShaderSource(frag_handle, sizeof(fragment_shader_with_version)/sizeof(char*), fragment_shader_with_version, nullptr);
    oglCompileShader(frag_handle);
#ifdef TOOLS
    CheckShader(frag_handle, "fragment shader", shaderName);
#endif
    // Link
    unsigned int ShaderHandle = oglCreateProgram();
    oglAttachShader(ShaderHandle, vert_handle);
    oglAttachShader(ShaderHandle, frag_handle);
    oglLinkProgram(ShaderHandle);
#ifdef TOOLS
    checkProgramLinking(ShaderHandle, shaderName);
#endif

    return ShaderHandle;
}

bool shader_compiled(unsigned int shader)
{
    GLint linked;
    oglGetProgramiv(shader, GL_LINK_STATUS, &linked);
    return linked;
}

void bind_matrix(const char* name, const float* value)
{
    auto location = oglGetUniformLocation(currentShader, name);
    oglUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void bind_matrix(const char* name, const matrix* value, unsigned int matrixCount)
{
    auto location = oglGetUniformLocation(currentShader, name);
    oglUniformMatrix4fv(location, matrixCount, GL_FALSE, value[0].m16);
}

void bind_vec3(const char* name, vec3 value)
{
    auto location = oglGetUniformLocation(currentShader, name);
    float tmp[4] = { value.x, value.y, value.z, 0.f };
    //oglUniform4fv(location, 1, tmp);
    oglUniform3f(location, value.x, value.y, value.z);
}

void bind_vec3(const char* name, vec4 value)
{
    auto location = oglGetUniformLocation(currentShader, name);
    //oglUniform4fv(location, 1, &value.x);
    oglUniform3f(location, value.x, value.y, value.z);
}

void bind_float(const char* name, float value)
{
    auto location = oglGetUniformLocation(currentShader, name);
    //oglUniform4fv(location, 1, &value.x);
    oglUniform1f(location, value);
}

void bind_vec4(const char* name, vec4 value)
{
    auto location = oglGetUniformLocation(currentShader, name);
    //oglUniform4fv(location, 1, &value.x);
    oglUniform4f(location, value.x, value.y, value.z, value.w);
}

void bind_vec4(const char* name, const float* values, unsigned int count)
{
    auto location = oglGetUniformLocation(currentShader, name);
    //oglUniform4fv(location, 1, &value.x);
    oglUniform4fv(location, count, values);
}

void bind_texture(const char* name, int slot, unsigned int texture, int textureMode)
{
    auto location = oglGetUniformLocation(currentShader, name);
    oglUniform1i(location, slot);
    oglActiveTextureARB(GL_TEXTURE0 + slot);
    glBindTexture(textureMode, texture);
}

void bind_shader(unsigned int shader)
{
    oglUseProgram(shader);
    currentShader = shader;
}

void destroy_shader(unsigned int shader)
{
    oglDeleteShader(shader);
}

void enable_wireframe(bool enable)
{
    glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
}

void draw(const Mesh& mesh /*, unsigned int instanceBuffer, int instanceCount*/)
{
    oglBindBuffer(GL_ARRAY_BUFFER, mesh.VboHandle);
    oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ElementsHandle);


    auto AttribLocationVtxPosition = (GLuint)oglGetAttribLocation(currentShader, shd_Position);
    auto AttribLocationVtxNormal = (GLuint)oglGetAttribLocation(currentShader, shd_Normal);
    auto AttribLocationVtxTangent = (GLuint)oglGetAttribLocation(currentShader, shd_Tangent);
    auto AttribLocationVtxTexCoord0 = (GLuint)oglGetAttribLocation(currentShader, shd_TexCoord0);
    auto AttribLocationVtxTexCoord1 = (GLuint)oglGetAttribLocation(currentShader, shd_TexCoord1);
    auto AttribLocationVtxBones = (GLuint)oglGetAttribLocation(currentShader, shd_Bones);
    auto AttribLocationVtxInfluences = (GLuint)oglGetAttribLocation(currentShader, shd_Influences);
    auto AttribLocationVtxColor = (GLuint)oglGetAttribLocation(currentShader, shd_Color);
    
    auto format = mesh.format;
    
    if (format & ATTRIB_POSITION && AttribLocationVtxPosition != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxPosition);
    }
    if (format&ATTRIB_NORMAL && AttribLocationVtxNormal != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxNormal);
    }
    if (format&ATTRIB_TANGENT && AttribLocationVtxTangent != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxTangent);
    }
    if (format&ATTRIB_TEXCOORD0 && AttribLocationVtxTexCoord0 != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxTexCoord0);
    }
    if (format&ATTRIB_TEXCOORD1 && AttribLocationVtxTexCoord1 != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxTexCoord1);
    }
    if (format&ATTRIB_BONES && AttribLocationVtxBones != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxBones);
    }
    if (format&ATTRIB_INFLUENCES && AttribLocationVtxInfluences != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxInfluences);
    }
    if (format&ATTRIB_COLOR && AttribLocationVtxColor != 0xFFFFFFFF)
    {
        oglEnableVertexAttribArray(AttribLocationVtxColor);
    }
    
    auto size = GetSize(format);
    int offset = 0;
    if (format&ATTRIB_POSITION && AttribLocationVtxPosition != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxPosition,   3, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)offset);
    }
    offset += (format & ATTRIB_POSITION) ? 12 : 0;
    if (format&ATTRIB_NORMAL && AttribLocationVtxNormal != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxNormal,    3, GL_FLOAT,         GL_TRUE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxNormal != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxNormal, 3, GL_FLOAT,            GL_TRUE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_NORMAL) ? 12 : 0;
    
    if (format&ATTRIB_TANGENT && AttribLocationVtxTangent != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxTangent,    3, GL_FLOAT,         GL_TRUE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxTangent != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxTangent, 3, GL_FLOAT,            GL_TRUE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_TANGENT) ? 12 : 0;
    
    if (format&ATTRIB_TEXCOORD0 && AttribLocationVtxTexCoord0 != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxTexCoord0, 2, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxTexCoord0 != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxTexCoord0, 2, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_TEXCOORD0) ? 8 : 0;

    if (format&ATTRIB_BONES && AttribLocationVtxBones != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxBones,    4, GL_UNSIGNED_BYTE,         GL_FALSE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxBones != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxBones, 4, GL_UNSIGNED_BYTE,            GL_FALSE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_BONES) ? 4 : 0;

    if (format&ATTRIB_INFLUENCES && AttribLocationVtxInfluences != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxInfluences,    4, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxInfluences != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxInfluences, 4, GL_FLOAT,            GL_FALSE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_INFLUENCES) ? 16 : 0;

    if (format&ATTRIB_TEXCOORD1 && AttribLocationVtxTexCoord1 != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxTexCoord1, 2, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxTexCoord1 != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxTexCoord1, 2, GL_FLOAT,         GL_FALSE, size, (void*)(int64_t)0);
    }
    offset += (format & ATTRIB_TEXCOORD1) ? 8 : 0;
    
    if (format&ATTRIB_COLOR && AttribLocationVtxColor != 0xFFFFFFFF)
    {
        oglVertexAttribPointer(AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE,         GL_TRUE, size, (void*)(int64_t)offset);
    }
    else if(AttribLocationVtxColor != -1)
    {
        oglVertexAttribPointer(AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE,         GL_TRUE, size, (void*)0);
    }
    /*
    if (instanceBuffer)
    {
        oglBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
        oglVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        oglEnableVertexAttribArray(1);
        oglVertexAttribDivisor(1, 1);// Tell OpenGL this attribute advances once per instance
        oglDrawElementsInstanced(mesh.renderingMode, mesh.element_count, mesh.use32bits ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);
    }
    else
    */
    {
        glLineWidth(1.f);
        glDrawElements(mesh.renderingMode, mesh.element_count, mesh.use32bits ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);
    }
    //offset += (format & ATTRIB_COLOR) ? 4 : 0;
    
    oglBindVertexArray(0);
}
