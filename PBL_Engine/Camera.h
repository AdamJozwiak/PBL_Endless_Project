#pragma once
#include "Components/Transform.hpp"
#include "Graphics.h"

class Camera {
  public:
    DirectX::XMMATRIX GetFreeMatrix() noexcept;
    DirectX::XMMATRIX GetMatrix(Transform const &transform) noexcept;
    DirectX::XMMATRIX GetCameraMatrix(
        DirectX::XMMATRIX const &transform) noexcept;
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
    DirectX::XMVECTOR GetCameraPos() const noexcept;
    void setCameraPos(DirectX::XMFLOAT3 const &newPosition);
    void setCameraPos(DirectX::XMVECTOR const &newPosition);
    DirectX::XMFLOAT3 pos() const noexcept;
    DirectX::XMFLOAT3 at() const noexcept;
    void setCameraRotation(float pitch, float yaw);

  public:
    float r = 0.0f;  // distance from the origin
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;

    DirectX::XMVECTOR position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    DirectX::XMVECTOR defaultForward =
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
    DirectX::XMVECTOR defaultRight =
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
    DirectX::XMVECTOR right = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

    DirectX::XMMATRIX rotation;
    DirectX::XMMATRIX groundWorld;

    float moveLeftRight = 0.0f;
    float moveBackForward = 0.0f;
};
