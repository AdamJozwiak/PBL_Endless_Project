// ///////////////////////////////////////////////////////////////// Includes //
#include "Button.hpp"

#include <Events/OnButtonClick.hpp>

#include "ECS/ECS.hpp"
#include "Systems/SoundSystem.hpp"
#include "Text.h"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
Button::Button(Window& window, const WCHAR* fontFamily, float fontSize,
               DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size)
    : window(window),
      gfx(window.Gfx()),
      textRenderer(std::make_unique<Text>(gfx, fontFamily, fontSize)),
      position(position),
      size(size) {}

Button::Button(Window& window, const WCHAR* fontFamily,
               std::wstring const& path, float fontSize,
               DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size)
    : window(window),
      gfx(window.Gfx()),
      textRenderer(std::make_unique<Text>(gfx, fontFamily, path, fontSize)),
      position(position),
      size(size) {}

void Button::draw(std::string const& text, float a, float r, float g, float b) {
    auto isMouseInside = [this] {
        DirectX::XMINT2 const mousePosition{window.mouse.GetPosX(),
                                            window.mouse.GetPosY()};
        return mousePosition.x > position.x &&
               mousePosition.x < position.x + size.x &&
               mousePosition.y > position.y &&
               mousePosition.y < position.y + size.y;
    };
    bool const mouse = window.mouse.LeftIsPressed();

    // Handle the mouse clicks
    if (isMouseInside()) {
        if (!hovered) {
            hovered = true;

            Registry::instance().system<SoundSystem>()->play(
                "Assets\\Audio\\Airlock\\airlock-door-click-01.wav", 0.025f);
        }

        if (mouse && !clicked) {
            clicked = true;
            Registry::instance().send(OnButtonClick{.button = this});

            Registry::instance().system<SoundSystem>()->play(
                "Assets\\Audio\\gui\\01.wav");
        } else if (!mouse && clicked) {
            clicked = false;
        }
    } else {
        if (hovered) {
            hovered = false;

            Registry::instance().system<SoundSystem>()->play(
                "Assets\\Audio\\Airlock\\airlock-door-click-01.wav", 0.0125f);
        }
    }

    textRenderer->RenderText(gfx, text, isMouseInside(), position, a, r, g, b);
}

// ///////////////////////////////////////////////////////////////////////// //
