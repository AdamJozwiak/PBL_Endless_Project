// ///////////////////////////////////////////////////////////////// Includes //
#include "BillboardRenderSystem.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"
#include "imgui/imgui.h"

// /////////////////////////////////////////////////////////////////// System //
// ----------------------------------------- System's virtual functions -- == //
void BillboardRenderSystem::filters() {
    filter<Active>().filter<Flame>().filter<Light>();
}

void BillboardRenderSystem::setup() {
    window = &registry.system<WindowSystem>()->window();
    bloom = registry.system<RenderSystem>()->bloom;
    colorCorrection = registry.system<RenderSystem>()->colorCorrection;
}

void BillboardRenderSystem::release() {}

void BillboardRenderSystem::update(float deltaTime) {
    // Render billboards
    for (auto torch : entities) {
        auto flame = torch.get<Flame>().fireParticle;
        auto light = torch.get<Light>().pointLight;
        flame->pos = light->lightPositionWorld();
        flame->Draw(window->Gfx());
    }
    {
        if (ImGui::Begin("Simulation Speed")) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    bloom->End();

    colorCorrection->Begin();
    { bloom->Draw(window->Gfx()); }
    colorCorrection->End();

    {
        window->Gfx().BeginFrame(0.07f, 0.0f, 0.12f, false);
        colorCorrection->Draw(window->Gfx());
    }
}

// ////////////////////////////////////////////////////////////////////////// //
