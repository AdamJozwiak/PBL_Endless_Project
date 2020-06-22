// ///////////////////////////////////////////////////////////////// Includes //
#include "UIRenderSystem.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "PropertySystem.hpp"
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"
#include "imgui/imgui.h"

// /////////////////////////////////////////////////////////////////// System //
// ----------------------------------------- System's virtual functions -- == //
void UIRenderSystem::filters() { filter<RectTransform>().filter<UIElement>(); }

void UIRenderSystem::setup() {
    window = &registry.system<WindowSystem>()->window();
    bloom = registry.system<RenderSystem>()->bloom;
    colorCorrection = registry.system<RenderSystem>()->colorCorrection;
}

void UIRenderSystem::release() {}

void UIRenderSystem::update(float deltaTime) {
    // Render UI
    Text::beginDrawing();
    {
        for (auto const& entity : entities) {
            auto const& rectTransform = entity.get<RectTransform>();
            auto const& uiElement = entity.get<UIElement>();

            if (uiElement.alpha > 0.001f) {
                if (uiElement.text) {
                    uiElement.text->RenderText(window->Gfx(), uiElement.content,
                                               false, rectTransform.position,
                                               uiElement.alpha);
                }
                if (uiElement.button) {
                    uiElement.button->draw(deltaTime, uiElement.content,
                                           uiElement.alpha);
                }
            }
        }
    }
    Text::endDrawing();

    window->Gfx().EndFrame();
}

// ////////////////////////////////////////////////////////////////////////// //
