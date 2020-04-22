#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ECS/Entity.hpp"
#include "EngineAPI.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API Script {
  public:
    // ========================================================= Behaviour == //
    Script(Entity const& entity) : entity(entity) {}
    virtual ~Script() = default;

    virtual void setup(){};
    virtual void update(float const deltaTime){};

  protected:
    // ============================================================== Data == //
    Entity entity;
};

// ////////////////////////////////////////////////////////////////////////// //
