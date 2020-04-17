#include "App.h"

App::App() : wnd(800, 600, "PBL_ENGINE")
{

}

int App::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto exitCode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *exitCode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	wnd.Gfx().ClearBuffer(0, 0, 0);
	wnd.Gfx().DrawTestTriangle();
	wnd.Gfx().EndFrame();
}
