#include "Camera.h"

#include "imgui/imgui.h"

namespace dx = DirectX;

dx::XMMATRIX Camera::GetMatrix() noexcept {
    // Set constants
    constexpr float maxPitchAngleValue = 85.0f;
    constexpr float movementSmoothing = 0.8f;

    // Clip pitch angle values
    pitch = std::clamp(pitch, dx::XMConvertToRadians(-maxPitchAngleValue),
                       dx::XMConvertToRadians(maxPitchAngleValue));

    // Create rotation matrix
    rotation = dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

    // Compute required directions
    right = dx::XMVector3TransformCoord(defaultRight, rotation);
    forward = dx::XMVector3TransformCoord(defaultForward, rotation);

    // Update position
    position =
        dx::XMVectorAdd(position, dx::XMVectorScale(right, moveLeftRight));
    position =
        dx::XMVectorAdd(position, dx::XMVectorScale(forward, moveBackForward));

    // Smoothly slow down the movement
    moveLeftRight = std::lerp(0.0f, moveLeftRight, movementSmoothing);
    moveBackForward = std::lerp(0.0f, moveBackForward, movementSmoothing);

    // Compute target point
    dx::XMVECTOR target = dx::XMVectorAdd(
        position, dx::XMVector3Normalize(
                      XMVector3TransformCoord(defaultForward, rotation)));

    // Return camera's view matrix
    return dx::XMMatrixLookAtLH(position, target,
                                dx::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
}

void Camera::SpawnControlWindow() noexcept {
    if (ImGui::Begin("Camera")) {
        ImGui::Text("Position");
        ImGui::SliderFloat("R", &r, 0.2f, 80.0f, "%.1f");
        ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
        ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
        ImGui::Text("Orientation");
        ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
        ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
        ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
        if (ImGui::Button("Reset")) {
            Reset();
        }
    }
    ImGui::End();
}

void Camera::Reset() noexcept {
    r = 20.0f;
    theta = 0.0f;
    phi = 0.0f;
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
}

DirectX::XMVECTOR Camera::GetCameraPos() const noexcept { return position; }

DirectX::XMFLOAT3 Camera::pos() const noexcept {
    dx::XMFLOAT3 result;
    dx::XMStoreFloat3(&result, position);
    return result;
}

DirectX::XMFLOAT3 Camera::at() const noexcept {
    dx::XMFLOAT3 result;
    dx::XMVECTOR at = dx::XMVector3Normalize(
        XMVector3TransformCoord(defaultForward, rotation));
    dx::XMStoreFloat3(&result, at);
    return result;
}
