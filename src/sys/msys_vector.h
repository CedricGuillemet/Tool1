#pragma once
#include "msys_assert.h"
#include <math.h>
#include <float.h>
#include "msys_libc.h"
#include <stdint.h>

struct matrix;

static const float PI = 3.141592f;
//static const float FLT_EPSILON = 0.00001f;

typedef struct vec3
{
    float x,y,z;
    void set(float v) { x = y = z = v; }
    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const;
    void lerp(const vec3& v, float t);
    constexpr vec3() = default;
    constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}
    vec3& operator = (const vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    
    vec3& operator += (const vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator -= (const vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    constexpr void cross(const vec3& v1, const vec3& v2)
    {
        x = v1.y * v2.z - v1.z * v2.y;
        y = v1.z * v2.x - v1.x * v2.z;
        z = v1.x * v2.y - v1.y * v2.x;
    }
    constexpr float dot(const vec3 &v) const
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    constexpr vec3& operator *= ( const vec3& v ) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    constexpr vec3& operator *= ( float v ) { x *= v;    y *= v;    z *= v; return *this; }
/*
    vec4 getVec4() const
    {
        return vec4( x, y, z, 0.f );
    }*/
    
    constexpr vec3 operator - () const;
    constexpr vec3 operator - ( const vec3& v ) const;
    constexpr vec3 operator + ( const vec3& v ) const;
    void normalize(vec3 source);
    vec3 normalize();
    
    void TransformVector(const matrix& matrix);
    void TransformPoint(const matrix& matrix);
    //void TransformVector(const vec4& v, const matrix& matrix ) { (*this) = v; this->TransformVector(matrix); }
    //void TransformPoint(const vec4& v, const matrix& matrix ) { (*this) = v; this->TransformPoint(matrix); }
    float& operator [] (int index) const { assert(index < 3); return ((float*)&x)[index]; }
    void isMaxOf(const vec3& v)
    {
        x = (v.x>x)?v.x:x;
        y = (v.y>y)?v.y:y;
        z = (v.z>z)?v.z:z;
    }
    void isMinOf(const vec3& v)
    {
        x = (v.x>x)?x:v.x;
        y = (v.y>y)?y:v.y;
        z = (v.z>z)?z:v.z;
    }
    int LongestAxis() const;
    /*
    void toLinear()
    {
        x = powf(x, 2.2f);
        y = powf(y, 2.2f);
        z = powf(z, 2.2f);
    }
    void toSrgb()
    {
        x = powf(x, 1.f/2.2f);
        y = powf(y, 1.f/2.2f);
        z = powf(z, 1.f/2.2f);
    }
    */
    constexpr uint32 ToUInt32() const { return /*((int)(w*255.f)<< 24) +*/ (static_cast<int>(z*255.f)<< 16) + (static_cast<int>(y*255.f)<< 8) + static_cast<int>(x*255.f); }
    constexpr void FromUInt32(uint32 v) { /*w = (float)( (v&0xFF000000) >> 24) * (1.f/255.f);*/ z = (float)( (v&0xFF0000) >> 16) * (1.f/255.f);
        y = (float)( (v&0xFF00) >> 8) * (1.f/255.f); x = (float)( (v&0xFF)) * (1.f/255.f); }
} vec3;

inline vec3 TransformPoint(const vec3& v, const matrix& matrix) { vec3 p(v); p.TransformPoint(matrix); return p; }
inline vec3 TransformVector(const vec3& v, const matrix& matrix) { vec3 p(v); p.TransformVector(matrix); return p; }

constexpr vec3 vec3::operator - () const { return vec3{-x, -y, -z}; }
constexpr vec3 vec3::operator - ( const vec3& v ) const { return vec3{x - v.x, y - v.y, z - v.z}; }
constexpr vec3 vec3::operator + ( const vec3& v ) const { return vec3{x + v.x, y + v.y, z + v.z}; }
constexpr vec3 operator * (const vec3 v, float f) { return {v.x * f, v.y * f, v.z * f}; }
float DistanceSq(vec3 a, vec3 b);
float Distance(vec3 a, vec3 b);
float Dot(vec3 a, vec3 b);
float Clamp(float v, float min, float max);
float Lerp(float a, float b, float t);
vec3 Lerp(vec3 a, vec3 b, float t);
vec3 Cross(vec3 a, vec3 b);
vec3 Normalized(vec3 value);
constexpr int SIGNBIT(float v) { return (v<0.f)?1:0; }

vec3 perpStark(vec3 u);

template<typename T> void AngularLimitedVector(T& vector, T normalizedAxis, float maxDot)
{
    float vectorLength = vector.length();
    T normalizedVector = vector * (1.f / vectorLength);
    maxDot = msys_fmaxf(maxDot, FLT_EPSILON);
    float dt = normalizedAxis.dot(normalizedVector);
    if (dt >= maxDot)
    {
        return; // OK!
    }
   /* if (dt < -FLT_EPSILON)
    {
        // When character moves too fast, some vector may get below axis
        // it's some kind of length limit here:
        vector = normalizedAxis * vector.length();
        return;
    }*/

    T shortestVectToAxis = normalizedVector - normalizedAxis * dt;
    if (shortestVectToAxis.lengthSq() <= FLT_EPSILON)
    {
        return; // OK!
    }
    shortestVectToAxis.normalize();

    T newNormalizedVector = normalizedAxis * maxDot + shortestVectToAxis * msys_sqrtf(1 - maxDot * maxDot);

    vector = newNormalizedVector * vectorLength;

    float sens = (vector.dot(normalizedAxis) < 0.f) ? -1.f : 1.f;
    vector *= sens;
}
//inline vec4::vec4(const vec3& v) : x(v.x), y(v.y), z(v.z), w(0.f) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
struct vec4
{
    float x,y,z,w;
    vec4() {}
    vec4(float x, float y, float z, float w = 0.f) : x(x), y(y), z(z), w(w) {}
    
    void set(float _x, float _y, float _z, float _w)
    {
        x = _x; y = _y; z = _z; w = _w;
    }
    void normalize()
    {
        float invlen = 1.f / (msys_sqrtf(x*x + y*y + z*z + w*w) + FLT_EPSILON);
        x *= invlen;
        y *= invlen;
        z *= invlen;
        w *= invlen;
    }
};
struct matrix
{
public:
    union
    {
        float m[4][4];
        float m16[16];
        struct
        {
            vec4 right, up, dir, position;
        };
    };

    matrix(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
    {
        m16[0] = v1;
        m16[1] = v2;
        m16[2] = v3;
        m16[3] = v4;
        m16[4] = v5;
        m16[5] = v6;
        m16[6] = v7;
        m16[7] = v8;
        m16[8] = v9;
        m16[9] = v10;
        m16[10] = v11;
        m16[11] = v12;
        m16[12] = v13;
        m16[13] = v14;
        m16[14] = v15;
        m16[15] = v16;
    }
    //~matrix() {};
    matrix(const matrix& other);
    matrix(const vec4 & r, const vec4 &u, const vec4& d, const vec4& p) { set(r, u, d, p); }
    matrix() {}
    void set(const vec4 & r, const vec4 &u, const vec4& d, const vec4& p) { right=r; up=u; dir=d; position=p; }
    void set(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
    {
        m16[0] = v1;
        m16[1] = v2;
        m16[2] = v3;
        m16[3] = v4;
        m16[4] = v5;
        m16[5] = v6;
        m16[6] = v7;
        m16[7] = v8;
        m16[8] = v9;
        m16[9] = v10;
        m16[10] = v11;
        m16[11] = v12;
        m16[12] = v13;
        m16[13] = v14;
        m16[14] = v15;
        m16[15] = v16;
    }
    static matrix GetIdentity() { return matrix(1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);
    }
    operator float * () { return m16; }
    operator const float* () const { return m16; }
    void translation(const vec3& position)
    {
        this->translation(position.x, position.y, position.z);
    }
    vec3 GetPosition() const
    {
        return vec3(position.x, position.y, position.z);
    }

    vec3 GetDirection() const
    {
        return vec3(dir.x, dir.y, dir.z);
    }
    vec3 GetUp() const
    {
        return vec3(up.x, up.y, up.z);
    }
    void translation(float _x, float _y, float _z) { this->translation( vec4(_x, _y, _z) ); }
    void rotationQuaternion(float x, float y, float z, float w);
    void translation(const vec4& vt);
    //void translationScale(const vec4& vt, const vec4& scale);
    void rotationY(const float angle );
    void rotationX(const float angle );

    void rotationZ(const float angle );
    void scale(float _s);
    void scale(float _x, float _y, float _z);
    //inline void scale(const vec4& s) { scale(s.x, s.y, s.z); }

    inline matrix& operator *= ( const matrix& mat )
    {
        matrix tmpMat;
        tmpMat = *this;
        tmpMat.Multiply(mat);
        *this = tmpMat;
        return *this;
    }
    inline matrix operator * (const matrix& mat) const
    {
        matrix matT;
        matT.Multiply(*this, mat);
        return matT;
    }

    void Multiply( const matrix& mat);

    void Multiply( const matrix &m1, const matrix &m2 );

    void glhPerspectivef2Rad(float fovyRad, float aspectRatio, float znear, float zfar, bool homogeneousNdc, bool rightHand = false);
    
    void glhFrustumf2(float x, float y, float width, float height,    float znear, float zfar, bool homogeneousNdc, bool rightHand = false);
    void OrthoOffCenterLH(const float l, float r, float b, const float t, float zn, const float zf );
    void lookAtRH(const vec3 &eye, const vec3 &at, const vec3 &up );
    void lookAtLH(const vec3 &eye, const vec3 &at, const vec3 &up );
    void LookAt(const vec3 &eye, const vec3 &at, const vec3 &up );
    //void rotationQuaternion( const vec4 &q );

    inline float GetDeterminant() const
    {
        return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] +    m[0][2] * m[1][0] * m[2][1] -
            m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] -    m[0][0] * m[1][2] * m[2][1];
    }

    float inverse(const matrix &srcMatrix, bool affine = false );
    float inverse(bool affine=false);
    void identity() {
        right.set(1.f, 0.f, 0.f, 0.f);
        up.set(0.f, 1.f, 0.f, 0.f);
        dir.set(0.f, 0.f, 1.f, 0.f);
        position.set(0.f, 0.f, 0.f, 1.f);
    }
    inline void transpose()
    {
        matrix tmpm;
        for (int l = 0; l < 4; l++)
        {
            for (int c = 0; c < 4; c++)
            {
                tmpm.m[l][c] = m[c][l];
            }
        }
        (*this) = tmpm;
    }
    void rotationAxis(const vec3 & axis, float angle );
    void rotationYawPitchRoll(const float yaw, const float pitch, const float roll );
    static matrix Identity;
};

void GenUnitCircle(vec3* position, int count);
float MinimumDistance(vec3 v, vec3 w, vec3 p, vec3& projection, float& t);
void Slerp(const vec4 *q1, const vec4 *q2, float t, vec4 *result);
//void Nlerp(const vec4 *q1, const vec4 *q2, float t, vec4 *result);
void matrixToQuaternion(const float *matrix, vec4 *quaternion);

#pragma pack(push, r1, 1)
struct CompressQuat {
    uint16_t sign    : 1;  // The sign of the largest component. 1 for negative.
    uint16_t sign_a  : 1;
    uint16_t a       : 14;
    
    uint16_t sign_b  : 1;
    uint16_t b       : 14;
    uint16_t sign_c  : 1;
    
    uint16_t c       : 14;
    uint16_t largest : 2;  // The largest component of the quaternion.
};
static_assert(sizeof(CompressQuat) == 6);
#pragma pack(pop, r1)

void compress_pack(const struct vec4 *_src, struct CompressQuat *out);
void uncompress_pack(const struct CompressQuat *_src, struct vec4 *out);

template<typename T> void msys_swap(T& A, T& B)
{
    T temp{ A };
    A = B;
    B = temp;
}

inline bool rayIntersectsAABB(const vec3& ray_origin, const vec3& ray_dir, const vec3& box_min, const vec3& box_max, vec3& intersection)
{
    vec3 inv_dir = {1.0f / ray_dir.x, 1.0f / ray_dir.y, 1.0f / ray_dir.z};

    float tmin = (box_min.x - ray_origin.x) * inv_dir.x;
    float tmax = (box_max.x - ray_origin.x) * inv_dir.x;
    if (tmin > tmax) msys_swap(tmin, tmax);

    float tymin = (box_min.y - ray_origin.y) * inv_dir.y;
    float tymax = (box_max.y - ray_origin.y) * inv_dir.y;
    if (tymin > tymax) msys_swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) return false;

    tmin = msys_fmaxf(tmin, tymin);
    tmax = msys_fminf(tmax, tymax);

    float tzmin = (box_min.z - ray_origin.z) * inv_dir.z;
    float tzmax = (box_max.z - ray_origin.z) * inv_dir.z;
    if (tzmin > tzmax) msys_swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) return false;

    tmin = msys_fmaxf(tmin, tzmin);
    tmax = msys_fminf(tmax, tzmax);

    // Calculate intersection point if intersection occurs
    if (tmin < 0) return false; // Intersection behind the ray's origin
    intersection = ray_origin + ray_dir * tmin;
    return true;
}
