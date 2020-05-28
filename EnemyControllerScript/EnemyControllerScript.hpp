#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "EnemyControllerScriptAPI.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Script.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class ENEMYCONTROLLERSCRIPT_API EnemyControllerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    EnemyControllerScript(Entity const &entity);

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
