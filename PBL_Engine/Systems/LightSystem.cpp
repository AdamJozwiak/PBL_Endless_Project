// ///////////////////////////////////////////////////////////////// Includes //
#include "LightSystem.hpp"

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

void LightSystem::setup() {}

void LightSystem::update(float deltaTime) {
    for (Entity entity : entities) {
        auto pointLight = entity.get<Light>().pointLight;
        auto localPos = entity.get<Transform>().position;
        DirectX::XMVECTOR vec = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        vec = XMVector3TransformCoord(
            vec, Registry::instance().system<GraphSystem>()->transform(entity));
        auto camera = Registry::instance()
                          .system<PropertySystem>()
                          ->findEntityByTag("MainCamera")
                          .at(0)
                          .get<MainCamera>()
                          .camera;
        pointLight->setLightPositionWorld(vec);
        pointLight->AddToBuffer(DirectX::XMMatrixIdentity(),
                                camera->GetCameraPos());
        pointLight->Bind(Registry::instance().system<WindowSystem>()->gfx());
        lightIntensity = pointLight->getIntensity();
        if (lightIntensity <= 1.20f || lightIntensity >= 2.0f) {
            lightFactor = -lightFactor;
        }
        lightIntensity += lightFactor;
        pointLight->setIntensity(lightIntensity);
    }
};

void LightSystem::release() {}

// ////////////////////////////////////////////////////////////////////////// //