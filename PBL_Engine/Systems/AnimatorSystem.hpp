#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <string>
#include <unordered_map>

#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(AnimatorSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;
};

// ////////////////////////////////////////////////////////////////////////// //
