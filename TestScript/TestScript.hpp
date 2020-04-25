#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Script.hpp"
#include "TestScriptAPI.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class TESTSCRIPT_API TestScript : public Script {
  public:
    // ========================================================= Behaviour == //
    TestScript(Entity const &entity);

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
    float lastX, lastY, lastZ;
    float lastDeltaTime;
};

// ////////////////////////////////////////////////////////////////////////// //
