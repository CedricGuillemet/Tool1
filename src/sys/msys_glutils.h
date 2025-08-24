#pragma once

#define GL_TRIANGLES                      0x0004
#define GL_TEXTURE_2D                     0x0DE1
#define GL_RGBA32F                        0x8814
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A

#ifdef WIN32
#define GL_RG 0x8227
#define GL_RG32F 0x8230
#endif

enum {
    ATTRIB_POSITION = 1 << 0,
    ATTRIB_NORMAL = 1 << 1,
    ATTRIB_TANGENT = 1 << 2,
    ATTRIB_TEXCOORD0 = 1 << 3,
    ATTRIB_BONES = 1 << 4,
    ATTRIB_INFLUENCES = 1 << 5,
    ATTRIB_TEXCOORD1 = 1 << 6,
    ATTRIB_COLOR = 1 << 7
};

struct Mesh
{
    //unsigned int stream;
    unsigned int VboHandle;
    unsigned int ElementsHandle;
    unsigned int element_count;
    int format;
    bool use32bits;
    int renderingMode;
};

struct FrameBuffer
{
    unsigned int fbo;
    unsigned int texture;
    unsigned int width;
    unsigned int height;
    unsigned int depthTexture;
};

void init_glutils();
void get_display_pos(int& x, int& y);
void get_display_size(int& width, int& height);
void set_display_pos(int x, int y);
void set_display_size(int width, int height);
void reset_display_size();

Mesh build_unit_cube_mesh();
Mesh build_frustum_mesh();
Mesh build_unit_grid(int widthSteps, int heightSteps, float width, float height, const vec3& translation = vec3(0.f, 0.f, 0.f));
Mesh build_sphere_mesh(const int steps = 10, float radius = 1.f);
Mesh build_plan_mesh();
Mesh build_wire_sphere_mesh(const int steps = 20);
Mesh build_fullscreen_triangle();
void bind_framebuffer(unsigned framebuffer, unsigned int width, unsigned int height);
unsigned int build_instances(const void* data, int size);

unsigned int load_texture(const void* data, unsigned int width, unsigned int height, int format, int componentType, int internalFormat);
//unsigned int load_texture3D(void* data, unsigned int width, unsigned int height, unsigned int depth, int format, int componentType, int internalFormat);
void bind_texture(const char* name, int slot, unsigned int texture, int textureMode = GL_TEXTURE_2D);
void bind_matrix(const char* name, const float* value);
void bind_matrix(const char* name, const matrix* value, unsigned int matrixCount);

void bind_vec3(const char* name, vec3 value);
void bind_vec3(const char* name, vec4 value);
void bind_vec4(const char* name, vec4 value);
void bind_float(const char* name, float value);
void bind_vec4(const char* name, const float* values, unsigned int count);

FrameBuffer build_framebuffer(unsigned int width, unsigned int height, int format, int componentType, int internalFormat, bool depthBuffer = false);
FrameBuffer build_framebuffer_cubemap(unsigned int width, int format, int componentType, int internalFormat);
void destroy_framebuffer(FrameBuffer framebuffer);
void generate_mipmaps(unsigned int texture, int textureMode);
void bind_backbuffer();
void bind_framebuffer(FrameBuffer framebuffer, int faceIndex = -1);

Mesh build_mesh(const void* vertices, int format, unsigned int vertexCount, const void* indices, unsigned int indexCount, bool use32Bits, int renderingMode = GL_TRIANGLES);
void destroy_mesh(Mesh& mesh);
unsigned int compile_shader(const char* vertex, const char* fragment, const char* shaderName, bool incrementStep = true);
bool shader_compiled(unsigned int shader);
void bind_shader(unsigned int shader);
void destroy_shader(unsigned int shader);
void draw(const Mesh& mesh /*, unsigned int instanceBuffer = 0, int instanceCount = 0*/);
void enable_wireframe(bool enable);

const float* get_cube_positions();
const unsigned short* get_cube_indices();

unsigned int GetSize(int format);
void texture2D_minmax_filter(int minFilter, int maxFilter);
void texture2D_wrap(int wrapR, int wrapS, int wrapT);

void CheckErr(const char* str = "");
#undef GL_TRIANGLES
