#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <string>

// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(BehaviourSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    Behaviour behaviour(std::string const &name, Entity const &entity);
};

// ////////////////////////////////////////////////////////////////////////// //
