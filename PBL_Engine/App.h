#pragma once
#include "Camera.h"
#include "ImguiManager.h"
#include "Timer.h"
#include "Window.h"

class App {
  public:
    App();
    int Go();
    ~App();

  private:
    void DoFrame();

  private:
    ImguiManager imgui;
    Window wnd;
    Timer timer;
    Camera cam;
    float speed_factor = 1.0f;
    std::vector<std::unique_ptr<class Renderable>> renderables;
    static constexpr size_t nRenderables = 180;
};