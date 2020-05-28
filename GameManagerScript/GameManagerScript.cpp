// ///////////////////////////////////////////////////////////////// Includes //
#include "GameManagerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" GAMEMANAGERSCRIPT_API void create(std::shared_ptr<Script>& script,
                                             Entity entity) {
    script = std::make_shared<GameManagerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
GameManagerScript::GameManagerScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void GameManagerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(GameManagerScript::onCollisionEnter));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void GameManagerScript::update(float const deltaTime){};

// ------------------------------------------------------------- Events -- == //
void GameManagerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

// ------------------------------------------------------------ Methods -- == //
void GameManagerScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
