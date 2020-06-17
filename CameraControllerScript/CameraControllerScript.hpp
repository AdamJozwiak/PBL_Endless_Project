#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <Components/Transform.hpp>
#include <memory>

#include "CameraControllerScriptAPI.hpp"
#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Events/OnGameStateChange.hpp"
#include "Script.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class CAMERACONTROLLERSCRIPT_API CameraControllerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    CameraControllerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);
    void onGameStateChange(OnGameStateChange const &event);

    // -------------------------------------------------------- Methods -- == //
    void setPosition();  //? May not be useful?
    void shake(float deltaTime);

  private:
    // ============================================================== Data == //
    GameState currentState = GAME_LAUNCH_FADE_IN;

    bool (*isKeyPressed)(int const key);

    EntityId playerId;
    EntityId menuCameraId;
    Transform originalTransform;
    float smoothing = 0.2f;
    DirectX::XMFLOAT3 shakeOffset;
    DirectX::XMFLOAT3 offset = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 lastPosition = {0.0f, 0.0f, 0.0f};
    // Entity cameraTarget;
    // float rotateSpeed = 0.0f;
    // float rotate;
    // float offsetDistance = 0.0f;
    // float offsetHeight = 0.0f;
    // bool following = true;
};

// ////////////////////////////////////////////////////////////////////////// //
