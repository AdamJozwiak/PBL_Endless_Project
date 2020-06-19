// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "LightSystem.hpp"

#include <cmath>

#include "../Camera.h"
#include "../Window.h"
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void LightSystem::filters() {
    filter<Transform>();
    filter<Light>();
}

void LightSystem::setup() {
    baseLightIntensity = 1.5f;
    timer = 0.0f;
    camera = registry.system<PropertySystem>()
                 ->findEntityByTag("MainCamera")
                 .at(0)
                 .get<MainCamera>()
                 .camera;
}

void LightSystem::update(float deltaTime) {
    timer += deltaTime;

    for (Entity entity : entities) {
        auto pointLight = entity.get<Light>().pointLight;
        DirectX::XMVECTOR vec = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        vec = XMVector3TransformCoord(
            vec, Registry::instance().system<GraphSystem>()->transform(entity));
        pointLight->setLightPositionWorld(vec);
        pointLight->AddToBuffer(DirectX::XMMatrixIdentity(),
                                camera->GetCameraPos());
        pointLight->Bind(Registry::instance().system<WindowSystem>()->gfx());
        if (entity.get<Properties>().name != "Player Torch") {
            pointLight->setIntensity(
                baseLightIntensity +
                baseLightIntensity *
                    oscillator(timer + pointLight->getNumber()));
        }
    }
};

void LightSystem::release() {}

float LightSystem::oscillator(float const x) {
    static constexpr float A = 0.7f, PHI = 1.2f, T = 3.0f;
    return A *
           ((A * std::pow(
                     std::sin((2.0f * M_PI / (T + 2.0f * std::sin(3.0f * x))) *
                                  (std::sin(0.2f * x * std::pow(T, 0.4f))) +
                              PHI),
                     2.0f) -
             0.5f * A) *
                (std::sin(5.0f * x)) +
            (1.0f / std::pow(T, 2.0f)) * std::cos(x) -
            0.2f * std::log(std::pow(std::sin(x), 2.0f) + 0.1f));
};

// ////////////////////////////////////////////////////////////////////////// //
