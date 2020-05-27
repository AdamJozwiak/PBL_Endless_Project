#pragma once
#include <DirectXMath.h>

#include "CPlane.h"

#define FRUSTUM_NEAR 0
#define FRUSTUM_FAR 1
#define FRUSTUM_LEFT 2
#define FRUSTUM_RIGHT 3
#define FRUSTUM_UP 4
#define FRUSTUM_DOWN 5

class CFrustum {
  public:
    CFrustum(){};
    CFrustum(const DirectX::XMFLOAT4X4& m);

  public:
    bool SphereIntersection(DirectX::XMFLOAT3 vecCenter, float flRadius);

  public:
    CPlane p[6];
};
