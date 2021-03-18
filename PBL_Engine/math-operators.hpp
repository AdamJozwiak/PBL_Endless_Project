#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <cmath>

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

template <typename T>
inline T lerp(T const& a, T const& b, float const t) {
    return std::lerp(a, b, t);
}
template <>
inline DirectX::XMFLOAT3 lerp<DirectX::XMFLOAT3>(DirectX::XMFLOAT3 const& a,
                                                 DirectX::XMFLOAT3 const& b,
                                                 float const t) {
    return DirectX::XMFLOAT3(lerp(a.x, b.x, t), lerp(a.y, b.y, t),
                             lerp(a.z, b.z, t));
}
template <>
inline DirectX::XMFLOAT4 lerp<DirectX::XMFLOAT4>(DirectX::XMFLOAT4 const& a,
                                                 DirectX::XMFLOAT4 const& b,
                                                 float const t) {
    return DirectX::XMFLOAT4(lerp(a.x, b.x, t), lerp(a.y, b.y, t),
                             lerp(a.z, b.z, t), lerp(a.w, b.w, t));
}

inline float length(DirectX::XMFLOAT3 const& a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// ////////////////////////////////////////////////////////////////////////// //
