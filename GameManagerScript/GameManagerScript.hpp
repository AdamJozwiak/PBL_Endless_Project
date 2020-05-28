#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "GameManagerScriptAPI.hpp"
#include "Script.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class GAMEMANAGERSCRIPT_API GameManagerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    GameManagerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);

    // -------------------------------------------------------- Methods -- == //
    void method();

  private:
    // ============================================================== Data == //
    bool (*isKeyPressed)(int const key);
};

// ////////////////////////////////////////////////////////////////////////// //
