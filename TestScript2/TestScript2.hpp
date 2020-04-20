#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "Script.hpp"
#include "TestScript2API.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class TestScript;

// //////////////////////////////////////////////////////////////////// Class //
class TESTSCRIPT2_API TestScript2 : public Script {
  public:
    // ========================================================= Behaviour == //
    TestScript2(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // ------------------------------------ Access to different scripts -- == //
    std::shared_ptr<TestScript> differentScript;
};

// ////////////////////////////////////////////////////////////////////////// //
