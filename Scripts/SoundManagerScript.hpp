#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "Script.hpp"
#include "ScriptsAPI.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
ECS_SYSTEM_FORWARD(SoundSystem);

// //////////////////////////////////////////////////////////////////// Class //
class SCRIPTS_API SoundManagerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    SoundManagerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

  private:
    // ============================================================== Data == //
    std::shared_ptr<SoundSystem> soundSystem;
};

// ////////////////////////////////////////////////////////////////////////// //
