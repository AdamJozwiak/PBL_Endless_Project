// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "CameraControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"
#include "easings.hpp"

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
    registry.listen<OnGameStateChange>(
        MethodListener(CameraControllerScript::onGameStateChange));

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
    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
        } break;
        case MENU: {
        } break;
        case CHANGE_MENU_TYPE_TO_MAIN: {
        } break;
        case CHANGE_MENU_TYPE_TO_PAUSE: {
        } break;
        case MENU_TO_GAME_FADE_OUT: {
        } break;
        case NEW_GAME_SETUP: {
        } break;
        case GAME_FADE_IN: {
        } break;
        case GAME: {
            auto const& playerTransform = Entity(playerId).get<Transform>();

            entity.get<Transform>().position = DirectX::XMFLOAT3{
                interpolate(easeOutQuad, lastPosition.x,
                            playerTransform.position.x + offset.x, smoothing,
                            deltaTime),
                interpolate(easeOutQuad, lastPosition.y,
                            playerTransform.position.y * 0.75f + offset.y,
                            smoothing, deltaTime),
                interpolate(easeOutQuad, lastPosition.z,
                            playerTransform.position.z + offset.z, smoothing,
                            deltaTime)};

            //! This line was in late update, may not work
            lastPosition = entity.get<Transform>().position;
        } break;
        case DEATH_RESULTS: {
        } break;
        case RESULTS_TO_GAME_FADE_OUT: {
        } break;
        case GAME_EXIT_FADE_OUT: {
        } break;
        default: {
        } break;
    };
};

// ------------------------------------------------------------- Events -- == //
void CameraControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}
void CameraControllerScript::onGameStateChange(OnGameStateChange const& event) {
    currentState = event.nextState;
}

// ------------------------------------------------------------ Methods -- == //
void CameraControllerScript::setPosition() {}

void CameraControllerScript::shake(float deltaTime) {
    auto const& playerTransform = Entity(playerId).get<Transform>();
    auto const& cameraPosition = entity.get<Transform>().position;
    std::random_device rnd;
    std::mt19937 rng(rnd());
    std::uniform_int_distribution<int> uni(-1, 1);
    shakeOffset = {(float)uni(rng), (float)uni(rng), 0.0f};
    entity.get<Transform>().position = DirectX::XMFLOAT3{
        interpolate(easeOutQuad, lastPosition.x,
                    playerTransform.position.x + offset.x + shakeOffset.x,
                    smoothing, deltaTime),
        interpolate(
            easeOutQuad, lastPosition.y,
            playerTransform.position.y * 0.75f + offset.y + shakeOffset.y,
            smoothing, deltaTime),
        interpolate(easeOutQuad, lastPosition.z,
                    playerTransform.position.z + offset.z + shakeOffset.z,
                    smoothing, deltaTime)};

    //! This line was in late update, may not work
    lastPosition = entity.get<Transform>().position;
}

// ////////////////////////////////////////////////////////////////////////// //
