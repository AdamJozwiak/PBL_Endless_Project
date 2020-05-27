#pragma once

#include <DirectXMath.h>
#include <math.h>

#include <vector>

class CPlane {
  public:
    void normalize() {
        float flScale = 1 / sqrt(normal.x * normal.x + normal.y * normal.y +
                                 normal.z * normal.z);
        normal.x *= flScale;
        normal.y *= flScale;
        normal.z *= flScale;
        d *= flScale;
    }

    DirectX::XMFLOAT3 normal;
    float d;
};