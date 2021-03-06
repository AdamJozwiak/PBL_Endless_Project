#include "Window.h"

#include <sstream>

#include "WindowsThrowMacros.h"
#include "imgui/examples/imgui_impl_win32.h"
#include "resource.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM,
                                                             LPARAM);

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept : hInst(GetModuleHandle(nullptr)) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = static_cast<HICON>(
        LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetName();
    wc.hIconSm = static_cast<HICON>(
        LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));
    RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass() {
    UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept { return wndClassName; }

HINSTANCE Window::WindowClass::GetInstance() noexcept { return wndClass.hInst; }

Window::Window(int width, int height, const char* name)
    : width(width), height(height) {
    // Calculate window size based on desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
                         FALSE) == 0) {
        throw HWND_LAST_EXCEPT();
    }
    // Create window and get hWnd
    hWnd = CreateWindow(WindowClass::GetName(), name,
                        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT,
                        CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
                        nullptr, nullptr, WindowClass::GetInstance(),
                        this  // <--- Custom Parameter
    );

    if (hWnd == nullptr) {
        throw HWND_LAST_EXCEPT();
    }
    // new windows start as hidden
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    // Init ImGui Win32 Impl
    ImGui_ImplWin32_Init(hWnd);
    // create graphics object
    pGfx = std::make_unique<Graphics>(hWnd, width, height);

    // Register mouse raw input device
    RAWINPUTDEVICE rawInputDevice{.usUsagePage = 0x01,
                                  .usUsage = 0x02,
                                  .dwFlags = 0,
                                  .hwndTarget = nullptr};
    UINT numberOfRawInputDevices = 1u;
    if (RegisterRawInputDevices(&rawInputDevice, numberOfRawInputDevices,
                                sizeof(rawInputDevice)) == FALSE) {
        throw HWND_LAST_EXCEPT();
    }
}

Window::~Window() {
    ImGui_ImplWin32_Shutdown();
    DestroyWindow(hWnd);
}

std::optional<int> Window::ProcessMessages() {
    MSG msg;

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return msg.wParam;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return {};
}

Graphics& Window::Gfx() {
    if (!pGfx) {
        throw HWND_NOGFX_EXCEPT();
    }
    return *pGfx;
}

LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam,
                                      LPARAM lParam) noexcept {
    // Use create parameter passed in from CreateWindow() to store window class
    // pointer at WinAPI side
    if (msg == WM_NCCREATE)  // Non Client Create
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate =
            reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        // set WinAPI-managed user data to store ptr to window instance
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is
        // finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        // forward message to window instance handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }
    // If we get a message before the WM_NCCREATE message, handle with default
    // handler
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam,
                                      LPARAM lParam) noexcept {
    // Retrieve ptr to window instance
    Window* const pWnd =
        reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    // Forward message to window instance handler
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

// ------------------------------ Messages ------------------------------ //

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam,
                          LPARAM lParam) noexcept {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true;
    }

    const auto imio = ImGui::GetIO();

    switch (msg) {
        // we don't want the DefProc to handle this message because
        // we want our destructor to destroy the window, so return 0 instead of
        // break
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_DESTROY:
            return 0;
        // clear keystate when window loses focus to prevent input getting
        // "stuck"
        case WM_KILLFOCUS:
            keyboard.ClearState();
            break;
        /*********** KEYBOARD MESSAGES ***********/
        case WM_KEYDOWN:
        // syskey commands need to be handled to track ALT key (VK_MENU) and F10
        case WM_SYSKEYDOWN:

            if (imio.WantCaptureKeyboard) {
                break;
            }

            if (!(lParam & 0x40000000) ||
                keyboard.AutorepeatIsEnabled())  // filter autorepeat
            {
                keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (imio.WantCaptureKeyboard) {
                break;
            }
            keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
            break;
        case WM_CHAR:
            if (imio.WantCaptureKeyboard) {
                break;
            }
            keyboard.OnChar(static_cast<unsigned char>(wParam));
            break;
        /*********** END KEYBOARD MESSAGES ***********/

        /************* MOUSE MESSAGES ****************/
        case WM_MOUSEMOVE: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            // in client region -> log move, and log enter + capture mouse (if
            // not previously in window)
            if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
                mouse.OnMouseMove(pt.x, pt.y);
                if (!mouse.IsInWindow()) {
                    SetCapture(hWnd);
                    mouse.OnMouseEnter();
                }
            }
            // not in client -> log move / maintain capture if button down
            else {
                if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
                    mouse.OnMouseMove(pt.x, pt.y);
                }
                // button up -> release capture / log event for leaving
                else {
                    ReleaseCapture();
                    mouse.OnMouseLeave();
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnLeftPressed(pt.x, pt.y);
            // SetForegroundWindow(hWnd);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnRightPressed(pt.x, pt.y);
            break;
        }
        case WM_LBUTTONUP: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnLeftReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_RBUTTONUP: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnRightReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_MOUSEWHEEL: {
            if (imio.WantCaptureKeyboard) {
                break;
            }
            const POINTS pt = MAKEPOINTS(lParam);
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            mouse.OnWheelDelta(pt.x, pt.y, delta);
            break;
        }
            /************** END MOUSE MESSAGES **************/

        // Raw mouse messages
        case WM_INPUT: {
            if (!mouse.RawEnabled()) {
                break;
            }

            // Get the size of the input data
            UINT size;
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
                                nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) {
                break;
            }
            rawBuffer.resize(size);

            // Read the input data
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
                                rawBuffer.data(), &size,
                                sizeof(RAWINPUTHEADER)) != size) {
                break;
            }

            // Process the raw input data
            auto& rawInput =
                reinterpret_cast<RAWINPUT const&>(*rawBuffer.data());
            if (rawInput.header.dwType == RIM_TYPEMOUSE &&
                (rawInput.data.mouse.lLastX != 0 ||
                 rawInput.data.mouse.lLastY != 0)) {
                mouse.OnRawDelta(rawInput.data.mouse.lLastX,
                                 rawInput.data.mouse.lLastY);
            }
        } break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ------------------------ Exception Handling ------------------------------ //

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept {
    char* pMsgBuf = nullptr;
    // windows will allocate memory for err string and make our pointer point to
    // it
    const DWORD nMsgLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr);
    // 0 string length returned indicates a failure
    if (nMsgLen == 0) {
        return "Unidentified error code";
    }
    // copy error string from windows-allocated buffer to std::string
    std::string errorString = pMsgBuf;
    // free windows buffer
    LocalFree(pMsgBuf);
    return errorString;
}

Window::HrException::HrException(int line, const char* file,
                                 HRESULT hr) noexcept
    : Exception(line, file), hr(hr) {}

const char* Window::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
        << "[Description] " << GetErrorDescription() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept {
    return "Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept { return hr; }

std::string Window::HrException::GetErrorDescription() const noexcept {
    return Exception::TranslateErrorCode(hr);
}

const char* Window::NoGfxException::GetType() const noexcept {
    return "Window Exception [No Graphics]";
}
