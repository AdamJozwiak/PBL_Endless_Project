#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
// ECS
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(ScriptSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
};

// ////////////////////////////////////////////////////////////////////////// //
