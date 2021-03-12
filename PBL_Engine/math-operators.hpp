#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

// //////////////////////////////////////////////////////////////// Utilities //
inline DirectX::XMFLOAT3 operator+(DirectX::XMFLOAT3 const& a,
                                   DirectX::XMFLOAT3 const& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline DirectX::XMFLOAT3& operator+=(DirectX::XMFLOAT3& a,
                                     DirectX::XMFLOAT3 const& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline DirectX::XMFLOAT3 operator-(DirectX::XMFLOAT3 const& a,
                                   DirectX::XMFLOAT3 const& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline DirectX::XMFLOAT3& operator-=(DirectX::XMFLOAT3& a,
                                     DirectX::XMFLOAT3 const& b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline DirectX::XMFLOAT3 operator*(DirectX::XMFLOAT3 const& a, float const b) {
    return {a.x * b, a.y * b, a.z * b};
}

inline DirectX::XMFLOAT3& operator*=(DirectX::XMFLOAT3& a, float const b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

inline DirectX::XMFLOAT3 operator/(DirectX::XMFLOAT3 const& a, float const b) {
    return {a.x / b, a.y / b, a.z / b};
}

inline DirectX::XMFLOAT3& operator/=(DirectX::XMFLOAT3& a, float const b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
    return a;
}

inline float length(DirectX::XMFLOAT3 const& a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// ////////////////////////////////////////////////////////////////////////// //
