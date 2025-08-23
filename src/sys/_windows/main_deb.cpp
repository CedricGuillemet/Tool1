//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#define XRES         1920
#define YRES         1080

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <string.h>
#include <stdio.h>
#include "../../intro.h"
#include "../msys.h"
#include "../events.h"
#include <stdlib.h>

#ifdef TOOLS
#include "tools.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include <cstdint>
#include <stdint.h>
#endif


#ifdef WIN32

float msys_cosf(const float x)
{
    return cosf(x);
}
float msys_sinf(const float x)
{
    return sinf(x);
}
float msys_tanf(const float x)
{
    return tanf(x);
}
float msys_sqrtf(const float x)
{
    return sqrtf(x);
}
float msys_floorf(const float x)
{
    return floorf(x);
}

void msys_memset(void* dst, int val, int amount)
{
    for (int i = 0; i < amount; i++) ((char*)dst)[i] = val;
}

void msys_memcpy(void* dst, const void* ori, int amount)
{
    for (int i = 0; i < amount; i++) ((char*)dst)[i] = ((char*)ori)[i];
}

int msys_strlen(const char* str)
{
    int n; for (n = 0; str[n]; n++); return n;
}
#endif
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------------------------
void msys_debugInit(void) {}
void msys_debugEnd(void) {}
void msys_debugPrintf(const char* format, ...) {}

typedef struct
{
    //---------------
    HINSTANCE   hInstance;
    HDC         hDC;
    HGLRC       hRC;
    HWND        hWnd;
    //---------------
    int         full;
    //---------------
    MSYS_EVENTINFO   events;
}WININFO;

static const PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    32,             // zbuffer
    0,              // stencil!
    0,
    PFD_MAIN_PLANE,
    0, 0, 0, 0
};


static const char fnt_wait[]    = "arial";
static const char msg_wait[]   = "wait while loading...";
static const char msg_error[] = "intro_init()!\n\n"\
                             "  no memory?\n"\
                             "  no music?\n"\
                             "  no sjades?";
static const char wndclass[] = "_";

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static WININFO     wininfo;

static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#ifdef TOOLS
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
#endif
	// salvapantallas
	if( uMsg==WM_SYSCOMMAND && (wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) )
		return( 0 );

	// boton x o pulsacion de escape
	if( uMsg==WM_CLOSE || uMsg==WM_DESTROY /* || (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)*/)
	{
		PostQuitMessage(0);
        return( 0 );
	}

    return( DefWindowProc(hWnd,uMsg,wParam,lParam) );
}

static void window_end(WININFO* info)
{
    exit(0);
    /*
    if( info->hRC )
    {
        wglMakeCurrent( 0, 0 );
        wglDeleteContext( info->hRC );
    }

    if( info->hDC  ) ReleaseDC( info->hWnd, info->hDC );
    if( info->hWnd ) DestroyWindow( info->hWnd );

    UnregisterClass( wndclass, info->hInstance );

    if( info->full )
    {
        ChangeDisplaySettings( 0, 0 );
		while( ShowCursor( 1 )<0 ); // show cursor
    }
    */
}

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
    const int* attribList);
wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;


static int window_init( WININFO *info )
{
	unsigned int	PixelFormat;
    DWORD			dwExStyle, dwStyle;
    DEVMODE			dmScreenSettings;
    WNDCLASSA		wc;
    RECT			rec;

    msys_memset( &wc, 0, sizeof(WNDCLASS) );
    wc.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = info->hInstance;
    wc.lpszClassName = wndclass;
    //wc.hbrBackground=(HBRUSH)CreateSolidBrush(0x00785838);
	
    msys_debugPrintf("window_init A\n");

    if( !RegisterClassA(&wc) )
        return( 0 );

    msys_debugPrintf("window_init B\n");
    if( info->full )
    {
        msys_memset( &dmScreenSettings,0,sizeof(DEVMODE) );
        dmScreenSettings.dmSize       = sizeof(DEVMODE);
        dmScreenSettings.dmFields     = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmPelsWidth  = XRES;
        dmScreenSettings.dmPelsHeight = YRES;

        if( ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
            return( 0 );

        dwExStyle = WS_EX_APPWINDOW;
        dwStyle   = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		while( ShowCursor( 0 )>=0 );	// hide cursor
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle   = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    }

    msys_debugPrintf("window_init C\n");
    rec.left   = 0;
    rec.top    = 0;
    rec.right  = XRES;
    rec.bottom = YRES;

    AdjustWindowRect( &rec, dwStyle, 0 );

    info->hWnd = CreateWindowExA( dwExStyle, wc.lpszClassName, wc.lpszClassName, dwStyle,
                               (GetSystemMetrics(SM_CXSCREEN)-rec.right+rec.left)>>1,
                               (GetSystemMetrics(SM_CYSCREEN)-rec.bottom+rec.top)>>1,
                               rec.right-rec.left, rec.bottom-rec.top, 0, 0, info->hInstance, 0 );
    msys_debugPrintf("window_init D\n");
    if( !info->hWnd )
        return( 0 );

    msys_debugPrintf("window_init E\n");
    if( !(info->hDC=GetDC(info->hWnd)) )
        return( 0 );
    msys_debugPrintf("window_init F\n");
    if( !(PixelFormat=ChoosePixelFormat(info->hDC,&pfd)) )
        return( 0 );
    msys_debugPrintf("window_init G\n");
    if( !SetPixelFormat(info->hDC,PixelFormat,&pfd) )
        return( 0 );
    msys_debugPrintf("window_init H\n");
    // dummy
    HGLRC       dummyContext;
    if( !(dummyContext = wglCreateContext(info->hDC)) )
        return( 0 );
    msys_debugPrintf("window_init I\n");

    if (!wglMakeCurrent(info->hDC, dummyContext))
        return(0);
    msys_debugPrintf("window_init J\n");
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    int gl33_attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    auto wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");

    msys_debugPrintf("wglCreateContextAttribsARB %p\n", wglCreateContextAttribsARB);

    auto wglCreateContextAttribs = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribs");
    msys_debugPrintf("wglCreateContextAttribs %p\n", wglCreateContextAttribs);
    //wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
    //    "wglChoosePixelFormatARB");
    if (!(info->hRC = wglCreateContextAttribsARB(info->hDC, nullptr, gl33_attribs)))
        return(0);

    msys_debugPrintf("window_init K\n");
    if( !wglMakeCurrent(info->hDC,info->hRC) )
        return( 0 );
    
    msys_debugPrintf("window_init L\n");
    //SetForegroundWindow( info->hWnd );    // slightly higher priority
    //SetFocus( info->hWnd );               // sets keyboard focus to the window
    
#ifdef TOOLS
    msys_glextInit();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    intro_init();
    tools_init();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(info->hWnd);
    ImGui_ImplOpenGL3_Init("#version 120");
#endif
    msys_debugPrintf("window_init M\n");
    return( 1 );
}

void Present()
{
    SwapBuffers(wininfo.hDC);
}

void MakeLoaderContextCurrent()
{
}

float GetDPIFactor()
{
    HDC hdc = GetDC(wininfo.hWnd);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(wininfo.hWnd, hdc);
    return (float)dpiX / 96.0f; // Assuming 96 DPI is 100% scaling
}

int WINAPI WinMain( HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG         msg;
    int         done=0;

/*
    if( !msys_debugInit() )
        return 0;
*/
    wininfo.hInstance = GetModuleHandle( 0 );

    msys_debugPrintf("winmain A\n");
    //if( MessageBox( 0, "fullscreen?", info->wndclass, MB_YESNO|MB_ICONQUESTION)==IDYES )
    //   info->full++;

    if( !window_init(&wininfo) )
    {
        window_end( &wininfo );
        MessageBoxA( 0, msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }

    msys_debugPrintf("winmain B\n");

    if( !msys_init((intptr)wininfo.hWnd) ) 
    {
        window_end( &wininfo );
        MessageBoxA( 0, msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }

    msys_debugPrintf("winmain C\n");
#ifndef TOOLS
    if( !intro_init() )
    {
        window_end( &wininfo );
        MessageBox( 0, msg_error, 0, MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }
#endif
    msys_debugPrintf("winmain D\n");
    while( !done )
    {
        if( PeekMessage(&msg,0,0,0,PM_REMOVE) )
        {
            if( msg.message==WM_QUIT ) done=1;
		    TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            done = intro_do();

#ifdef TOOLS
            ImGuiIO& io = ImGui::GetIO();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Start the Dear ImGui frame
            tools_do();

            // Rendering
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();

            GLsizei width = (GLsizei)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
            GLsizei height = (GLsizei)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
            glViewport(0, 0, width, height);
            glClearColor(0,0,0,1);
            glClearDepth(1.f);
            glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
            const bool forceRecording = !_stricmp(lpCmdLine, "-Record");
            done += tools_record(forceRecording)? 1 : 0;
#endif

            static long to = 0; if( !to ) to=timeGetTime(); 
            float t = 0.001f * (float)(timeGetTime() - to);
            Present();
        }
    }

    window_end( &wininfo );

    msys_debugEnd();

    return( 0 );
}
