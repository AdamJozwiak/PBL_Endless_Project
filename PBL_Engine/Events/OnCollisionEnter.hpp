#pragma once

#include <DirectXMath.h>

#include "ECS/Entity.hpp"
#include "ECS/Event.hpp"

ECS_EVENT(OnCollisionEnter) {
    Entity a, b;
    DirectX::XMFLOAT3 minSeparatingVector;
};
