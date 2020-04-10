#pragma once
//Error exception macro
#define HWND_EXCEPT( hr ) Window::HrException( __LINE__,__FILE__,(hr) )
#define HWND_LAST_EXCEPT() Window::HrException( __LINE__,__FILE__,GetLastError() )
#define HWND_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )