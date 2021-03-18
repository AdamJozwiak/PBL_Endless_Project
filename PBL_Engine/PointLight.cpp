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

PointLight::~PointLight() {
    torchNumbers.push(number);
    setIntensity(0.0f);
}

DirectX::XMFLOAT4 PointLight::lightPositionWorld() const {
    return lightParametersConstantBuffer.lightPositionWorld[number];
}

void PointLight::setLightPositionWorld(DirectX::XMVECTOR newWorldPos) {
    DirectX::XMFLOAT4 tmp = {
        DirectX::XMVectorGetX(newWorldPos), DirectX::XMVectorGetY(newWorldPos),
        DirectX::XMVectorGetZ(newWorldPos), DirectX::XMVectorGetW(newWorldPos)};
    lightParametersConstantBuffer.lightPositionWorld[number] = tmp;
    if (cameras.at(0) != nullptr) {
        for (auto& camera : cameras) {
            camera->setCameraPos(newWorldPos);
        }
    }
}

void PointLight::setMainLightPosition(DirectX::XMVECTOR pos) {
    DirectX::XMFLOAT4 tmp = {
        DirectX::XMVectorGetX(pos), DirectX::XMVectorGetY(pos),
        DirectX::XMVectorGetZ(pos), DirectX::XMVectorGetW(pos)};
    lightParametersConstantBuffer.mainLightPosition = tmp;
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

void PointLight::setAttenuationC(float const value) {
    attenuationConstant = value;
    // Find the vector which contains the target intensity
    auto& attenuationCVector =
        lightParametersConstantBuffer.attenuationConstant[number / 4];

    // Choose the specific component inside the found vector
    std::optional<std::reference_wrapper<float>> targetAttenuationCComponent;
    switch (number % 4) {
        case 0:
            targetAttenuationCComponent = attenuationCVector.x;
            break;
        case 1:
            targetAttenuationCComponent = attenuationCVector.y;
            break;
        case 2:
            targetAttenuationCComponent = attenuationCVector.z;
            break;
        case 3:
            targetAttenuationCComponent = attenuationCVector.w;
            break;
    }

    // Set the intensity
    targetAttenuationCComponent->get() = value;
}

float PointLight::getAttenuationC() { return attenuationConstant; }

void PointLight::setAttenuationL(float const value) {
    attenuationLinear = value;
    // Find the vector which contains the target intensity
    auto& attenuationLVector =
        lightParametersConstantBuffer.attenuationLinear[number / 4];

    // Choose the specific component inside the found vector
    std::optional<std::reference_wrapper<float>> targetAttenuationLComponent;
    switch (number % 4) {
        case 0:
            targetAttenuationLComponent = attenuationLVector.x;
            break;
        case 1:
            targetAttenuationLComponent = attenuationLVector.y;
            break;
        case 2:
            targetAttenuationLComponent = attenuationLVector.z;
            break;
        case 3:
            targetAttenuationLComponent = attenuationLVector.w;
            break;
    }

    // Set the intensity
    targetAttenuationLComponent->get() = value;
}

float PointLight::getAttenuationL() { return attenuationLinear; }

void PointLight::setAttenuationQ(float const value) {
    attenuationQuadratic = value;
    // Find the vector which contains the target intensity
    auto& attenuationQVector =
        lightParametersConstantBuffer.attenuationQuadratic[number / 4];

    // Choose the specific component inside the found vector
    std::optional<std::reference_wrapper<float>> targetAttenuationQComponent;
    switch (number % 4) {
        case 0:
            targetAttenuationQComponent = attenuationQVector.x;
            break;
        case 1:
            targetAttenuationQComponent = attenuationQVector.y;
            break;
        case 2:
            targetAttenuationQComponent = attenuationQVector.z;
            break;
        case 3:
            targetAttenuationQComponent = attenuationQVector.w;
            break;
    }

    // Set the intensity
    targetAttenuationQComponent->get() = value;
}

float PointLight::getAttenuationQ() { return attenuationQuadratic; }

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

        /*ImGui::Text("Falloff");
        ImGui::SliderFloat("Constant",
                           &lightParametersConstantBuffer.attenuationConstant,
                           0.05f, 10.0f, "%.2f", 4);
        ImGui::SliderFloat("Linear",
                           &lightParametersConstantBuffer.attenuationLinear,
                           0.0001f, 4.0f, "%.4f", 8);
        ImGui::SliderFloat("Quadratic",
                           &lightParametersConstantBuffer.attenuationQuadratic,
                           0.0000001f, 10.0f, "%.7f", 10);*/

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
    setAttenuationC(0.1f);
    setAttenuationL(0.0f);
    setAttenuationQ(0.2f);
    setIntensity(1.0f);
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

    // TODO: A dumb way to pass the light parameters to the vertex shader, fix
    // this somehow
    static VertexConstantBuffer<LightParametersConstantBuffer> c(gfx, 11);
    c.Update(gfx, lightParametersConstantBuffer);
    c.Bind(gfx);
}

void PointLight::AddCameras() {
    auto lightPosition = lightPositionWorld();
    for (int i = 0; i < 6; i++) {
        cameras[i] = std::make_shared<Camera>();
        cameras[i]->setCameraPos(
            DirectX::XMVectorSet(lightPosition.x, lightPosition.y,
                                 lightPosition.z, lightPosition.w));
    }
    cameras[0]->setCameraRotation(0.0f,
                                  DirectX::XMConvertToRadians(90.0f));  // X+

    cameras[1]->setCameraRotation(0.0f,
                                  DirectX::XMConvertToRadians(-90.0f));  // X-
    cameras[2]->setCameraRotation(DirectX::XMConvertToRadians(-90.0f),
                                  DirectX::XMConvertToRadians(0.0f));  // Y+
    cameras[3]->setCameraRotation(DirectX::XMConvertToRadians(90.0f),
                                  0.0f);        // Y-
    cameras[4]->setCameraRotation(0.0f, 0.0f);  // Z+
    cameras[5]->setCameraRotation(0.0f,
                                  DirectX::XMConvertToRadians(180.0f));  // Z-
}

void PointLight::initTorchNumbers() {
    for (int i = 0; i < PointLight::MAX_LIGHT_COUNT; i++) {
        torchNumbers.push(i);
    }
    for (int i = 0; i < MAX_LIGHT_COUNT / 4; i++) {
        lightParametersConstantBuffer.attenuationConstant[i] =
            DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
    }
}
int PointLight::getNumber() { return number; }
// zrobic oddzielna metode przydzielajaca zasoby i ostatnie swiatlo wywoluje,
// moze byc static, bind

void PointLight::setColor(DirectX::XMFLOAT4 const& color) {
    lightParametersConstantBuffer.diffuseColor[number] = color;
}
DirectX::XMFLOAT4 PointLight::getColor() {
    return lightParametersConstantBuffer.diffuseColor[number];
}

std::shared_ptr<Camera> PointLight::getLightCamera(int iterator) {
    return cameras[iterator];
}
