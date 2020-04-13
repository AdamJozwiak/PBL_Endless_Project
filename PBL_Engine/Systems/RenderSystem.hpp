#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Camera.h"
#include "Components/Components.hpp"
#include "ECS/System.hpp"
#include "ImguiManager.h"

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

  private:
    // ============================================================== Data == //
    std::unique_ptr<Window> window;
    std::unique_ptr<Camera> camera;
    ImguiManager imgui;
    float speed_factor = 1.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
