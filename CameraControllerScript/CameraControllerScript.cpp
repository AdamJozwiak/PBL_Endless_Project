// ///////////////////////////////////////////////////////////////// Includes //
#include "CameraControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

// //////////////////////////////////////////////////////////////// Utilities //
DirectX::XMFLOAT3 operator-(DirectX::XMFLOAT3 const& a,
                            DirectX::XMFLOAT3 const& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

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
    // Set event listeners
    registry.listen<OnCollisionEnter>(
        MethodListener(CameraControllerScript::onCollisionEnter));

    // Set utility functors
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };

    playerId =
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;
    // registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;
    offset = entity.get<Transform>().position -
             Entity(playerId).get<Transform>().position;
};

void CameraControllerScript::update(float const deltaTime) {
    auto const& playerTransform = Entity(playerId).get<Transform>();

    entity.get<Transform>().position = DirectX::XMFLOAT3{
        std::lerp(lastPosition.x, playerTransform.position.x + offset.x,
                  smoothing * deltaTime),
        std::lerp(lastPosition.y, playerTransform.position.y * 0.75f + offset.y,
                  smoothing * deltaTime),
        std::lerp(lastPosition.z, playerTransform.position.z + offset.z,
                  smoothing * deltaTime)};

    //! This line was in late update, may not work
    lastPosition = entity.get<Transform>().position;
};

// ------------------------------------------------------------- Events -- == //
void CameraControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

// ------------------------------------------------------------ Methods -- == //
void CameraControllerScript::setPosition() {}

// ////////////////////////////////////////////////////////////////////////// //
