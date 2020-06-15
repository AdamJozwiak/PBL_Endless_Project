#pragma once
// ///////////////////////////////////////////////////////////////// Includes //
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(CheckCollisionsSystem) {
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;
};
// ////////////////////////////////////////////////////////////////////////// //
