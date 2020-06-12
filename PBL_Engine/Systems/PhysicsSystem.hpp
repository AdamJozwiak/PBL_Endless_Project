#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(PhysicsSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ------------------------------------------------------ Functions -- == //
    void gravity(Entity & entity, float deltaTime);

  protected:
    // ============================================================== Data == //
    float gravityFactor;
    float maxVelocity;
    float secondsToAchieveMaxVelocity;
    float movementFactor;
};

// ////////////////////////////////////////////////////////////////////////// //
