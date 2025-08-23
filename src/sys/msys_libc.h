//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_LIBC_H_
#define _MSYS_LIBC_H_

#include "msys_types.h"

#ifndef WINDOWS

#include <string.h>
#include <math.h>
#define msys_sinf(a)       sinf(a)
#define msys_cosf(a)       cosf(a)
#define msys_sqrtf(a)      sqrtf(a)
#define msys_atanf(a)      atanf(a)
#define msys_atan2f(a,b)   atan2f(a,b)
#define msys_tanf(a)       tanf(a)
#define msys_fabsf(a)      fabsf(a)
#define msys_logf(a)       logf(a)
#define msys_log10f(a)     log10f(a)
#define msys_expf(a)       expf(a)
#define msys_memset(a,b,c) memset(a,b,c)
#define msys_memcpy(a,b,c) memcpy(a,b,c)
#define msys_strlen(a)     strlen(a)
#define msys_fmaxf(a,b)    fmaxf(a,b)
#define msys_fminf(a,b)    fminf(a,b)
#define msys_floorf(a)     floorf(a)
#define msys_powf(a,b)     powf(a,b)
#else


#ifdef A32BITS

#if 0
MSYS_INLINE float msys_sinf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fsin;
    _asm fstp dword ptr[r];
    return r;
}
MSYS_INLINE float msys_cosf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fcos;
    _asm fstp dword ptr[r];
    return r;
}
MSYS_INLINE float msys_sqrtf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fsqrt;
    _asm fstp dword ptr[r];
    return r;
}
MSYS_INLINE float msys_fabsf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fabs;
    _asm fstp dword ptr[r];
    return r;
}
MSYS_INLINE float msys_tanf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fptan;
    _asm fstp st(0)
    _asm fstp dword ptr[r];
    return r;
}
#else

#include <math.h>

#define msys_sinf(a)       sinf(a)
#define msys_cosf(a)       cosf(a)
#define msys_sqrtf(a)      sqrtf(a)
#define msys_fabsf(a)      fabsf(a)
#define msys_atanf(a)      atanf(a)
#define msys_atan2f(a,b)   atan2f(a,b)
#define msys_tanf(a)       tanf(a)

#endif
/*
MSYS_INLINE void msys_sincosf(float x, float* r) {
    _asm fld dword ptr[x];
    _asm fsincos;
    _asm fstp dword ptr[r + 0];
    _asm fstp dword ptr[r + 4];
}
float msys_log2f(const float x);
float msys_expf(const float x);
float msys_fmodf(const float x, const float y);
float msys_powf(const float x, const float y);
float msys_floorf(const float x);
int   msys_ifloorf(const float x);
*/
#if 0
void  msys_memset(void* dst, int val, int amount);
void  msys_memcpy(void* dst, const void* ori, int amount);
int   msys_strlen(const char* str);
#else
#define msys_memset(a,b,c) memset(a,b,c)
#define msys_memcpy(a,b,c) memcpy(a,b,c)
#define msys_strlen(a)     strlen(a)
#endif
#else
    // x64
extern "C" float msys_sinf(const float x);
extern "C" float msys_cosf(const float x);
extern "C" float msys_sqrtf(const float x);
extern "C" float msys_fabsf(const float x);
extern "C" float msys_tanf(const float x);
extern "C" void  msys_sincosf(float x, float* r);
extern "C" float msys_log2f(const float x);
extern "C" float msys_expf(const float x);
extern "C" float msys_fmodf(const float x, const float y);
extern "C" float msys_powf(const float x, const float y);
extern "C" float msys_floorf(const float x);
extern "C" int   msys_ifloorf(const float x);
void  msys_memset(void* dst, int val, int amount);
void  msys_memcpy(void* dst, const void* ori, int amount);
int   msys_strlen(const char* str);

MSYS_INLINE float msys_fmaxf(float a, float b)
{
    return (a > b) ? a : b;
}

MSYS_INLINE float msys_fminf(float a, float b)
{
    return (a < b) ? a : b;
}


MSYS_INLINE float msys_fabsf(float v)
{
    return (v < 0.f) ? -v : v;
}

MSYS_INLINE int msys_ftoi(float x)
{
    /*int n;
    __asm {
        fld x
        fisttp n // the extra 't' means truncate
    }
    return n;*/
    return static_cast<int>(x);
}

MSYS_INLINE float msys_powf(float base, float exp) {
    if (exp < 0.001f) {
        return 1.0f;  // any number raised to the power of 0 is 1
    }
    
    int intExp = (int) exp;  // Handle only integer exponents for simplicity
    float result = 1.0f;
    int positiveExp = intExp > 0 ? intExp : -intExp; // Handle negative exponents

    for (int i = 0; i < positiveExp; i++) {
        result *= base;  // Multiply base exp times
    }
    
    // If the exponent was negative, return 1/result
    if (intExp < 0) {
        result = 1.0f / result;
    }

    return result;
}
#endif



#endif

#endif
