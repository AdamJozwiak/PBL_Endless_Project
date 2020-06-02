#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ECS/System.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class Window;
class Graphics;

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(WindowSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ----------------------------------------------- Public interface -- == //
    Window &window();
    Graphics &gfx();

  private:
    // ============================================================== Data == //
    std::unique_ptr<Window> windowPtr;
};

// ////////////////////////////////////////////////////////////////////////// //
