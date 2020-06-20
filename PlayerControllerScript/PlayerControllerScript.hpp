#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <memory>

#include "Components/Components.hpp"
#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Events/OnGameStateChange.hpp"
#include "PlayerControllerScriptAPI.hpp"
#include "Script.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class PLAYERCONTROLLERSCRIPT_API PlayerControllerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    PlayerControllerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);
    void onGameStateChange(OnGameStateChange const &event);

    // -------------------------------------------------------- Methods -- == //
    // void changeToHuman() { changeForm(humanForm); }
    void doGameLogic(float const deltaTime);
    void changeForm(EntityId const &newForm);
    void resetTorchLight();
    void transitionForms(float deltaTime);

    bool canChangeForm;
    bool inputLaneUp, inputLaneDown;
    bool inputAscend, inputDescend;
    bool inputChangeFormEagle, inputChangeFormCat;
    bool firstThrust = false;
    bool isGrounded = true;

  private:
    // ============================================================== Data == //
    GameState currentState = GAME_LAUNCH_FADE_IN;

    bool (*isKeyPressed)(int const key);
    float oscilate(float input, float factor, float max, float min = 0.0f);
    float ascend(float const x, float const thrustForce);

    float movementAcceleration = 40.0f;
    float runSpeed = 7.5f;
    float previousAnimSpeed = 0.0f;

    float aCValue = 0.0f;
    float aQValue = 0.0f;
    float minC = 0.1f;
    float minQ = 0.001f;
    float maxC = 0.01f;
    float maxQ = 0.1f;
    // float minC = 0.1f;
    // float minQ = 0.003f;
    // float maxC = 0.0f;
    // float maxQ = 0.5f;
    float intensityValue = 0.0f;
    float maxIntensity = 1.5f;
    float minIntensity = 1.5f;
    // float maxIntensity = 1.5f;
    // float minIntensity = 0.0f;
    float lightValue = 1.0f;
    float blackProportion = 1.0f;
    float deathTimer = 0.0f;

    DirectX::XMFLOAT3 originalScaleWolf, originalScaleEagle;
    float originalOffsetWolf, originalOffsetEagle;

    DirectX::XMFLOAT3 currentVelocity = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 moveInput = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 idlePosition = {0.0f, 0.0f, 0.0f};
    int currentLane = 0;
    int laneWidth = 2;
    Rigidbody rb;
    EntityId humanForm;            // TODO: To find the entity
    EntityId eagleForm;            // TODO: To find the entity
    EntityId catForm;              // TODO: To find the entity
    EntityId explosionPrefab;      // TODO: To find the entity
    EntityId explosionPrefabFail;  // TODO: To find the entity
    EntityId torch;                // TODO: To find the entity
    EntityId currentForm;          // TODO: To find the entity
    EntityId groundCheck;
    // Light torchLight;              // TODO: Add component for light
    // CameraController
    //    mainCamera;  // TODO: Add reference to the script? may not need
    float canChangeFormTimer = 0.0f;
    float canChangeFormCooldown = 1.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
