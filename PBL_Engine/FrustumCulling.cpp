#include "FrustumCulling.h"

CFrustum::CFrustum(const DirectX::XMFLOAT4X4& m) {
    p[FRUSTUM_RIGHT].normal.x = m.m[0][3] - m.m[0][0];
    p[FRUSTUM_RIGHT].normal.y = m.m[1][3] - m.m[1][0];
    p[FRUSTUM_RIGHT].normal.z = m.m[2][3] - m.m[2][0];
    p[FRUSTUM_RIGHT].d = m.m[3][3] - m.m[3][0];

    p[FRUSTUM_LEFT].normal.x = m.m[0][3] + m.m[0][0];
    p[FRUSTUM_LEFT].normal.y = m.m[1][3] + m.m[1][0];
    p[FRUSTUM_LEFT].normal.z = m.m[2][3] + m.m[2][0];
    p[FRUSTUM_LEFT].d = m.m[3][3] + m.m[3][0];

    p[FRUSTUM_DOWN].normal.x = m.m[0][3] + m.m[0][1];
    p[FRUSTUM_DOWN].normal.y = m.m[1][3] + m.m[1][1];
    p[FRUSTUM_DOWN].normal.z = m.m[2][3] + m.m[2][1];
    p[FRUSTUM_DOWN].d = m.m[3][3] + m.m[3][1];

    p[FRUSTUM_UP].normal.x = m.m[0][3] - m.m[0][1];
    p[FRUSTUM_UP].normal.y = m.m[1][3] - m.m[1][1];
    p[FRUSTUM_UP].normal.z = m.m[2][3] - m.m[2][1];
    p[FRUSTUM_UP].d = m.m[3][3] - m.m[3][1];

    p[FRUSTUM_FAR].normal.x = m.m[0][3] - m.m[0][2];
    p[FRUSTUM_FAR].normal.y = m.m[1][3] - m.m[1][2];
    p[FRUSTUM_FAR].normal.z = m.m[2][3] - m.m[2][2];
    p[FRUSTUM_FAR].d = m.m[3][3] - m.m[3][2];

    p[FRUSTUM_NEAR].normal.x = m.m[0][3] + m.m[0][2];
    p[FRUSTUM_NEAR].normal.y = m.m[1][3] + m.m[1][2];
    p[FRUSTUM_NEAR].normal.z = m.m[2][3] + m.m[2][2];
    p[FRUSTUM_NEAR].d = m.m[3][3] + m.m[3][2];

    for (int i = 0; i < 6; i++) p[i].normalize();
}

float dot(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

bool CFrustum::SphereIntersection(DirectX::XMFLOAT3 vecCenter, float flRadius) {
    for (int i = 0; i < 6; i++) {
        if (dot(vecCenter, p[i].normal) + p[i].d + flRadius <= 0) {
            return false;
        }
    }

    return true;
}
