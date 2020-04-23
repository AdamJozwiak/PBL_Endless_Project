#pragma once
#define NOMINMAX
// ///////////////////////////////////////////////////////////////// Includes //
// ECS
#include "Components/Components.hpp"
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(ColliderSystem) {
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;

  public:
    // ========================================================= Behaviour == //
    // Box Collider
    BoxCollider AddBoxCollider(std::vector<DirectX::XMFLOAT3> objectVertPos);
    void CalculateAABB(BoxCollider & boxCollider,
                       DirectX::XMMATRIX const& worldSpace);
    bool CheckBoxesCollision(BoxCollider boxCollider,
                             BoxCollider differentBoxCollider);
    DirectX::XMVECTOR GetColliderMin();
    DirectX::XMVECTOR GetColliderMax();

    // Sphere Collider
    SphereCollider AddSphereCollider(
        std::vector<DirectX::XMFLOAT3> objectVertPos);
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
};
// ////////////////////////////////////////////////////////////////////////// //