#include "msys_vector.h"
#include "msys_libc.h"
#include "memory.h"
static const float epsilon = 0.00001f;
static const float DEG2RAD = 3.141592f / 180.f;

/*#ifdef WINDOWS
#define fabsf msys_fabsf
#define cosf msys_cosf
#define sinf msys_sinf
#else
*/
#include <math.h>
//#endif

/*
static void FPU_MatrixF_x_MatrixF(const float *a, const float *b, float *r)
{
    r[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
    r[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
    r[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
    r[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

    r[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
    r[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
    r[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
    r[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

    r[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
    r[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
    r[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
    r[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

    r[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
    r[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
    r[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
    r[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}*/

void FPU_MatrixF_x_MatrixF( float* __restrict mat1,  float* __restrict mat2,  float* __restrict res) {
    float* ptr = res;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            *ptr = 0.f;
            for (int k = 0; k < 4; k++)
            {
                *ptr += mat1[i * 4 + k] * mat2[k * 4 + j];
            }
            ptr++;
        }
    }
}

void vec3::normalize(vec3 source) { *this = source; this->normalize(); }
float vec3::length() const { return msys_sqrtf(lengthSq() + epsilon); }
vec3 vec3::normalize() { (*this) *= (1.f/length() + epsilon); return (*this); }

void vec3::lerp(const vec3& v, float t)
{
    for(int i = 0; i < 3; i++)
    {
        (*this)[i] += (v[i] - (*this)[i]) * t;
    }
    /*
    x += (v.x - x) * t;
    y += (v.y - y) * t;
    z += (v.z - z) * t;
     */
}

void vec3::TransformVector(const matrix& matrix )
{
    vec3 out;

    for(int i = 0; i < 3; i++)
    {
        out[i] = x * matrix.m[0][i] + y * matrix.m[1][i] + z * matrix.m[2][i] ;
    }
    /*
    out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] ;
    out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] ;
    out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] ;
*/
    x = out.x;
    y = out.y;
    z = out.z;
}

void vec3::TransformPoint(const matrix& matrix )
{
    vec3 out;

    /*
    out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] + matrix.m[3][0] ;
    out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] + matrix.m[3][1] ;
    out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] + matrix.m[3][2] ;
*/
    for(int i = 0; i < 3; i++)
    {
        out[i] = x * matrix.m[0][i] + y * matrix.m[1][i] + z * matrix.m[2][i] + matrix.m[3][i];
    }
    x = out.x;
    y = out.y;
    z = out.z;
}

float DistanceSq(vec3 a, vec3 b)
{
    vec3 d = b - a;
    return d.dot(d);
}

float Distance(vec3 a, vec3 b)
{
    vec3 d = b - a;
    return msys_sqrtf(d.dot(d));
}

float Dot(vec3 a, vec3 b)
{
    return a.dot(b);
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

vec3 Lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}
float Clamp(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

vec3 Cross(vec3 a, vec3 b)
{
    vec3 t;
    t.cross(a, b);
    return t;
}

vec3 Normalized(vec3 value)
{
    vec3 ret;
    ret.normalize(value);
    return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//matrix will receive the calculated perspective matrix.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.

void matrix::glhPerspectivef2Rad(float fovyRad, float aspectRatio,
    float znear, float zfar, bool homogeneousNdc, bool rightHand)
{
    /* const float height = 1.0f / msys_tanf(fovyRad * 0.5f);//fovyRad == 1
    const float width = height * 1.0f / aspectRatio;*/

    const float halfFovyRad = fovyRad * 0.5f;
    const float sinHalfFovy = msys_sinf(halfFovyRad);
    const float cosHalfFovy = msys_cosf(halfFovyRad);
    const float height = cosHalfFovy / sinHalfFovy;
    const float width = height / aspectRatio;

    glhFrustumf2(0.0f, 0.0f, width, height, znear, zfar, homogeneousNdc, rightHand);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void matrix::glhFrustumf2(float x, float y, float width, float height,
                                  float znear, float zfar, bool homogeneousNdc, bool rightHand)
{
    const float diff = zfar - znear;
    const float aa = homogeneousNdc ? (zfar + znear) / diff : zfar / diff;
    const float bb = homogeneousNdc ? (2.0f * zfar * znear) / diff : znear * aa;

    m16[0] = width;
    m16[1] = 0.f;
    m16[2] = 0.f;
    m16[3] = 0.f;
    m16[4] = 0.f;
    m16[5] = height;
    m16[6] = 0.f;
    m16[7] = 0.f;
    m16[8] = rightHand ? x : -x;
    m16[9] = rightHand ? y : -y;
    m16[10] = rightHand ? -aa : aa;
    m16[11] = rightHand ? -1.0f : 1.0f;
    m16[12] = 0.f;
    m16[13] = 0.f;
    m16[14] = -bb;
    m16[15] = 0.f;
}
#ifdef TOOLS
void matrix::lookAtRH(const vec3 &eye, const vec3 &at, const vec3 &up )
{
    vec3 X, Y, Z, tmp;
    
    Z.normalize(eye - at);
    Y.normalize(up);
    
    tmp.cross(Y, Z);
    X.normalize(tmp);
    
    tmp.cross(Z, X);
    Y.normalize(tmp);
    
    m[0][0] = X.x;
    m[0][1] = Y.x;
    m[0][2] = Z.x;
    m[0][3] = 0.0f;
    
    m[1][0] = X.y;
    m[1][1] = Y.y;
    m[1][2] = Z.y;
    m[1][3] = 0.0f;
    
    m[2][0] = X.z;
    m[2][1] = Y.z;
    m[2][2] = Z.z;
    m[2][3] = 0.0f;
    
    m[3][0] = -X.dot(eye);
    m[3][1] = -Y.dot(eye);
    m[3][2] = -Z.dot(eye);
    m[3][3] = 1.0f;
}
#endif

void matrix::lookAtLH(const vec3 &eye, const vec3 &at, const vec3 &up )
{
    vec3 X, Y, Z, tmp;

    Z.normalize(at - eye);
    Y.normalize(up);

    tmp.cross(Y, Z);
    X.normalize(tmp);

    tmp.cross(Z, X);
    Y.normalize(tmp);

    m[0][0] = X.x;
    m[0][1] = Y.x;
    m[0][2] = Z.x;
    m[0][3] = 0.0f;

    m[1][0] = X.y;
    m[1][1] = Y.y;
    m[1][2] = Z.y;
    m[1][3] = 0.0f;

    m[2][0] = X.z;
    m[2][1] = Y.z;
    m[2][2] = Z.z;
    m[2][3] = 0.0f;

    m[3][0] = -X.dot(eye);
    m[3][1] = -Y.dot(eye);
    m[3][2] = -Z.dot(eye);
    m[3][3] = 1.0f;
}

void matrix::LookAt(const vec3 &eye, const vec3 &at, const vec3 &up )
{
    vec3 X, Y, Z, tmp;

    Z.normalize(eye - at);
    Y.normalize(up);

    tmp.cross(Y, Z);
    X.normalize(tmp);

    tmp.cross(Z, X);
    Y.normalize(tmp);

    m[0][0] = X.x;
    m[0][1] = X.y;
    m[0][2] = X.z;
    m[0][3] = 0.0f;

    m[1][0] = Y.x;
    m[1][1] = Y.y;
    m[1][2] = Y.z;
    m[1][3] = 0.0f;

    m[2][0] = Z.x;
    m[2][1] = Z.y;
    m[2][2] = Z.z;
    m[2][3] = 0.0f;

    m[3][0] = eye.x;
    m[3][1] = eye.y;
    m[3][2] = eye.z;
    m[3][3] = 1.0f;
}

void matrix::OrthoOffCenterLH(const float l, float r, float b, const float t, float zn, const float zf )
{
    m[0][0] = 2 / (r-l);
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = 2 / (t-b);
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f / (zf - zn);
    m[2][3] = 0.0f;

    m[3][0] = (l+r)/(l-r);
    m[3][1] = (t+b)/(b-t);
    m[3][2] = zn / (zn - zf);
    m[3][3] = 1.0f;
}

float matrix::inverse(const matrix &srcMatrix, bool affine)
{
    *this = srcMatrix;
    return inverse(affine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

float determinant(float* mat)
{
    float det = 0;
    for (int i = 0; i < 4; i++)
    {
        float submat[3][3];
        for (int j = 1; j < 4; j++)
        {
            int col = 0;
            for (int k = 0; k < 4; k++)
            {
                if (k == i) continue;
                submat[j-1][col] = mat[j * 4 + k];
                col++;
            }
        }
        float subdet = submat[0][0] * (submat[1][1] * submat[2][2] - submat[1][2] * submat[2][1]) -
                       submat[0][1] * (submat[1][0] * submat[2][2] - submat[1][2] * submat[2][0]) +
                       submat[0][2] * (submat[1][0] * submat[2][1] - submat[1][1] * submat[2][0]);
        det += (i % 2 == 0 ? 1.f : -1.f) * mat[i] * subdet;
    }
    return det;
}

// Function to calculate the inverse of a 4x4 matrix
void _inverse(float* mat, float* inv) {
    
}

float matrix::inverse(bool affine)
{
    float res[16];
    
    float det = determinant(m16);
    if (det == 0.f) {
        return 0.f;
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            float submat[3][3];
            int row = 0;
            for (int k = 0; k < 4; k++)
            {
                if (k == i) continue;
                int col = 0;
                for (int l = 0; l < 4; l++)
                {
                    if (l == j) continue;
                    submat[row][col] = m16[k * 4 + l];
                    col++;
                }
                row++;
            }
            float subdet = submat[0][0] * (submat[1][1] * submat[2][2] - submat[1][2] * submat[2][1]) -
                           submat[0][1] * (submat[1][0] * submat[2][2] - submat[1][2] * submat[2][0]) +
                           submat[0][2] * (submat[1][0] * submat[2][1] - submat[1][1] * submat[2][0]);
            res[j * 4 + i] = ((i + j) % 2 == 0 ? 1.f : -1.f) * subdet / det;
        }
    }
    
    msys_memcpy(m16, res, sizeof(float) * 16);
    return det;
}


void matrix::rotationAxis(const vec3 & axis, float angle )
{
    /*float length2 = axis.lengthSq();
    if ( length2 < epsilon)
    {
        identity();
        return;
    }
*/
    vec3 n = axis;// * (1.f / sqrtf(length2));
    float s = msys_sinf(angle);
    float c = msys_cosf(angle);
    float k = 1.f - c;

    float xx = n.x * n.x * k + c;
    float yy = n.y * n.y * k + c;
    float zz = n.z * n.z * k + c;
    float xy = n.x * n.y * k;
    float yz = n.y * n.z * k;
    float zx = n.z * n.x * k;
    float xs = n.x * s;
    float ys = n.y * s;
    float zs = n.z * s;

    m[0][0] = xx;
    m[0][1] = xy + zs;
    m[0][2] = zx - ys;
    m[0][3] = 0.f;
    m[1][0] = xy - zs;
    m[1][1] = yy;
    m[1][2] = yz + xs;
    m[1][3] = 0.f;
    m[2][0] = zx + ys;
    m[2][1] = yz - xs;
    m[2][2] = zz;
    m[2][3] = 0.f;
    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = 0.f;
    m[3][3] = 1.f;
}

void matrix::rotationYawPitchRoll(const float yaw, const float pitch, const float roll )
{
    float cy = msys_cosf(yaw);
    float sy = msys_sinf(yaw);

    float cp = msys_cosf(pitch);
    float sp = msys_sinf(pitch);

    float cr = msys_cosf(roll);
    float sr = msys_sinf(roll);

    float spsy = sp * sy;
    float spcy = sp * cy;

    m[0][0] = cr * cp;
    m[0][1] = sr * cp;
    m[0][2] = -sp;
    m[0][3] = 0;
    m[1][0] = cr * spsy - sr * cy;
    m[1][1] = sr * spsy + cr * cy;
    m[1][2] = cp * sy;
    m[1][3] = 0;
    m[2][0] = cr * spcy + sr * sy;
    m[2][1] = sr * spcy - cr * sy;
    m[2][2] = cp * cy;
    m[2][3] = 0;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}

matrix::matrix(const matrix& other) { msys_memcpy(&m16[0], &other.m16[0], sizeof(float) * 16); }

void matrix::Multiply( const matrix &m1, const matrix &m2 )
{
    FPU_MatrixF_x_MatrixF( (float*)&m1, (float*)&m2, (float*)this);
}

void matrix::translation(const vec4& vt)
{
    right.set(1.f, 0.f, 0.f, 0.f);
    up.set(0.f, 1.f, 0.f, 0.f);
    dir.set(0.f, 0.f, 1.f, 0.f);
    position.set(vt.x, vt.y, vt.z, 1.f);
}

void matrix::rotationQuaternion(float x, float y, float z, float w)
{
    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float zw = z * w;
    const float zx = z * x;
    const float yw = y * w;
    const float yz = y * z;
    const float xw = x * w;

    m16[0] = 1.0f - 2.0f * (yy + zz);
    m16[1] = 2.0f * (xy + zw);
    m16[2] = 2.0f * (zx - yw);
    m16[3] = 0.0f;

    m16[4] = 2.0f * (xy - zw);
    m16[5] = 1.0f - 2.0f * (zz + xx);
    m16[6] = 2.0f * (yz + xw);
    m16[7] = 0.0f;

    m16[8] = 2.0f * (zx + yw);
    m16[9] = 2.0f * (yz - xw);
    m16[10] = 1.0f - 2.0f * (yy + xx);
    m16[11] = 0.0f;

    m16[12] = 0.0f;
    m16[13] = 0.0f;
    m16[14] = 0.0f;
    m16[15] = 1.0f;
}

void matrix::rotationY(const float angle )
{
    float c = msys_cosf(angle);
    float s = msys_sinf(angle);

    right.set(c, 0.f, -s, 0.f);
    up.set(0.f, 1.f, 0.f , 0.f);
    dir.set(s, 0.f, c , 0.f);
    position.set(0.f, 0.f, 0.f , 1.f);
}

void matrix::rotationX(const float angle )
{
    float c = msys_cosf(angle);
    float s = msys_sinf(angle);

    right.set(1.f, 0.f , 0.f, 0.f);
    up.set(0.f, c , s, 0.f);
    dir.set(0.f, -s, c, 0.f);
    position.set(0.f, 0.f , 0.f, 1.f);
}

void matrix::rotationZ(const float angle )
{
    float c = msys_cosf(angle);
    float s = msys_sinf(angle);

    right.set(c , s, 0.f, 0.f);
    up.set(-s, c, 0.f, 0.f);
    dir.set(0.f , 0.f, 1.f, 0.f);
    position.set(0.f , 0.f, 0, 1.f);
}
void matrix::scale(float _s)
{
    right.set(_s, 0.f, 0.f, 0.f);
    up.set(0.f, _s, 0.f, 0.f);
    dir.set(0.f, 0.f, _s, 0.f);
    position.set(0.f, 0.f, 0.f, 1.f);
}
void matrix::scale(float _x, float _y, float _z)
{
    right.set(_x, 0.f, 0.f, 0.f);
    up.set(0.f, _y, 0.f, 0.f);
    dir.set(0.f, 0.f, _z, 0.f);
    position.set(0.f, 0.f, 0.f, 1.f);
}

void matrix::Multiply( const matrix& mat)
{
    matrix tmp;
    tmp = *this;

    FPU_MatrixF_x_MatrixF( (float*)&tmp, (float*)&mat, (float*)this);
}

void GenUnitCircle(vec3* position, int count)
{
    const float step = PI * 2.f / float(count);
    for(int f = 0; f < count; f++)
    {
        float ng = f * step;
        position[f] = vec3(msys_cosf(ng), msys_sinf(ng), 0.f);
    }
}

float MinimumDistance(vec3 v, vec3 w, vec3 p, vec3& projection, float& t)
{
    // Return minimum Distance between line segment vw and point p
    const float l2 = Dot(v - w, v - w);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 < FLT_EPSILON) return Distance(p, v);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    t = Dot(p - v, w - v) / l2;
    if (t < FLT_EPSILON) return Distance(p, v);       // Beyond the 'v' end of the segment
    else if (t > 1.0-FLT_EPSILON) return Distance(p, w);  // Beyond the 'w' end of the segment
    projection = v + (w - v) * t;  // Projection falls on the segment
    return Distance(p, projection);
}
/*
void Nlerp(const vec4 *q1, const vec4 *q2, float t, vec4 *result)
{
    result->x = Lerp(q1->x, q2->x, t);
    result->y = Lerp(q1->y, q2->y, t);
    result->z = Lerp(q1->z, q2->z, t);
    result->w = Lerp(q1->w, q2->w, t);
    result->normalize();
}
*/
static volatile const float Infinity = INFINITY;
static volatile const float Tiny = 0x1p-126f;

static float PositiveTail(float x)
{
    if (1 <= x)
    {
        // If x is 1, return 0.
        if (1 == x)
            return 0;
        // If x is outside the domain, generate invalid and return NaN.
        else
            return Infinity - Infinity;
    }

    #if defined __i386__ || defined __x86_64__

        float a = 1-x;
        float ef;

        // Estimate 1/sqrt(1-x) with a relative error of at most 1.5*2**-12.
        __asm__("rsqrtss %[a], %[ef]" : [ef] "=x" (ef) : [a] "x" (a));

        // Refine the estimate using a minimax polynomial.
        double e = ef;
        double e1a = e*a;
        double e2a = e*e1a;
        double s = (e2a - 0x1.AAAAABC2AAAAFp1) * e2a + 0x1.3FFFFED400007p2;

        return (float)
((e1a                   * ((x + 0x1.5BF7EF31D03E9p1) * x + 0x1.D75F3135B1D17p3))
*(0x1.82BAABF9AAC5Ep-10 * ((x - 0x1.136F5A328AFC8p3) * x + 0x1.B17BE5D0DECD9p4))
*s);


    #else    // #if defined __i386__ || defined __x86_64__

        return static_cast<float>(
  (                       ((x + 0x1.5BF7EF31D03E9p1) * x + 0x1.D75F3135B1D17p3))
* (0x1.01D1C56316584p-8 * ((x - 0x1.136F5A328AFC8p3) * x + 0x1.B17BE5D0DECD9p4))
* msys_sqrtf(1.f-x));

    #endif    // #if defined __i386__ || defined __x86_64__
}


/*    Return arccosine(x) given that x < -.62, with the same properties as
    acosf.
*/
static float NegativeTail(float x)
{
    static const double Pi = 0x3.243f6a8885a308d313198a2e03707344ap0;

    if (x <= -1)
    {
        // If x is -1, generate inexact and return pi rounded down.
        if (-1 == x)
            return 0x1.921fb4p1f + Tiny;
        // If x is outside the domain, generate invalid and return NaN.
        else
            return Infinity - Infinity;
    }

    #if defined __i386__ || defined __x86_64__

        float a = 1+x;
        float ef;

        // Estimate 1/sqrt(1+x) with a relative error of at most 1.5*2**-12.
        __asm__("rsqrtss %[a], %[ef]" : [ef] "=x" (ef) : [a] "x" (a));

        // Refine the estimate using a minimax polynomial.
        double e = ef;
        double e1a = e*a;
        double e2a = e*e1a;
        double s = (e2a - 0x1.AAAAABC2AAAAFp1) * e2a + 0x1.3FFFFED400007p2;

        return (float) (Pi -
(e1a                   * ((x - 0x1.5BF7EF31D03E9p1) * x + 0x1.D75F3135B1D17p3))
*(0x1.82BAABF9AAC5Ep-10 * ((x + 0x1.136F5A328AFC8p3) * x + 0x1.B17BE5D0DECD9p4))
*s);


    #else    // #if defined __i386__ || defined __x86_64__

        return static_cast<float>(Pi -
  (                       ((x - 0x1.5BF7EF31D03E9p1) * x + 0x1.D75F3135B1D17p3))
* (0x1.01D1C56316584p-8 * ((x + 0x1.136F5A328AFC8p3) * x + 0x1.B17BE5D0DECD9p4))
* msys_sqrtf(1.f+x));

    #endif    // #if defined __i386__ || defined __x86_64__
}


// See documentation above.
float msys_acosf(float x)
{
    static const double HalfPi = 0x3.243f6a8885a308d313198a2e03707344ap-1;

    if (x < -.62f)
        return NegativeTail(x);
    else if (x <= +.62f)
    {
        // Square x.  (Convert to double first to avoid underflow.)
        double x2 = (double) x * x;

        return (float)(HalfPi - x -
(0x1.1F1B81164C324p-4 * x
    * ((x2 + 0x1.9249F14B97277p0) * x2 + 0x1.408A21C01FB5Ap0))
*    (x2 * ((x2 - 0x1.899ED21055CD3p0) * x2 + 0x1.E638836E9888Ep0)));
    }
    else
        return PositiveTail(x);
}
void Slerp(const vec4 *q1, const vec4 *q2, float t, vec4 *result)
{
    float dot = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w;

    float mulq2 = 1.f;
    if (dot < 0.0f) {
        // If the dot product is negative, the quaternions have opposite handedness.
        // In that case, negate q2 to move along the shortest path.
        mulq2 = -1.f;
        dot = -dot;
    }

    const float epsilon = 0.0001f;
    float t1, t2;

    // Avoid division by zero by slightly offsetting t in case of very small dot product
    if (dot > 1.0f - epsilon) {
        // Quaternions are very close, so linearly interpolate
        t1 = 1.0f - t;
        t2 = t;
    } else {
        float angle = msys_acosf(dot);
        //float sinAngle = msys_sinf(angle);
        float sinAngle = msys_sqrtf(1.f - dot * dot);
        t1 = msys_sinf((1.0f - t) * angle) / sinAngle;
        t2 = msys_sinf(t * angle) / sinAngle;
    }

    result->x = t1 * q1->x + t2 * q2->x * mulq2;
    result->y = t1 * q1->y + t2 * q2->y * mulq2;
    result->z = t1 * q1->z + t2 * q2->z * mulq2;
    result->w = t1 * q1->w + t2 * q2->w * mulq2;
}

void matrixToQuaternion(const float *matrix, vec4 *quaternion) {
    const float* data = matrix;//matrix->m16;
    const float m11 = data[0],
        m12 = data[4],
        m13 = data[8];
    const float m21 = data[1],
        m22 = data[5],
        m23 = data[9];
    const float m31 = data[2],
        m32 = data[6],
        m33 = data[10];
    const float trace = m11 + m22 + m33;
    float s;

    if (trace > 0.f) {
        s = 0.5f / msys_sqrtf(trace + 1.0f);

        quaternion->w = 0.25f / s;
        quaternion->x = (m32 - m23) * s;
        quaternion->y = (m13 - m31) * s;
        quaternion->z = (m21 - m12) * s;
    } else if (m11 > m22 && m11 > m33) {
        s = 2.0f * msys_sqrtf(1.0f + m11 - m22 - m33);

        quaternion->w = (m32 - m23) / s;
        quaternion->x = 0.25f * s;
        quaternion->y = (m12 + m21) / s;
        quaternion->z = (m13 + m31) / s;
    } else if (m22 > m33) {
        s = 2.0f * msys_sqrtf(1.0f + m22 - m11 - m33);

        quaternion->w = (m13 - m31) / s;
        quaternion->x = (m12 + m21) / s;
        quaternion->y = 0.25f * s;
        quaternion->z = (m23 + m32) / s;
    } else {
        s = 2.0f * msys_sqrtf(1.0f + m33 - m11 - m22);

        quaternion->w = (m21 - m12) / s;
        quaternion->x = (m13 + m31) / s;
        quaternion->y = (m23 + m32) / s;
        quaternion->z = 0.25f * s;
    }
}
static const int Mapping[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};

#ifdef TOOLS
void compress_pack(const struct vec4 *_src, struct CompressQuat *out) {
    const float quat[4] = {_src->x, _src->y, _src->z, _src->w};

    int largest = 0;
    for (int i = 1; i < 4; ++i)
    {
        if (msys_fabsf(quat[i]) > msys_fabsf(quat[largest]))
        {
            largest = i;
        }
    }
    
    out->largest = (uint16_t) (largest & 0x3);
    // Stores the sign of the largest component.
    out->sign = (uint16_t)(quat[largest] < 0.f ? 1 : 0);
    
    static const float Float2Int = 16383.f;// * cq_Sqrt2;
    const int *map = Mapping[largest];
    
    const int32_t a = (int32_t)(msys_floorf(quat[map[0]] * Float2Int + .5f));
    const int32_t b = (int32_t)(msys_floorf(quat[map[1]] * Float2Int + .5f));
    const int32_t c = (int32_t)(msys_floorf(quat[map[2]] * Float2Int + .5f));
    
    int16_t l_a = a;//(const int16_t) (clamp(-maxAbsRage, a, maxAbsRage));
    int16_t l_b = b;//(const int16_t) (clamp(-maxAbsRage, b, maxAbsRage));
    int16_t l_c = c;//(const int16_t) (clamp(-maxAbsRage, c, maxAbsRage));

    out->sign_a = (uint16_t)(l_a < 0 ? 1 : 0);
    out->sign_b = (uint16_t)(l_b < 0 ? 1 : 0);
    out->sign_c = (uint16_t)(l_c < 0 ? 1 : 0);
    
    l_a = abs(l_a);
    l_b = abs(l_b);
    l_c = abs(l_c);
    
    out->a = l_a;
    out->b = l_b;
    out->c = l_c;
}
#endif
void uncompress_pack(const struct CompressQuat *_src, struct vec4 *out)
{
    const float Int2Float = 1.f / (16383.f /* * cq_Sqrt2*/);

    size_t largest = _src->largest;
    float quat[4] = {0, 0, 0, 0};
    const int *map = Mapping[largest];

    quat[map[0]] =  _src->sign_a > 0 ? -(_src->a * Int2Float) : (_src->a * Int2Float);
    quat[map[1]] =  _src->sign_b > 0 ? -(_src->b * Int2Float) : (_src->b * Int2Float);
    quat[map[2]] =  _src->sign_c > 0 ? -(_src->c * Int2Float) : (_src->c * Int2Float);

    quat[largest] = msys_sqrtf(1.f - quat[map[0]] * quat[map[0]] - quat[map[1]] * quat[map[1]] - quat[map[2]] * quat[map[2]]);//
    quat[largest] *= _src->sign > 0 ? -1.0f : 1.0f;
    
    out->x =  quat[0];
    out->y =  quat[1];
    out->z =  quat[2];
    out->w =  quat[3];
}

vec3 perpStark(vec3 u)
{
    vec3 a = vec3(msys_fabsf(u.x), msys_fabsf(u.y), msys_fabsf(u.z));
    unsigned int uyx = SIGNBIT(a.x - a.y);
    unsigned int uzx = SIGNBIT(a.x - a.z);
    unsigned int uzy = SIGNBIT(a.y - a.z);

    unsigned int xm = uyx & uzx;
    unsigned int ym = (1^xm) & uzy;
    unsigned int zm = 1^(xm & ym);

    vec3 v = Cross( u, vec3( (float)xm, (float)ym, (float)zm ) );
    return v;
}
