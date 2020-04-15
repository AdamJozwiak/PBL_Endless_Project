#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ECS/Component.hpp"

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(Transform) {
    Transform *parent;

    // TODO: Replace these with appropriate vectors
    float rotation_x;
    float rotation_y;
    float rotation_z;
    float rotation_w;
    float position_x;
    float position_y;
    float position_z;
    float scale_x;
    float scale_y;
    float scale_z;
    float eulerAngle_x;
    float eulerAngle_y;
    float eulerAngle_z;

    int root_Order;
};

// ////////////////////////////////////////////////////////////////////////// //
