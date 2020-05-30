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

    void draw(std::string const& text);

  private:
    // ============================================================== Data == //
    Window& window;
    Graphics& gfx;

    std::shared_ptr<Text> textRenderer;
    DirectX::XMFLOAT2 position, size;
};

// ////////////////////////////////////////////////////////////////////////// //
