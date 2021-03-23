// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "CameraControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"
#include "easings.hpp"
#include "math-operators.hpp"
#include "script-factory.hpp"

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(CameraControllerScript)

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
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0);
    menuCameraId = registry.system<PropertySystem>()
                       ->findEntityByName("Menu Camera")
                       .at(0);
    // registry.system<PropertySystem>()->findEntityByTag("Player").at(0);
    auto& entityTransform = entity.get<Transform>();
    Entity const& menuCamera = menuCameraId;
    auto const& menuCameraTransform = menuCamera.get<Transform>();

    offset =
        entityTransform.position - Entity(playerId).get<Transform>().position;

    originalTransform = entityTransform;

    entityTransform = menuCameraTransform;
    entityTransform.position.z += 7.5f;

    middleTransform = entityTransform;
    middleTransform.position =
        lerp(originalTransform.position, menuCameraTransform.position, 0.5f);
    middleTransform.rotation =
        lerp(originalTransform.rotation, menuCameraTransform.rotation, 0.5f);

    entity.set<Transform>(entityTransform);
};

void CameraControllerScript::update(float const deltaTime) {
    switch (currentState) {
        case GameState::GAME_LAUNCH_FADE_IN:
        case GameState::MENU: {
            // Get the camera to the starting menu position
            auto smooth = 0.2f;
            auto& transform = entity.get<Transform>();
            Entity const& menuCamera = menuCameraId;
            auto& menuCameraTransform = menuCamera.get<Transform>();

            transform.position =
                interpolate(easeOutSine, transform.position,
                            menuCameraTransform.position, smooth, deltaTime);
            transform.rotation =
                interpolate(easeOutSine, transform.rotation,
                            menuCameraTransform.rotation, smooth, deltaTime);

            entity.set<Transform>(transform);
        } break;
        case GameState::CHANGE_MENU_TYPE_TO_MAIN: {
        } break;
        case GameState::CHANGE_MENU_TYPE_TO_PAUSE: {
        } break;
        // case GameState::MENU_TO_GAME_FADE_OUT: {
        //     auto& transform = entity.get<Transform>();
        //     auto easing = &easeOutQuint;
        //     auto smooth = 0.1f;
        //     transform.position.x =
        //         interpolate(easing, transform.position.x,
        //                     middleTransform.position.x, smooth, deltaTime);
        //     transform.position.y =
        //         interpolate(easing, transform.position.y,
        //                     middleTransform.position.y, smooth, deltaTime);
        //     transform.position.z =
        //         interpolate(easing, transform.position.z,
        //                     middleTransform.position.z, smooth, deltaTime);
        //     transform.rotation.x =
        //         interpolate(easing, transform.rotation.x,
        //                     middleTransform.rotation.x, smooth, deltaTime);
        //     transform.rotation.y =
        //         interpolate(easing, transform.rotation.y,
        //                     middleTransform.rotation.y, smooth, deltaTime);
        //     transform.rotation.z =
        //         interpolate(easing, transform.rotation.z,
        //                     middleTransform.rotation.z, smooth, deltaTime);
        //     transform.rotation.w =
        //         interpolate(easing, transform.rotation.w,
        //                     middleTransform.rotation.w, smooth, deltaTime);
        // } break;
        case GameState::NEW_GAME_SETUP: {
        } break;
        case GameState::MENU_TO_GAME_FADE_OUT:
        case GameState::GAME_FADE_IN: {
            auto& transform = entity.get<Transform>();
            Entity const& player = playerId;
            auto const& playerTransform = player.get<Transform>();
            auto easing = &easeOutQuint;
            auto smooth = 0.5f;

            transform.position = interpolate(easing, transform.position,
                                             playerTransform.position + offset,
                                             smooth, deltaTime);
            transform.rotation =
                interpolate(easing, transform.rotation,
                            originalTransform.rotation, smooth, deltaTime);

            entity.set<Transform>(transform);

            lastPosition = transform.position;
        } break;
        case GameState::GAME: {
            Entity const& player = playerId;
            auto const& playerTransform = player.get<Transform>();

            auto& transform = entity.get<Transform>();
            entity.set<Transform>([&] {
                transform.position = interpolate(
                    easeOutQuad, lastPosition,
                    playerTransform.position + offset, smoothing, deltaTime);
                return transform;
            }());

            //! This line was in late update, may not work
            lastPosition = transform.position;
        } break;
        case GameState::DEATH_RESULTS: {
        } break;
        case GameState::RESULTS_TO_GAME_FADE_OUT: {
        } break;
        case GameState::GAME_EXIT_FADE_OUT: {
        } break;
        default: {
        } break;
    };
};

// ------------------------------------------------------------- Events -- == //
void CameraControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a == entity || event.b == entity) {
        auto other = event.a == entity ? event.b : event.a;
    }
}
void CameraControllerScript::onGameStateChange(OnGameStateChange const& event) {
    currentState = event.nextState;
}

// ------------------------------------------------------------ Methods -- == //
void CameraControllerScript::setPosition() {}

void CameraControllerScript::shake(float const deltaTime) {
    auto& transform = entity.get<Transform>();
    auto const& playerTransform = Entity(playerId).get<Transform>();
    auto const& cameraPosition = entity.get<Transform>().position;
    static std::random_device rnd;
    static std::mt19937 rng(rnd());
    static std::normal_distribution<float> normalDistribution(0.0f, 0.5f);
    shakeOffset = {normalDistribution(rng), normalDistribution(rng), 0.0f};
    transform.position = DirectX::XMFLOAT3{
        interpolate(easeOutQuad, lastPosition.x,
                    playerTransform.position.x + offset.x + shakeOffset.x,
                    smoothing, deltaTime),
        interpolate(easeOutQuad, lastPosition.y,
                    playerTransform.position.y + offset.y + shakeOffset.y,
                    smoothing, deltaTime),
        interpolate(easeOutQuad, lastPosition.z,
                    playerTransform.position.z + offset.z + shakeOffset.z,
                    smoothing, deltaTime)};
    entity.set<Transform>(transform);

    //! This line was in late update, may not work
    lastPosition = entity.get<Transform>().position;
}

// ////////////////////////////////////////////////////////////////////////// //
