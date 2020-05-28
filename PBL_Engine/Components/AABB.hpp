#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <vector>

#include "ECS/Component.hpp"

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(AABB) {
    DirectX::XMVECTOR vertexMin;
    DirectX::XMVECTOR vertexMax;
    std::array<DirectX::XMFLOAT3, 8> vertices;
};

// ////////////////////////////////////////////////////////////////////////// //