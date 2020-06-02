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

    DirectX::XMMATRIX transformMatrix(Entity entity);

    // ============================================================== Data == //
    Window *window;

  private:
    // ============================================================== Data == //
    std::shared_ptr<Camera> mainCamera, freeCamera;
    ImguiManager imgui;
    float speed_factor = 1.0f;
    std::unique_ptr<SolidSphere> sphere;
    std::unique_ptr<Billboard> billboard;
    std::unique_ptr<FireParticle> fireParticle;
    std::unique_ptr<PostProcessing> bloom, colorCorrection;
};

// ////////////////////////////////////////////////////////////////////////// //
