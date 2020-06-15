#include "PointLight.h"

#include <optional>

#include "imgui/imgui.h"

PointLight::LightParametersConstantBuffer
    PointLight::lightParametersConstantBuffer;

FixedQueue<int, PointLight::MAX_LIGHT_COUNT> PointLight::torchNumbers;

PointLight::PointLight(Graphics& gfx, float radius) : mesh(gfx, radius) {
    number = PointLight::torchNumbers.pop();
    Reset();
}

PointLight::~PointLight() { torchNumbers.push(number); }

DirectX::XMFLOAT4 PointLight::lightPositionWorld() const {
    return lightParametersConstantBuffer.lightPositionWorld[number];
}

void PointLight::setLightPositionWorld(DirectX::XMVECTOR newWorldPos) {
    DirectX::XMFLOAT4 tmp = {
        DirectX::XMVectorGetX(newWorldPos), DirectX::XMVectorGetY(newWorldPos),
        DirectX::XMVectorGetZ(newWorldPos), DirectX::XMVectorGetW(newWorldPos)};
    lightParametersConstantBuffer.lightPositionWorld[number] = tmp;
}

void PointLight::setIntensity(float const intensity) {
    lightIntensity = intensity;
    // Find the vector which contains the target intensity
    auto& intensityVector = lightParametersConstantBuffer.intensity[number / 4];

    // Choose the specific component inside the found vector
    std::optional<std::reference_wrapper<float>> targetIntensityComponent;
    switch (number % 4) {
        case 0:
            targetIntensityComponent = intensityVector.x;
            break;
        case 1:
            targetIntensityComponent = intensityVector.y;
            break;
        case 2:
            targetIntensityComponent = intensityVector.z;
            break;
        case 3:
            targetIntensityComponent = intensityVector.w;
            break;
    }

    // Set the intensity
    targetIntensityComponent->get() = intensity;
}

float PointLight::getIntensity() { return lightIntensity; }

void PointLight::setAttenuationQ(float value) {
    lightParametersConstantBuffer.attenuationQuadratic = value;
}

float PointLight::getAttenuationQ() {
    return lightParametersConstantBuffer.attenuationQuadratic;
}

void PointLight::SpawnControlWindow() noexcept {
    if (ImGui::Begin("Light")) {
        ImGui::Text("Position");
        ImGui::SliderFloat(
            "X", &lightParametersConstantBuffer.lightPositionWorld[number].x,
            -60.0f, 60.0f, "%.1f");
        ImGui::SliderFloat(
            "Y", &lightParametersConstantBuffer.lightPositionWorld[number].y,
            -60.0f, 60.0f, "%.1f");
        ImGui::SliderFloat(
            "Z", &lightParametersConstantBuffer.lightPositionWorld[number].z,
            -60.0f, 60.0f, "%.1f");

        ImGui::Text("Intensity/Color");
        /* ImGui::SliderFloat("Intensity", */
        /*                    &lightParametersConstantBuffer.diffuseIntensity,
         */
        /*                    0.01f, 2.0f, "%.2f", 2); */
        ImGui::ColorEdit3(
            "Diffuse Color",
            &PointLight::lightParametersConstantBuffer.diffuseColor[number].x);
        /* ImGui::ColorEdit3("Ambient",
         * &lightParametersConstantBuffer.ambient.x); */

        ImGui::Text("Falloff");
        ImGui::SliderFloat("Constant",
                           &lightParametersConstantBuffer.attenuationConstant,
                           0.05f, 10.0f, "%.2f", 4);
        ImGui::SliderFloat("Linear",
                           &lightParametersConstantBuffer.attenuationLinear,
                           0.0001f, 4.0f, "%.4f", 8);
        ImGui::SliderFloat("Quadratic",
                           &lightParametersConstantBuffer.attenuationQuadratic,
                           0.0000001f, 10.0f, "%.7f", 10);

        if (ImGui::Button("Reset")) {
            Reset();
        }
    }
    ImGui::End();
}

void PointLight::Reset() noexcept {
    lightParametersConstantBuffer.lightPositionWorld[number] = {30.0f, 1.0f,
                                                                0.0f, 0.0f};
    lightParametersConstantBuffer.viewPositionWorld = {0.0f, 0.0f, 0.0f, 0.0f};
    lightParametersConstantBuffer.diffuseColor[number] = {1.0f, 0.8f, 0.6f,
                                                          1.0f};
    lightParametersConstantBuffer.attenuationConstant = 0.0f;
    lightParametersConstantBuffer.attenuationLinear = 0.0f;
    lightParametersConstantBuffer.attenuationQuadratic = 0.025f;
    setIntensity(1.5f);
}

void PointLight::AddToBuffer(DirectX::FXMMATRIX view,
                             DirectX::XMVECTOR cameraWorldPosition) {
    const auto lightPosition = DirectX::XMLoadFloat4(
        &lightParametersConstantBuffer.lightPositionWorld[number]);
    DirectX::XMStoreFloat4(
        &lightParametersConstantBuffer.lightPositionWorld[number],
        DirectX::XMVector3Transform(lightPosition, view));
    DirectX::XMStoreFloat4(&lightParametersConstantBuffer.viewPositionWorld,
                           cameraWorldPosition);
}

void PointLight::Draw(Graphics& gfx) const noexcept(!IS_DEBUG) {
    auto const& lightPositionWorld =
        lightParametersConstantBuffer.lightPositionWorld[number];
    mesh.SetPos(
        {lightPositionWorld.x, lightPositionWorld.y, lightPositionWorld.z});
    mesh.Draw(gfx);
}

void PointLight::Bind(Graphics& gfx) noexcept {
    static PixelConstantBuffer<LightParametersConstantBuffer> cbuf(gfx, 10);
    cbuf.Update(gfx, lightParametersConstantBuffer);
    cbuf.Bind(gfx);
}

void PointLight::initTorchNumbers() {
    for (int i = 0; i < PointLight::MAX_LIGHT_COUNT; i++) {
        torchNumbers.push(i);
    }
}
int PointLight::getNumber() { return number; }
// zrobic oddzielna metode przydzielajaca zasoby i ostatnie swiatlo wywoluje,
// moze byc static, bind
