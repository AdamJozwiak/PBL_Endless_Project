#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "Script.hpp"
#include "ScriptsAPI.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class SoundManagerScript;

// //////////////////////////////////////////////////////////////////// Class //
class SCRIPTS_API FlameScript : public Script {
  public:
    // ========================================================= Behaviour == //
    FlameScript(Entity const &entity);
    ~FlameScript() override;

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

  private:
    // ============================================================== Data == //
    std::shared_ptr<SoundManagerScript> soundManager;
};

// ////////////////////////////////////////////////////////////////////////// //
