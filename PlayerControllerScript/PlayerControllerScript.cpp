// ///////////////////////////////////////////////////////////////// Includes //
#include "PlayerControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" PLAYERCONTROLLERSCRIPT_API void create(
    std::shared_ptr<Script>& script, Entity entity) {
    script = std::make_shared<PlayerControllerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
PlayerControllerScript::PlayerControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void PlayerControllerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(PlayerControllerScript::onCollisionEnter));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void PlayerControllerScript::update(float const deltaTime){};

// ------------------------------------------------------------- Events -- == //
void PlayerControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

// ------------------------------------------------------------ Methods -- == //
void PlayerControllerScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
