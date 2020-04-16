#pragma once

#include "ECS/Component.hpp"

ECS_COMPONENT(BoxCollider) {

    float size_x;
    float size_y;
    float size_z;
    float center_x;
    float center_y;
    float center_z;
};