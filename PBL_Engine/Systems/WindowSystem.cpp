// ///////////////////////////////////////////////////////////////// Includes //
#include "WindowSystem.hpp"

#include "ECS/ECS.hpp"
#include "Window.h"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void WindowSystem::filters() {}

void WindowSystem::setup() {
    windowPtr = std::make_unique<Window>(1680, 1050, "Ohunkakan");
}

void WindowSystem::update(float deltaTime){};

void WindowSystem::release() {}

// --------------------------------------------------- Public interface -- == //
Window& WindowSystem::window() { return *windowPtr; }

Graphics& WindowSystem::gfx() { return windowPtr->Gfx(); }

// ////////////////////////////////////////////////////////////////////////// //
