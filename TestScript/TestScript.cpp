// ///////////////////////////////////////////////////////////////// Includes //
#include "TestScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" TESTSCRIPT_API void create(std::shared_ptr<Script>& script,
                                      Entity entity) {
    script = std::make_shared<TestScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
TestScript::TestScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void TestScript::setup() {
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void TestScript::update(float const deltaTime) {
    auto& transform = entity.get<Transform>();
    transform.eulerAngle_y += 45.0f * deltaTime;

    if (isKeyPressed('A')) {
        transform.position_x -= 5.0f * deltaTime;
    }
    if (isKeyPressed('D')) {
        transform.position_x += 5.0f * deltaTime;
    }
    if (isKeyPressed('W')) {
        transform.position_z += 5.0f * deltaTime;
    }
    if (isKeyPressed('S')) {
        transform.position_z -= 5.0f * deltaTime;
    }
};

// ------------------------------------------------------------ Methods -- == //
void TestScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
