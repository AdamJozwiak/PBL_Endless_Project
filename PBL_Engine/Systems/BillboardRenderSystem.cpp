// ///////////////////////////////////////////////////////////////// Includes //
#include "BillboardRenderSystem.hpp"

#include <DirectXMath.h>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "GraphSystem.hpp"
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"
#include "imgui/imgui.h"

// /////////////////////////////////////////////////////////////////// System //
// ----------------------------------------- System's virtual functions -- == //
void BillboardRenderSystem::filters() {
    filter<Active>().filter<Transform>().filter<Properties>().filter<Flame>();
}

void BillboardRenderSystem::setup() {
    window = &registry.system<WindowSystem>()->window();
    bloom = registry.system<RenderSystem>()->bloom;
    colorCorrection = registry.system<RenderSystem>()->colorCorrection;
}

void BillboardRenderSystem::release() {}

void BillboardRenderSystem::update(float deltaTime) {
    // Render billboards
    for (auto entity : entities) {
        auto const &flame = entity.get<Flame>().fireParticle;

        DirectX::XMVECTOR positionWorldVector =
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        positionWorldVector = DirectX::XMVector3TransformCoord(
            positionWorldVector,
            Registry::instance().system<GraphSystem>()->transform(entity));

        DirectX::XMFLOAT4 positionWorld;
        DirectX::XMStoreFloat4(&positionWorld, positionWorldVector);

        auto const &tag = entity.get<Properties>().tag;
        if (tag == "Torch") {
            auto const &light = entity.get<Light>().pointLight;
            flame->pos = light->lightPositionWorld();
        } else {
            flame->pos = DirectX::XMFLOAT4{
                positionWorld.x, positionWorld.y - 0.2f, positionWorld.z, 0.0f};
        }
        flame->Draw(window->Gfx());
    }
    {
        // if (ImGui::Begin("Simulation Speed")) {
        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        //                 1000.0f / ImGui::GetIO().Framerate,
        //                 ImGui::GetIO().Framerate);
        // }
        // ImGui::End();
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

void BillboardRenderSystem::setBlackProportion(float proportion) {
    colorCorrection->SetBlackProportion(proportion);
}
float BillboardRenderSystem::getBlackProportion() {
    return colorCorrection->GetBlackProportion();
}

// ////////////////////////////////////////////////////////////////////////// //
