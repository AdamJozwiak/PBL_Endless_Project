#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Camera.h"
#include "ImguiManager.h"

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

    // ============================================================== Data == //
    std::unique_ptr<Window> window;

  private:
    // ============================================================== Data == //
    std::unique_ptr<Camera> camera;
    ImguiManager imgui;
    float speed_factor = 1.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
