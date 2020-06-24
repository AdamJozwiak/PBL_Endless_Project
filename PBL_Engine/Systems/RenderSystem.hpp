#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Billboard.h"
#include "Camera.h"
#include "FireParticle.h"
#include "ImguiManager.h"
#include "Mesh.h"
#include "PointLight.h"
#include "PostProcessing.h"
#include "SolidSphere.h"

// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class Window;

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(RenderSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ============================================================== Data == //
    Window *window;
    std::shared_ptr<PostProcessing> bloom, colorCorrection, shadowPass;

  private:
    // ============================================================== Data == //
    std::shared_ptr<Camera> mainCamera, freeCamera;
    std::shared_ptr<PointLight> playersTorch;
    DirectX::XMMATRIX normalFOV, shadowFOV;
    ImguiManager imgui;
    Transform *mainCameraTransform;
    bool (*isKeyPressed)(int const key);
};

// ////////////////////////////////////////////////////////////////////////// //
