// ///////////////////////////////////////////////////////////////// Includes //
#include "FlameScript.hpp"

#include "Components/Behaviour.hpp"
#include "ECS/ECS.hpp"
#include "SoundManagerScript.hpp"
#include "Systems/PropertySystem.hpp"
#include "Systems/SoundSystem.hpp"
#include "script-factory.hpp"

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(FlameScript)

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
FlameScript::FlameScript(Entity const& entity) : Script(entity){};

FlameScript::~FlameScript(){}

// ----------------------------------------- System's virtual functions -- == //
void FlameScript::setup() {
    soundManager = std::static_pointer_cast<SoundManagerScript>(
        registry.system<PropertySystem>()
            ->findEntityByName("Sound Manager")
            .at(0)
            .get<Behaviour>()
            .script);
};

void FlameScript::update(float const deltaTime){};

// ////////////////////////////////////////////////////////////////////////// //
