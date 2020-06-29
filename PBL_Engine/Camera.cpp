#include "Camera.h"

#include "imgui/imgui.h"

namespace dx = DirectX;

dx::XMMATRIX Camera::GetFreeMatrix() noexcept {
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

dx::XMMATRIX Camera::GetMatrix(Transform const &transform) noexcept {
    // Set position
    setCameraPos(transform.position);

    // Convert rotation quaternion into axis-angle representation
    dx::XMVECTOR quaternion =
        dx::XMVectorSet(transform.rotation.x, transform.rotation.y,
                        transform.rotation.z, transform.rotation.w);

    dx::XMVECTOR axis;
    float angle;
    dx::XMQuaternionToAxisAngle(&axis, &angle, quaternion);

    rotation = dx::XMMatrixIdentity();
    if (angle) {
        rotation = dx::XMMatrixRotationAxis(axis, angle);
    }

    // Compute required directions
    right = dx::XMVector3TransformCoord(defaultRight, rotation);
    forward = dx::XMVector3TransformCoord(defaultForward, rotation);

    // Compute target point
    dx::XMVECTOR target = dx::XMVectorAdd(
        position, dx::XMVector3Normalize(
                      XMVector3TransformCoord(defaultForward, rotation)));

    // Return camera's view matrix
    return dx::XMMatrixLookAtLH(position, target,
                                dx::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
}

DirectX::XMMATRIX Camera::GetCameraMatrix(
    DirectX::XMMATRIX const &transform) noexcept {
    // Set position
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    pos = XMVector3TransformCoord(pos, transform);
    setCameraPos(pos);

    // Create rotation matrix
    rotation = dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

    // Compute required directions
    right = dx::XMVector3TransformCoord(defaultRight, rotation);
    forward = dx::XMVector3TransformCoord(defaultForward, rotation);
    DirectX::XMVECTOR up = dx::XMVector3TransformCoord(
        dx::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), rotation);

    // Compute target point
    dx::XMVECTOR target = dx::XMVectorAdd(
        position, dx::XMVector3Normalize(
                      XMVector3TransformCoord(defaultForward, rotation)));

    // Return camera's view matrix
    return dx::XMMatrixLookAtLH(position, target, up);
}

void Camera::Reset() noexcept {
    r = 0.0f;
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
}

DirectX::XMVECTOR Camera::GetCameraPos() const noexcept { return position; }

void Camera::setCameraPos(DirectX::XMFLOAT3 const &newPosition) {
    position = dx::XMLoadFloat3(&newPosition);
}

void Camera::setCameraPos(DirectX::XMVECTOR const &newPosition) {
    position = newPosition;
}

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

void Camera::setCameraRotation(float pitch, float yaw) {
    this->pitch = pitch;
    this->yaw = yaw;
}
