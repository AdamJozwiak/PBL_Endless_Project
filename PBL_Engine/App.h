#pragma once
#include "Window.h"
#include "ImguiManager.h"
#include "Timer.h"

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
	std::vector<std::unique_ptr<class Renderable>> renderables;
	static constexpr size_t nRenderables = 180;
};