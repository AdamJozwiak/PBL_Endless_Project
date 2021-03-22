#pragma once
#define NOMINMAX
// ///////////////////////////////////////////////////////////////// Includes //

#include <DirectXMath.h>

#include <memory>

// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
ECS_SYSTEM_FORWARD(GraphSystem);
ECS_SYSTEM_FORWARD(CheckCollisionsSystem);

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(ColliderSystem) {
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

  public:
    // ========================================================= Behaviour == //
    // AABB
    AABB AddAABB(std::vector<DirectX::XMFLOAT3> const& objectVertPos);
    void CalculateAABB(AABB & aabb, DirectX::XMMATRIX const& worldSpace);

    // Box Collider
    BoxCollider AddBoxCollider(BoxCollider boxCollider);
    bool CheckBoxesCollision(BoxCollider const& boxCollider,
                             BoxCollider const& differentBoxCollider);
    DirectX::XMVECTOR GetColliderMin();
    DirectX::XMVECTOR GetColliderMax();

    // Sphere Collider
    SphereCollider AddSphereCollider(
        std::vector<DirectX::XMFLOAT3> const& objectVertPos);
    bool CheckSpheresCollision(
        SphereCollider sphereCollider, DirectX::XMMATRIX const& worldSpace,
        SphereCollider differentSphereCollider,
        DirectX::XMMATRIX const& differentSphereWorldSpace);

    DirectX::XMFLOAT3 CalculateSeparatingVector(
        BoxCollider const& boxCollider,
        BoxCollider const& differentBoxCollider);
    // ============================================================== Data == //
  public:
    // Box
    std::vector<unsigned short> boxColliderIndices;
    std::vector<unsigned short> aabbColliderIndices;

  private:
    float speed_factor = 1.0f;
    std::shared_ptr<GraphSystem> graphSystem;
    std::shared_ptr<CheckCollisionsSystem> checkCollisionsSystem;
};
// ////////////////////////////////////////////////////////////////////////// //
