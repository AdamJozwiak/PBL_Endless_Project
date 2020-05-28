// ///////////////////////////////////////////////////////////////// Includes //
#include "CameraControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" CAMERACONTROLLERSCRIPT_API void create(
    std::shared_ptr<Script>& script, Entity entity) {
    script = std::make_shared<CameraControllerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
CameraControllerScript::CameraControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void CameraControllerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(CameraControllerScript::onCollisionEnter));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void CameraControllerScript::update(float const deltaTime){};

// ------------------------------------------------------------- Events -- == //
void CameraControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

// ------------------------------------------------------------ Methods -- == //
void CameraControllerScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
