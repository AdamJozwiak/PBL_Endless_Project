#pragma once
#define NOMINMAX
// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class GraphSystem;

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(ColliderSystem) {
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;

  public:
    // ========================================================= Behaviour == //
    // Box Collider
    AABB AddAABB(std::vector<DirectX::XMFLOAT3> const& objectVertPos);
    void CalculateAABB(AABB & aabb, DirectX::XMMATRIX const& worldSpace);
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
    // ============================================================== Data == //
  public:
    // Box
    std::vector<unsigned short> boxColliderIndieces;

  private:
    float speed_factor = 1.0f;
    std::shared_ptr<GraphSystem> graphSystem;
};
// ////////////////////////////////////////////////////////////////////////// //
