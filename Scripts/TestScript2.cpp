// ///////////////////////////////////////////////////////////////// Includes //
#include "TestScript2.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "TestScript.hpp"
#include "script-factory.hpp"

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(TestScript2)

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
TestScript2::TestScript2(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void TestScript2::setup() {
    auto behaviourSystem = Registry::instance().system<BehaviourSystem>();
    auto const& entity = *behaviourSystem->entities.begin();
    auto const& behaviour = entity.get<Behaviour>();
    differentScript = std::static_pointer_cast<TestScript>(behaviour.script);
};

void TestScript2::update(float const deltaTime) { differentScript->method(); };

// ////////////////////////////////////////////////////////////////////////// //
