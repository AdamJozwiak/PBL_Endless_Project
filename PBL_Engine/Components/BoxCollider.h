#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <vector>

#include "ECS/Component.hpp"

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(BoxCollider) {
    DirectX::XMFLOAT3 size, center;
    DirectX::XMVECTOR boxColliderMin;
    DirectX::XMVECTOR boxColliderMax;
    std::vector<DirectX::XMFLOAT3> boxColliderVerts;
};

// ////////////////////////////////////////////////////////////////////////// //
