#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <memory>

#include "Graphics.h"
#include "Window.h"

// ///////////////////////////////////////////////////// Forward declarations //
class Text;

// //////////////////////////////////////////////////////////////////// Class //
class Button {
  public:
    // ========================================================= Behaviour == //
    Button(Window& window, const WCHAR* fontFamily, float fontSize,
           DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size);
    Button(Window& window, const WCHAR* fontFamily, std::wstring const& path,
           float fontSize, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size);

    void draw(std::string const& text, float a = 1.0f, float r = 255.0f,
              float g = 255.0f, float b = 255.0f);

  private:
    // ============================================================== Data == //
    Window& window;
    Graphics& gfx;

    bool clicked = false;
    bool hovered = false;

    std::shared_ptr<Text> textRenderer;
    DirectX::XMFLOAT2 position, size;
};

// ////////////////////////////////////////////////////////////////////////// //
