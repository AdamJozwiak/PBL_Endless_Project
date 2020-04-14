#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(SceneSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;

  private:
    // ============================================================== Data == //
};

// ////////////////////////////////////////////////////////////////////////// //
