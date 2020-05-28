// ///////////////////////////////////////////////////////////////// Includes //
#include "EnemyControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" ENEMYCONTROLLERSCRIPT_API void create(
    std::shared_ptr<Script>& script, Entity entity) {
    script = std::make_shared<EnemyControllerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
EnemyControllerScript::EnemyControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void EnemyControllerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(EnemyControllerScript::onCollisionEnter));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void EnemyControllerScript::update(float const deltaTime){};

// ------------------------------------------------------------- Events -- == //
void EnemyControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

// ------------------------------------------------------------ Methods -- == //
void EnemyControllerScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
