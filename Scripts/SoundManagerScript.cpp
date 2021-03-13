// ///////////////////////////////////////////////////////////////// Includes //
#include "SoundManagerScript.hpp"

#include "ECS/ECS.hpp"
#include "Systems/SoundSystem.hpp"
#include "script-factory.hpp"

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(SoundManagerScript)

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
SoundManagerScript::SoundManagerScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void SoundManagerScript::setup() {
    soundSystem = registry.system<SoundSystem>();
};

void SoundManagerScript::update(float const deltaTime){};

// ////////////////////////////////////////////////////////////////////////// //
