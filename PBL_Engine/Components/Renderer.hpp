#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <string>

#include "ECS/Component.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class Renderable;

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(Renderer) {
    std::shared_ptr<Renderable> renderable;
    struct {
        std::string albedoPath, ambientOcclusionPath, metallicSmoothnessPath,
            normalPath, heightPath;
        float parallaxHeight;
    } material;
};

// ////////////////////////////////////////////////////////////////////////// //
