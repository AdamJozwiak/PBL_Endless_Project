#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <memory>

#include "Components/Components.hpp"
#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
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

    // -------------------------------------------------------- Methods -- == //
    // void changeToHuman() { changeForm(humanForm); }
    void changeForm(EntityId const &newForm);
    void resetTorchLight(Entity light);
    void transitionForms(float deltaTime);

    bool canChangeForm;
    bool inputLaneUp, inputLaneDown;
    bool inputAscend, inputDescend;
    bool inputChangeFormEagle, inputChangeFormCat;
    bool firstThrust = false;
    bool isGrounded = true;
  private:
    // ============================================================== Data == //
    bool (*isKeyPressed)(int const key);
    float oscilate(float input, float factor, float max, float min = 0.0f);
    float ascend(float const x, float const thrustForce);

    float movementAcceleration = 40.0f;
    float runSpeed = 7.5f;
    float previousAnimSpeed = 0.0f;

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
    float canChangeFormCooldown = 2.5f;
};

// ////////////////////////////////////////////////////////////////////////// //
