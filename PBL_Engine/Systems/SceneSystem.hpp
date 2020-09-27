#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"
#include "LevelParser.h"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(SceneSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ----------------------------------------------- Public interface -- == //
    Entity spawnPrefab(std::string const &path, bool cache = false);

  private:
    // ============================================================== Data == //
    LevelParser levelParser;
};

// ////////////////////////////////////////////////////////////////////////// //
