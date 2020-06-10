#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <string>

#include "ECS/Component.hpp"

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(Skybox) {
    struct {
        std::string leftPath, rightPath, backPath, frontPath, bottomPath,
            topPath;
    } material;
};

// ////////////////////////////////////////////////////////////////////////// //
