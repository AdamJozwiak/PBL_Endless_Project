#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <vector>

#include "ECS/Component.hpp"

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(BoxCollider) {
    float size_x;
    float size_y;
    float size_z;
    float center_x;
    float center_y;
    float center_z;

    DirectX::XMVECTOR boxColliderMin;
    DirectX::XMVECTOR boxColliderMax;
    std::vector<DirectX::XMFLOAT3> boxColliderVerts;
};

// ////////////////////////////////////////////////////////////////////////// //
