#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Components/Components.hpp"
#include "ECS/System.hpp"
#include "PostProcessing.h"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(BillboardRenderSystem) {
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    void SetBlackProportion(float black);

    // ============================================================== Data == //
    Window *window;
    std::shared_ptr<PostProcessing> bloom, colorCorrection;
};
// ////////////////////////////////////////////////////////////////////////// //
