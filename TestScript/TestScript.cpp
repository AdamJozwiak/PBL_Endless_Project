// ///////////////////////////////////////////////////////////////// Includes //
#include "TestScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" TESTSCRIPT_API void create(std::shared_ptr<Script>& script,
                                      Entity entity) {
    script = std::make_shared<TestScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
TestScript::TestScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void TestScript::setup(){};

void TestScript::update(float const deltaTime){};

// ------------------------------------------------------------ Methods -- == //
void TestScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
