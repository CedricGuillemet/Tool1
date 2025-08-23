// /CRINKLER /PRINT:LABELS /PRINT:IMPORTS /COMPMODE:SLOW /HASHSIZE:200 /ORDERTRIES:6000  /UNSAFEIMPORT

//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//
#define XRES        1920
#define YRES        1080

#ifdef A64BITS
#pragma pack(8) // VERY important, so WNDCLASS get's the correct padding and we don't crash the system
#endif

//#pragma check_stack(off)
//#pragma runtime_checks("", off)

//#define ALLOWWINDOWED       // allow windowed mode

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <string.h>
#include "../../intro.h"
#include "../msys.h"
#include "../events.h"
//#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
int __fastcall intro_init(void)
{
    return 1;
}
int __fastcall intro_do(void)
{
    return 1;
}*/
/*
void __std_terminate()
{

}
*/

extern"C" {
/*int atexit(
    void(__cdecl* func)(void)
)
{
    return 0;
}
*/

    /*
__declspec(naked) void
memset(void* dst, int val, int amount)
{

}
*/
}


#ifdef _DEBUGPRINTF
static FILE* fp;
void msys_debugInit(void)
{
    fp = fopen("debug.txt", "wt");
    if (!fp)
        return;

    fprintf(fp, "debug file\n");
    fprintf(fp, "-------------------------\n");
    fflush(fp);

    return;
}

void msys_debugEnd(void)
{
    fprintf(fp, "-------------------------\n");
    fflush(fp);
    fclose(fp);
}

void msys_debugPrintf(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    vfprintf(fp, format, arglist);
    fflush(fp);
    va_end(arglist);
}
#else
void msys_debugInit(void) {}
void msys_debugEnd(void) {}
void msys_debugPrintf(const char* format, ...) {}
#endif
//----------------------------------------------------------------------------

typedef struct
{
    HINSTANCE   hInstance;
    HWND        hWnd;
    HDC         hDC;
    HGLRC       hRC;
    int         full;
}WININFO;

extern "C" int _fltused = 0;

static const PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
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

static DEVMODE screenSettings = { {0},
    #if _MSC_VER < 1400
    0,0,148,0,0x001c0000,{0},0,0,0,0,0,0,0,0,0,{0},0,32,XRES,YRES,0,0,      // Visual C++ 6.0
    #else
    0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,XRES,YRES,{0}, 0,           // Visuatl Studio 2005
    #endif
    #if(WINVER >= 0x0400)
    0,0,0,0,0,0,
    #if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    0,0
    #endif
    #endif
};
static const char wndclass[] = "_in";
static const char msg_error[] = "no memory?\nno music?\nno shades?";

WININFO     wininfo;
MSG         msg;
int         done = 0;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
    const int* attribList);
wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;


void PumpMessages()
{
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_CLOSE || (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)) { done = 1; break; }
        DispatchMessage(&msg);
    }
}

void Present()
{
    SwapBuffers(wininfo.hDC);
    PumpMessages();
    if (done)
    {
        ExitProcess(0);
    }
}

//----------------------------------------------------------------------------

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
        return 0;

    if (uMsg == WM_CLOSE || (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE))
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static void window_end(WININFO* info)
{
    if (info->hRC)
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(info->hRC);
    }

    if (info->hDC) ReleaseDC(info->hWnd, info->hDC);
    if (info->hWnd) DestroyWindow(info->hWnd);

    UnregisterClassA(wndclass, info->hInstance);

#ifdef ALLOWWINDOWED
    if (info->full)
#endif
    {
        ChangeDisplaySettings(0, 0);
        ShowCursor(1);
    }
}

static int window_init(WININFO* info)
{
    unsigned int	PixelFormat;
    DWORD			dwExStyle, dwStyle;
    RECT			rec;
    WNDCLASSA		wc;

    done = 0;
    info->hInstance = GetModuleHandle(0);

    msys_memset(&wc, 0, sizeof(WNDCLASSA));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = info->hInstance;
    wc.lpszClassName = wndclass;

    if (!RegisterClassA((WNDCLASSA*)&wc))
    {
        msys_debugPrintf("RegisterClassA Fail\n");
        return(0);
    }

#ifdef ALLOWWINDOWED
    if (info->full)
#endif
    {
        if (ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            msys_debugPrintf("ChangeDisplaySettings Fail\n");
            return(0);
        }
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        ShowCursor(0);
    }
#ifdef ALLOWWINDOWED
    else
    {
        dwExStyle = WS_EX_APPWINDOW;// | WS_EX_WINDOWEDGE;
        dwStyle = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    }
#endif
    rec.left = 0;
    rec.top = 0;
    rec.right = XRES;
    rec.bottom = YRES;

#ifdef ALLOWWINDOWED
    AdjustWindowRect(&rec, dwStyle, 0);
    info->hWnd = CreateWindowExA(dwExStyle, wc.lpszClassName, wc.lpszClassName, dwStyle,
        (GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
        (GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
        rec.right - rec.left, rec.bottom - rec.top, 0, 0, info->hInstance, 0);
#else
    info->hWnd = CreateWindowEx(dwExStyle, wc.lpszClassName, wc.lpszClassName, dwStyle, 0, 0,
        rec.right - rec.left, rec.bottom - rec.top, 0, 0, info->hInstance, 0);
#endif

    if (!info->hWnd)
    {
        msys_debugPrintf("hWnd Fail\n");
        return(0);
    }

    if (!(info->hDC = GetDC(info->hWnd)))
    {
        msys_debugPrintf("GetDC Fail\n");
        return(0);
    }

    if (!(PixelFormat = ChoosePixelFormat(info->hDC, &pfd)))
    {
        msys_debugPrintf("ChoosePixelFormat Fail\n");
        return(0);
    }

    if (!SetPixelFormat(info->hDC, PixelFormat, &pfd))
    {
        msys_debugPrintf("SetPixelFormat Fail\n");
        return(0);
    }

    if (!(info->hRC = wglCreateContext(info->hDC)))
    {
        msys_debugPrintf("wglCreateContext Fail\n");
        return(0);
    }

    if (!wglMakeCurrent(info->hDC, info->hRC))
    {
        msys_debugPrintf("wglMakeCurrent Fail\n");
        return(0);
    }

    //////////////////////////////
    #define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    static const int gl33_attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    auto wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");

    auto wglCreateContextAttribs = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribs");
    if (!(info->hRC = wglCreateContextAttribsARB(info->hDC, nullptr, gl33_attribs)))
        return(0);

    if( !wglMakeCurrent(info->hDC,info->hRC) )
        return( 0 );

    //////////////////////////////

    typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
    PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
    //PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

    // Extension is supported, init pointers.
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(1);
    }

    //////////////////////////////

    SetForegroundWindow(info->hWnd);
    SetFocus(info->hWnd);

    return(1);
}

//----------------------------------------------------------------------------
#if 0
extern "C" extern int __cdecl _heap_init(int);
extern "C" extern int __cdecl _mtinit(void);
extern "C" _CRTIMP int __cdecl _CrtSetCheckCount(int);
extern "C" extern int __cdecl _ioinit(void);
extern "C" extern int __cdecl _cinit(int);

/*
extern "C" extern int _heap_init(int);
extern "C" extern void _ioinit(void);
extern "C" extern void _cinit(void);

extern "C" extern void _mtinit(void);
*/

#include <rtcapi.h>
extern "C" extern void _RTC_Initialize(void);

int __cdecl MyErrorFunc(int, const wchar_t*, int, const wchar_t*, const wchar_t*, ...)
{
    MessageBox(0, "q", 0, 0);
    return 0;
}

/*
// C version:
_RTC_error_fnW __cdecl _CRT_RTC_INITW(void *res0, void **res1, int res2, int res3, int res4)
{
    return &MyErrorFunc;
}
*/

// C++ version:
extern "C" _RTC_error_fnW __cdecl _CRT_RTC_INITW(void* res0, void** res1, int res2, int res3, int res4)
{
    return &MyErrorFunc;
}

#include <winbase.h>

// RunTmChk.lib
#endif

//int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
void entrypoint()
{
    msys_debugInit();
    msys_debugPrintf("Starting\n");
#ifdef ALLOWWINDOWED
    wininfo.full = false;// (MessageBoxA(0, "fullscreen?", wndclass, MB_YESNO | MB_ICONQUESTION) == IDYES);
#endif
    if (!window_init(&wininfo))
    {
        window_end(&wininfo);
        //MessageBoxA(0, msg_error, 0, MB_OK | MB_ICONEXCLAMATION);
        msys_debugPrintf("exit A\n");
        //ExitProcess(0);
    }

    if (!msys_init((intptr)wininfo.hWnd))
    {
        window_end(&wininfo);
        msys_debugPrintf("Exit B\n");
        //MessageBoxA(0, msg_error, 0, MB_OK | MB_ICONEXCLAMATION);
        //ExitProcess(0);
    }

    if (!intro_init())
    {
        window_end(&wininfo);
        //MessageBoxA(0, msg_error, 0, MB_OK | MB_ICONEXCLAMATION);
        msys_debugPrintf("Exit C\n");
        //ExitProcess(0);
    }
    glViewport(0, 0, 1920, 1080);

    //msys_debugPrintf("Beginning loop\n");
    while (!done)
    {
        PumpMessages();
        done |= intro_do();
        SwapBuffers(wininfo.hDC);
    }
    //msys_debugPrintf("Edit loop\n");

    window_end(&wininfo);

    msys_end();

    ExitProcess(0);
    //return 0;
}

/*

extern "C" float msys_log2f(const float x)
{
    return logf(x) / logf(2.0f);
}

extern "C" float msys_expf(const float x)
{
    return expf(x);
}

extern "C" float msys_fmodf(const float x, const float y)
{
    return fmodf(x, y);
}

extern "C" float msys_powf(const float x, const float y)
{
    return powf(x, y);
}

extern "C" int msys_ifloorf(const float x)
{
    return (int)floorf(x);
}
*/

#ifdef _M_IX86 // use this file only for 32-bit architecture

#define CRT_LOWORD(x) dword ptr [x+0]
#define CRT_HIWORD(x) dword ptr [x+4]

extern "C"
{
    __declspec(naked) void _ftol2()
    {
        __asm
        {
            fistp qword ptr[esp - 8]
            mov   edx, [esp - 4]
            mov   eax, [esp - 8]
            ret
        }
    }

    __declspec(naked) void _ftol2_sse()
    {
        __asm
        {
            fistp dword ptr[esp - 4]
            mov   eax, [esp - 4]
            ret
        }
    }
    __declspec(naked) void _alldiv()
    {
#define DVND    esp + 16      // stack address of dividend (a)
#define DVSR    esp + 24      // stack address of divisor (b)

        __asm
        {
            push    edi
            push    esi
            push    ebx

            ; Determine sign of the result(edi = 0 if result is positive, non - zero
            ; otherwise) and make operands positive.

            xor edi, edi; result sign assumed positive

            mov     eax, CRT_HIWORD(DVND); hi word of a
            or eax, eax; test to see if signed
            jge     short L1; skip rest if a is already positive
            inc     edi; complement result sign flag
            mov     edx, CRT_LOWORD(DVND); lo word of a
            neg     eax; make a positive
            neg     edx
            sbb     eax, 0
            mov     CRT_HIWORD(DVND), eax; save positive value
            mov     CRT_LOWORD(DVND), edx
            L1 :
            mov     eax, CRT_HIWORD(DVSR); hi word of b
                or eax, eax; test to see if signed
                jge     short L2; skip rest if b is already positive
                inc     edi; complement the result sign flag
                mov     edx, CRT_LOWORD(DVSR); lo word of a
                neg     eax; make b positive
                neg     edx
                sbb     eax, 0
                mov     CRT_HIWORD(DVSR), eax; save positive value
                mov     CRT_LOWORD(DVSR), edx
                L2 :

            ;
            ; Now do the divide.First look to see if the divisor is less than 4194304K.
                ; If so, then we can use a simple algorithm with word divides, otherwise
                ; things get a little more complex.
                ;
            ; NOTE - eax currently contains the high order word of DVSR
                ;

            or eax, eax; check to see if divisor < 4194304K
                jnz     short L3; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; eax < -high order bits of quotient
                mov     ebx, eax; save high bits of quotient
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; eax < -low order bits of quotient
                mov     edx, ebx; edx:eax < -quotient
                jmp     short L4; set sign, restore stackand return

                ;
            ; Here we do it the hard way.Remember, eax contains the high word of DVSR
                ;

        L3:
            mov     ebx, eax; ebx:ecx < -divisor
                mov     ecx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L5 :
                shr     ebx, 1; shift divisor right one bit
                rcr     ecx, 1
                shr     edx, 1; shift dividend right one bit
                rcr     eax, 1
                or ebx, ebx
                jnz     short L5; loop until divisor < 4194304K
                div     ecx; now divide, ignore remainder
                mov     esi, eax; save quotient

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mul     CRT_HIWORD(DVSR); QUOT* CRT_HIWORD(DVSR)
                mov     ecx, eax
                mov     eax, CRT_LOWORD(DVSR)
                mul     esi; QUOT* CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L6; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
                ; subtract one(1) from the quotient.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L6; if result > original, do subtract
                jb      short L7; if result < original, we are ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L7; if less or equal we are ok, else subtract
                L6 :
            dec     esi; subtract 1 from quotient
                L7 :
            xor edx, edx; edx:eax < -quotient
                mov     eax, esi

                ;
            ; Just the cleanup left to do.edx:eax contains the quotient.Set the sign
                ; according to the save value, cleanup the stack, and return.
                ;

        L4:
            dec     edi; check to see if result is negative
                jnz     short L8; if EDI == 0, result should be negative
                neg     edx; otherwise, negate the result
                neg     eax
                sbb     edx, 0

                ;
            ; Restore the saved registersand return.
                ;

        L8:
            pop     ebx
                pop     esi
                pop     edi

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _alldvrm()
    {
#define DVND    esp + 16      // stack address of dividend (a)
#define DVSR    esp + 24      // stack address of divisor (b)

        __asm
        {
            push    edi
            push    esi
            push    ebp

            ; Determine sign of the quotient(edi = 0 if result is positive, non - zero
            ; otherwise) and make operands positive.
            ; Sign of the remainder is kept in ebp.

            xor edi, edi; result sign assumed positive
            xor ebp, ebp; result sign assumed positive

            mov     eax, CRT_HIWORD(DVND); hi word of a
            or eax, eax; test to see if signed
            jge     short L1; skip rest if a is already positive
            inc     edi; complement result sign flag
            inc     ebp; complement result sign flag
            mov     edx, CRT_LOWORD(DVND); lo word of a
            neg     eax; make a positive
            neg     edx
            sbb     eax, 0
            mov     CRT_HIWORD(DVND), eax; save positive value
            mov     CRT_LOWORD(DVND), edx
            L1 :
            mov     eax, CRT_HIWORD(DVSR); hi word of b
                or eax, eax; test to see if signed
                jge     short L2; skip rest if b is already positive
                inc     edi; complement the result sign flag
                mov     edx, CRT_LOWORD(DVSR); lo word of a
                neg     eax; make b positive
                neg     edx
                sbb     eax, 0
                mov     CRT_HIWORD(DVSR), eax; save positive value
                mov     CRT_LOWORD(DVSR), edx
                L2 :

            ;
            ; Now do the divide.First look to see if the divisor is less than 4194304K.
                ; If so, then we can use a simple algorithm with word divides, otherwise
                ; things get a little more complex.
                ;
            ; NOTE - eax currently contains the high order word of DVSR
                ;

            or eax, eax; check to see if divisor < 4194304K
                jnz     short L3; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; eax < -high order bits of quotient
                mov     ebx, eax; save high bits of quotient
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; eax < -low order bits of quotient
                mov     esi, eax; ebx:esi < -quotient
                ;
            ; Now we need to do a multiply so that we can compute the remainder.
                ;
            mov     eax, ebx; set up high word of quotient
                mul     CRT_LOWORD(DVSR); CRT_HIWORD(QUOT)* DVSR
                mov     ecx, eax; save the result in ecx
                mov     eax, esi; set up low word of quotient
                mul     CRT_LOWORD(DVSR); CRT_LOWORD(QUOT)* DVSR
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jmp     short L4; complete remainder calculation

                ;
            ; Here we do it the hard way.Remember, eax contains the high word of DVSR
                ;

        L3:
            mov     ebx, eax; ebx:ecx < -divisor
                mov     ecx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L5 :
                shr     ebx, 1; shift divisor right one bit
                rcr     ecx, 1
                shr     edx, 1; shift dividend right one bit
                rcr     eax, 1
                or ebx, ebx
                jnz     short L5; loop until divisor < 4194304K
                div     ecx; now divide, ignore remainder
                mov     esi, eax; save quotient

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mul     CRT_HIWORD(DVSR); QUOT* CRT_HIWORD(DVSR)
                mov     ecx, eax
                mov     eax, CRT_LOWORD(DVSR)
                mul     esi; QUOT* CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L6; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
                ; subtract one(1) from the quotient.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L6; if result > original, do subtract
                jb      short L7; if result < original, we are ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L7; if less or equal we are ok, else subtract
                L6 :
            dec     esi; subtract 1 from quotient
                sub     eax, CRT_LOWORD(DVSR); subtract divisor from result
                sbb     edx, CRT_HIWORD(DVSR)
                L7:
            xor ebx, ebx; ebx:esi < -quotient

                L4 :
            ;
            ; Calculate remainder by subtracting the result from the original dividend.
                ; Since the result is already in a register, we will do the subtract in the
                ; opposite directionand negate the result if necessary.
                ;

            sub     eax, CRT_LOWORD(DVND); subtract dividend from result
                sbb     edx, CRT_HIWORD(DVND)

                ;
            ; Now check the result sign flag to see if the result is supposed to be positive
                ; or negative.It is currently negated(because we subtracted in the 'wrong'
                    ; direction), so if the sign flag is set we are done, otherwise we must negate
                ; the result to make it positive again.
                ;

            dec     ebp; check result sign flag
                jns     short L9; result is ok, set up the quotient
                neg     edx; otherwise, negate the result
                neg     eax
                sbb     edx, 0

                ;
            ; Now we need to get the quotient into edx : eax and the remainder into ebx : ecx.
                ;
        L9:
            mov     ecx, edx
                mov     edx, ebx
                mov     ebx, ecx
                mov     ecx, eax
                mov     eax, esi

                ;
            ; Just the cleanup left to do.edx:eax contains the quotient.Set the sign
                ; according to the save value, cleanup the stack, and return.
                ;

            dec     edi; check to see if result is negative
                jnz     short L8; if EDI == 0, result should be negative
                neg     edx; otherwise, negate the result
                neg     eax
                sbb     edx, 0

                ;
            ; Restore the saved registersand return.
                ;

        L8:
            pop     ebp
                pop     esi
                pop     edi

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _allmul()
    {
#define A       esp + 8       // stack address of a
#define B       esp + 16      // stack address of b

        __asm
        {
            push    ebx

            mov     eax, CRT_HIWORD(A)
            mov     ecx, CRT_LOWORD(B)
            mul     ecx; eax has AHI, ecx has BLO, so AHI* BLO
            mov     ebx, eax; save result

            mov     eax, CRT_LOWORD(A)
            mul     CRT_HIWORD(B); ALO* BHI
            add     ebx, eax; ebx = ((ALO * BHI) + (AHI * BLO))

            mov     eax, CRT_LOWORD(A); ecx = BLO
            mul     ecx; so edx : eax = ALO * BLO
            add     edx, ebx; now edx has all the LO* HI stuff

            pop     ebx

            ret     16; callee restores the stack
        }

#undef A
#undef B
    }

    __declspec(naked) void _allrem()
    {
#define DVND    esp + 12      // stack address of dividend (a)
#define DVSR    esp + 20      // stack address of divisor (b)

        __asm
        {
            push    ebx
            push    edi


            ; Determine sign of the result(edi = 0 if result is positive, non - zero
            ; otherwise) and make operands positive.

            xor edi, edi; result sign assumed positive

            mov     eax, CRT_HIWORD(DVND); hi word of a
            or eax, eax; test to see if signed
            jge     short L1; skip rest if a is already positive
            inc     edi; complement result sign flag bit
            mov     edx, CRT_LOWORD(DVND); lo word of a
            neg     eax; make a positive
            neg     edx
            sbb     eax, 0
            mov     CRT_HIWORD(DVND), eax; save positive value
            mov     CRT_LOWORD(DVND), edx
            L1 :
            mov     eax, CRT_HIWORD(DVSR); hi word of b
                or eax, eax; test to see if signed
                jge     short L2; skip rest if b is already positive
                mov     edx, CRT_LOWORD(DVSR); lo word of b
                neg     eax; make b positive
                neg     edx
                sbb     eax, 0
                mov     CRT_HIWORD(DVSR), eax; save positive value
                mov     CRT_LOWORD(DVSR), edx
                L2 :

            ;
            ; Now do the divide.First look to see if the divisor is less than 4194304K.
                ; If so, then we can use a simple algorithm with word divides, otherwise
                ; things get a little more complex.
                ;
            ; NOTE - eax currently contains the high order word of DVSR
                ;

            or eax, eax; check to see if divisor < 4194304K
                jnz     short L3; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; edx < -remainder
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; edx < -final remainder
                mov     eax, edx; edx:eax < -remainder
                xor edx, edx
                dec     edi; check result sign flag
                jns     short L4; negate result, restore stackand return
                jmp     short L8; result sign ok, restore stackand return

                ;
            ; Here we do it the hard way.Remember, eax contains the high word of DVSR
                ;

        L3:
            mov     ebx, eax; ebx:ecx < -divisor
                mov     ecx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L5 :
                shr     ebx, 1; shift divisor right one bit
                rcr     ecx, 1
                shr     edx, 1; shift dividend right one bit
                rcr     eax, 1
                or ebx, ebx
                jnz     short L5; loop until divisor < 4194304K
                div     ecx; now divide, ignore remainder

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mov     ecx, eax; save a copy of quotient in ECX
                mul     CRT_HIWORD(DVSR)
                xchg    ecx, eax; save product, get quotient in EAX
                mul     CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L6; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
                ; subtract the original divisor from the result.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L6; if result > original, do subtract
                jb      short L7; if result < original, we are ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L7; if less or equal we are ok, else subtract
                L6 :
            sub     eax, CRT_LOWORD(DVSR); subtract divisor from result
                sbb     edx, CRT_HIWORD(DVSR)
                L7:

            ;
            ; Calculate remainder by subtracting the result from the original dividend.
                ; Since the result is already in a register, we will do the subtract in the
                ; opposite directionand negate the result if necessary.
                ;

            sub     eax, CRT_LOWORD(DVND); subtract dividend from result
                sbb     edx, CRT_HIWORD(DVND)

                ;
            ; Now check the result sign flag to see if the result is supposed to be positive
                ; or negative.It is currently negated(because we subtracted in the 'wrong'
                    ; direction), so if the sign flag is set we are done, otherwise we must negate
                ; the result to make it positive again.
                ;

            dec     edi; check result sign flag
                jns     short L8; result is ok, restore stackand return
                L4:
            neg     edx; otherwise, negate the result
                neg     eax
                sbb     edx, 0

                ;
            ; Just the cleanup left to do.edx:eax contains the quotient.
                ; Restore the saved registersand return.
                ;

        L8:
            pop     edi
                pop     ebx

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _allshl()
    {
        __asm
        {
            ;
            ; Handle shifts of 64 or more bits(all get 0)
                ;
            cmp     cl, 64
                jae     short RETZERO

                ;
            ; Handle shifts of between 0 and 31 bits
                ;
            cmp     cl, 32
                jae     short MORE32
                shld    edx, eax, cl
                shl     eax, cl
                ret

                ;
            ; Handle shifts of between 32 and 63 bits
                ;
        MORE32:
            mov     edx, eax
                xor eax, eax
                and cl, 31
                shl     edx, cl
                ret

                ;
            ; return 0 in edx : eax
                ;
        RETZERO:
            xor eax, eax
                xor edx, edx
                ret
        }
    }

    __declspec(naked) void _allshr()
    {
        __asm
        {
            ;
            ; Handle shifts of 64 bits or more(if shifting 64 bits or more, the result
                ; depends only on the high order bit of edx).
                ;
            cmp     cl, 64
                jae     short RETSIGN

                ;
            ; Handle shifts of between 0 and 31 bits
                ;
            cmp     cl, 32
                jae     short MORE32
                shrd    eax, edx, cl
                sar     edx, cl
                ret

                ;
            ; Handle shifts of between 32 and 63 bits
                ;
        MORE32:
            mov     eax, edx
                sar     edx, 31
                and cl, 31
                sar     eax, cl
                ret

                ;
            ; Return double precision 0 or -1, depending on the sign of edx
                ;
        RETSIGN:
            sar     edx, 31
                mov     eax, edx
                ret
        }
    }

    __declspec(naked) void _aulldiv()
    {
#define DVND    esp + 12      // stack address of dividend (a)
#define DVSR    esp + 20      // stack address of divisor (b)

        __asm
        {
            push    ebx
            push    esi

            ;
            ; Now do the divide.First look to see if the divisor is less than 4194304K.
                ; If so, then we can use a simple algorithm with word divides, otherwise
                ; things get a little more complex.
                ;

            mov     eax, CRT_HIWORD(DVSR); check to see if divisor < 4194304K
                or eax, eax
                jnz     short L1; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; get high order bits of quotient
                mov     ebx, eax; save high bits of quotient
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; get low order bits of quotient
                mov     edx, ebx; edx:eax < -quotient hi : quotient lo
                jmp     short L2; restore stackand return

                ;
            ; Here we do it the hard way.Remember, eax contains DVSRHI
                ;

        L1:
            mov     ecx, eax; ecx:ebx < -divisor
                mov     ebx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L3 :
                shr     ecx, 1; shift divisor right one bit; hi bit < -0
                rcr     ebx, 1
                shr     edx, 1; shift dividend right one bit; hi bit < -0
                rcr     eax, 1
                or ecx, ecx
                jnz     short L3; loop until divisor < 4194304K
                div     ebx; now divide, ignore remainder
                mov     esi, eax; save quotient

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mul     CRT_HIWORD(DVSR); QUOT* CRT_HIWORD(DVSR)
                mov     ecx, eax
                mov     eax, CRT_LOWORD(DVSR)
                mul     esi; QUOT* CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L4; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
                ; subtract one(1) from the quotient.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L4; if result > original, do subtract
                jb      short L5; if result < original, we are ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L5; if less or equal we are ok, else subtract
                L4 :
            dec     esi; subtract 1 from quotient
                L5 :
            xor edx, edx; edx:eax < -quotient
                mov     eax, esi

                ;
            ; Just the cleanup left to do.edx:eax contains the quotient.
                ; Restore the saved registersand return.
                ;

        L2:

            pop     esi
                pop     ebx

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _aulldvrm()
    {
#define DVND    esp + 8       // stack address of dividend (a)
#define DVSR    esp + 16      // stack address of divisor (b)

        __asm
        {
            push    esi

            ;
            ; Now do the divide.First look to see if the divisor is less than 4194304K.
                ; If so, then we can use a simple algorithm with word divides, otherwise
                ; things get a little more complex.
                ;

            mov     eax, CRT_HIWORD(DVSR); check to see if divisor < 4194304K
                or eax, eax
                jnz     short L1; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; get high order bits of quotient
                mov     ebx, eax; save high bits of quotient
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; get low order bits of quotient
                mov     esi, eax; ebx:esi < -quotient

                ;
            ; Now we need to do a multiply so that we can compute the remainder.
                ;
            mov     eax, ebx; set up high word of quotient
                mul     CRT_LOWORD(DVSR); CRT_HIWORD(QUOT)* DVSR
                mov     ecx, eax; save the result in ecx
                mov     eax, esi; set up low word of quotient
                mul     CRT_LOWORD(DVSR); CRT_LOWORD(QUOT)* DVSR
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jmp     short L2; complete remainder calculation

                ;
            ; Here we do it the hard way.Remember, eax contains DVSRHI
                ;

        L1:
            mov     ecx, eax; ecx:ebx < -divisor
                mov     ebx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L3 :
                shr     ecx, 1; shift divisor right one bit; hi bit < -0
                rcr     ebx, 1
                shr     edx, 1; shift dividend right one bit; hi bit < -0
                rcr     eax, 1
                or ecx, ecx
                jnz     short L3; loop until divisor < 4194304K
                div     ebx; now divide, ignore remainder
                mov     esi, eax; save quotient

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mul     CRT_HIWORD(DVSR); QUOT* CRT_HIWORD(DVSR)
                mov     ecx, eax
                mov     eax, CRT_LOWORD(DVSR)
                mul     esi; QUOT* CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L4; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
                ; subtract one(1) from the quotient.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L4; if result > original, do subtract
                jb      short L5; if result < original, we are ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L5; if less or equal we are ok, else subtract
                L4 :
            dec     esi; subtract 1 from quotient
                sub     eax, CRT_LOWORD(DVSR); subtract divisor from result
                sbb     edx, CRT_HIWORD(DVSR)
                L5:
            xor ebx, ebx; ebx:esi < -quotient

                L2 :
            ;
            ; Calculate remainder by subtracting the result from the original dividend.
                ; Since the result is already in a register, we will do the subtract in the
                ; opposite directionand negate the result.
                ;

            sub     eax, CRT_LOWORD(DVND); subtract dividend from result
                sbb     edx, CRT_HIWORD(DVND)
                neg     edx; otherwise, negate the result
                neg     eax
                sbb     edx, 0

                ;
            ; Now we need to get the quotient into edx : eax and the remainder into ebx : ecx.
                ;
            mov     ecx, edx
                mov     edx, ebx
                mov     ebx, ecx
                mov     ecx, eax
                mov     eax, esi
                ;
            ; Just the cleanup left to do.edx:eax contains the quotient.
                ; Restore the saved registersand return.
                ;

            pop     esi

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _aullrem()
    {
#define DVND    esp + 8       // stack address of dividend (a)
#define DVSR    esp + 16      // stack address of divisor (b)

        __asm
        {
            push    ebx

            ; Now do the divide.First look to see if the divisor is less than 4194304K.
            ; If so, then we can use a simple algorithm with word divides, otherwise
            ; things get a little more complex.
            ;

            mov     eax, CRT_HIWORD(DVSR); check to see if divisor < 4194304K
                or eax, eax
                jnz     short L1; nope, gotta do this the hard way
                mov     ecx, CRT_LOWORD(DVSR); load divisor
                mov     eax, CRT_HIWORD(DVND); load high word of dividend
                xor edx, edx
                div     ecx; edx < -remainder, eax < -quotient
                mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
                div     ecx; edx < -final remainder
                mov     eax, edx; edx:eax < -remainder
                xor edx, edx
                jmp     short L2; restore stackand return

                ;
            ; Here we do it the hard way.Remember, eax contains DVSRHI
                ;

        L1:
            mov     ecx, eax; ecx:ebx < -divisor
                mov     ebx, CRT_LOWORD(DVSR)
                mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
                mov     eax, CRT_LOWORD(DVND)
                L3 :
                shr     ecx, 1; shift divisor right one bit; hi bit < -0
                rcr     ebx, 1
                shr     edx, 1; shift dividend right one bit; hi bit < -0
                rcr     eax, 1
                or ecx, ecx
                jnz     short L3; loop until divisor < 4194304K
                div     ebx; now divide, ignore remainder

                ;
            ; We may be off by one, so to check, we will multiply the quotient
                ; by the divisorand check the result against the orignal dividend
                ; Note that we must also check for overflow, which can occur if the
                ; dividend is close to 2 * *64 and the quotient is off by 1.
                ;

            mov     ecx, eax; save a copy of quotient in ECX
                mul     CRT_HIWORD(DVSR)
                xchg    ecx, eax; put partial product in ECX, get quotient in EAX
                mul     CRT_LOWORD(DVSR)
                add     edx, ecx; EDX:EAX = QUOT * DVSR
                jc      short L4; carry means Quotient is off by 1

                ;
            ; do long compare here between original dividendand the result of the
                ; multiply in edx : eax.If original is larger or equal, we're ok, otherwise
                ; subtract the original divisor from the result.
                ;

            cmp     edx, CRT_HIWORD(DVND); compare hi words of resultand original
                ja      short L4; if result > original, do subtract
                jb      short L5; if result < original, we're ok
                cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
                jbe     short L5; if less or equal we're ok, else subtract
                L4:
            sub     eax, CRT_LOWORD(DVSR); subtract divisor from result
                sbb     edx, CRT_HIWORD(DVSR)
                L5:

            ;
            ; Calculate remainder by subtracting the result from the original dividend.
                ; Since the result is already in a register, we will perform the subtract in
                ; the opposite directionand negate the result to make it positive.
                ;

            sub     eax, CRT_LOWORD(DVND); subtract original dividend from result
                sbb     edx, CRT_HIWORD(DVND)
                neg     edx;and negate it
                neg     eax
                sbb     edx, 0

                ;
            ; Just the cleanup left to do.dx:ax contains the remainder.
                ; Restore the saved registersand return.
                ;

        L2:

            pop     ebx

                ret     16
        }

#undef DVND
#undef DVSR
    }

    __declspec(naked) void _aullshr()
    {
        __asm
        {
            cmp     cl, 64
            jae     short RETZERO

            ;
            ; Handle shifts of between 0 and 31 bits
                ;
            cmp     cl, 32
                jae     short MORE32
                shrd    eax, edx, cl
                shr     edx, cl
                ret

                ;
            ; Handle shifts of between 32 and 63 bits
                ;
        MORE32:
            mov     eax, edx
                xor edx, edx
                and cl, 31
                shr     eax, cl
                ret

                ;
            ; return 0 in edx : eax
                ;
        RETZERO:
            xor eax, eax
                xor edx, edx
                ret
        }
    }
}

#undef CRT_LOWORD
#undef CRT_HIWORD

#endif

/*
extern "C"
{
#pragma function(memset)
    void* memset(void* dest, int c, size_t count)
    {
        char* bytes = (char*)dest;
        while (count--)
        {
            *bytes++ = (char)c;
        }
        return dest;
    }

#pragma function(memcpy)
    void* memcpy(void* dest, const void* src, size_t count)
    {
        char* dest8 = (char*)dest;
        const char* src8 = (const char*)src;
        while (count--)
        {
            *dest8++ = *src8++;
        }
        return dest;
    }
}*/