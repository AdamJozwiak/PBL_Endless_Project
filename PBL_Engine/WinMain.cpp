#include "Window.h"
#include <sstream>

const LPCSTR WINDOW_NAME = "Engine Prototype";

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// ------------------------------ Window Instance Creation ------------------------------ //

	Window wnd(800, 300, "PBL_ENGINE");

	// ------------------------------ Message Handler ------------------------------ //

	MSG msg;
	BOOL getResult;

	while ((getResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (getResult == -1)
	{
		return -1;
	}

	return msg.wParam;
};