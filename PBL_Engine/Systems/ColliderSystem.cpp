// ///////////////////////////////////////////////////////////////// Includes //
#include "ColliderSystem.hpp"

#include "Cube.h"
#include "Renderable.h"
#include "Window.h"

// ECS
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Systems/RenderSystem.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
BoxCollider ColliderSystem::AddBoxCollider(
    std::vector<DirectX::XMFLOAT3> objectVertPos) {
    DirectX::XMFLOAT3 minVertex = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    BoxCollider boxCollider;
    // Look for the smallest and largest vertex
    for (size_t i = 0; i < objectVertPos.size(); i++) {
        minVertex.x = std::min(minVertex.x, objectVertPos[i].x);
        minVertex.y = std::min(minVertex.y, objectVertPos[i].y);
        minVertex.z = std::min(minVertex.z, objectVertPos[i].z);

        maxVertex.x = std::max(maxVertex.x, objectVertPos[i].x);
        maxVertex.y = std::max(maxVertex.y, objectVertPos[i].y);
        maxVertex.z = std::max(maxVertex.z, objectVertPos[i].z);
    }

    // Store Bounding Box's min and max vertices
    boxCollider.boxColliderMin =
        DirectX::XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
    boxCollider.boxColliderMax =
        DirectX::XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);

    // Front Vertices
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(minVertex.x, minVertex.y, minVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z));

    // Back Vertices
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z));
    boxCollider.boxColliderVerts.push_back(
        DirectX::XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z));

    // Indieces
    unsigned short* i = new unsigned short[36];
    // Front Face
    i[0] = 0;
    i[1] = 1;
    i[2] = 2;
    i[3] = 0;
    i[4] = 2;
    i[5] = 3;

    // Back Face
    i[6] = 4;
    i[7] = 5;
    i[8] = 6;
    i[9] = 4;
    i[10] = 6;
    i[11] = 7;

    // Top Face
    i[12] = 1;
    i[13] = 7;
    i[14] = 6;
    i[15] = 1;
    i[16] = 6;
    i[17] = 2;

    // Bottom Face
    i[18] = 0;
    i[19] = 4;
    i[20] = 5;
    i[21] = 0;
    i[22] = 5;
    i[23] = 3;

    // Left Face
    i[24] = 4;
    i[25] = 7;
    i[26] = 1;
    i[27] = 4;
    i[28] = 1;
    i[29] = 0;

    // Right Face
    i[30] = 3;
    i[31] = 2;
    i[32] = 6;
    i[33] = 3;
    i[34] = 6;
    i[35] = 5;

    for (int j = 0; j < 36; j++) boxColliderIndieces.push_back(i[j]);

    return boxCollider;
}

void ColliderSystem::CalculateAABB(BoxCollider& boxCollider,
                                   DirectX::XMMATRIX const& worldSpace) {
    DirectX::XMFLOAT3 minVertex = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    // Loop through the 8 vertices describing the bounding box
    for (size_t i = 0; i < 8; i++) {
        // Transform the bounding boxes vertices to the objects world space
        DirectX::XMVECTOR Vert =
            DirectX::XMVectorSet(boxCollider.boxColliderVerts[i].x,
                                 boxCollider.boxColliderVerts[i].y,
                                 boxCollider.boxColliderVerts[i].z, 0.0f);
        Vert = XMVector3TransformCoord(Vert, worldSpace);

        // Get the smallest vertex
        minVertex.x = std::min(
            minVertex.x,
            DirectX::XMVectorGetX(Vert));  // Find smallest x value in model
        minVertex.y = std::min(
            minVertex.y,
            DirectX::XMVectorGetY(Vert));  // Find smallest y value in model
        minVertex.z = std::min(
            minVertex.z,
            DirectX::XMVectorGetZ(Vert));  // Find smallest z value in model

        // Get the largest vertex
        maxVertex.x = std::max(
            maxVertex.x,
            DirectX::XMVectorGetX(Vert));  // Find largest x value in model
        maxVertex.y = std::max(
            maxVertex.y,
            DirectX::XMVectorGetY(Vert));  // Find largest y value in model
        maxVertex.z = std::max(
            maxVertex.z,
            DirectX::XMVectorGetZ(Vert));  // Find largest z value in model
    }

    // Store Bounding Box's min and max vertices
    boxCollider.boxColliderMin =
        DirectX::XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
    boxCollider.boxColliderMax =
        DirectX::XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);
}

bool ColliderSystem::CheckBoxesCollision(BoxCollider boxCollider,
                                         BoxCollider differentBoxCollider) {
    // Is obj1's max X greater than obj2's min X? If not, obj1 is to the
    // LEFT of obj2
    if (DirectX::XMVectorGetX(boxCollider.boxColliderMax) >
        DirectX::XMVectorGetX(differentBoxCollider.boxColliderMin))

        // Is obj1's min X less than obj2's max X? If not, obj1 is to the
        // RIGHT of obj2
        if (DirectX::XMVectorGetX(boxCollider.boxColliderMin) <
            DirectX::XMVectorGetX(differentBoxCollider.boxColliderMax))

            // Is obj1's max Y greater than obj2's min Y? If not, obj1 is
            // UNDER obj2
            if (DirectX::XMVectorGetY(boxCollider.boxColliderMax) >
                DirectX::XMVectorGetY(differentBoxCollider.boxColliderMin))

                // Is obj1's min Y less than obj2's max Y? If not, obj1 is
                // ABOVE obj2
                if (DirectX::XMVectorGetY(boxCollider.boxColliderMin) <
                    DirectX::XMVectorGetY(differentBoxCollider.boxColliderMax))

                    // Is obj1's max Z greater than obj2's min Z? If not,
                    // obj1 is IN FRONT OF obj2
                    if (DirectX::XMVectorGetZ(boxCollider.boxColliderMax) >
                        DirectX::XMVectorGetZ(
                            differentBoxCollider.boxColliderMin))

                        // Is obj1's min Z less than obj2's max Z? If not,
                        // obj1 is BEHIND obj2
                        if (DirectX::XMVectorGetZ(boxCollider.boxColliderMin) <
                            DirectX::XMVectorGetZ(
                                differentBoxCollider.boxColliderMax))

                            // If we've made it this far, then the two
                            // bounding boxes are colliding
                            return true;

    // If the two bounding boxes are not colliding, then return false
    return false;
}

SphereCollider ColliderSystem::AddSphereCollider(
    std::vector<DirectX::XMFLOAT3> objectVertPos) {
    SphereCollider sphereCollider;
    DirectX::XMFLOAT3 minVertex = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (size_t i = 0; i < objectVertPos.size(); i++) {
        minVertex.x = std::min(minVertex.x, objectVertPos[i].x);
        minVertex.y = std::min(minVertex.y, objectVertPos[i].y);
        minVertex.z = std::min(minVertex.z, objectVertPos[i].z);

        maxVertex.x = std::max(maxVertex.x, objectVertPos[i].x);
        maxVertex.y = std::max(maxVertex.y, objectVertPos[i].y);
        maxVertex.z = std::max(maxVertex.z, objectVertPos[i].z);
    }

    // Compute models real center
    float distX = (maxVertex.x - minVertex.x) / 2.0f;
    float distY = (maxVertex.y - minVertex.y) / 2.0f;
    float distZ = (maxVertex.z - minVertex.z) / 2.0f;

    sphereCollider.objectCenterOffset = DirectX::XMVectorSet(
        maxVertex.x - distX, maxVertex.y - distY, maxVertex.z - distZ, 0.0f);

    // Compute radius (distance between min and max bounding box vertices)
    // radius = sqrt(distX*distX + distY*distY + distZ*distZ) / 2.0f;
    sphereCollider.radius = DirectX::XMVectorGetX(DirectX::XMVector3Length(
        DirectX::XMVectorSet(distX, distY, distZ, 0.0f)));

    return sphereCollider;
}

bool ColliderSystem::CheckSpheresCollision(
    SphereCollider sphereCollider, DirectX::XMMATRIX const& worldSpace,
    SphereCollider differentSphereCollider,
    DirectX::XMMATRIX const& differentSphereWorldSpace) {
    // Declare local variables
    DirectX::XMVECTOR world_1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR world_2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    float objectsDistance = 0.0f;

    // Transform the objects world space to objects REAL center in world
    // space
    world_1 = DirectX::XMVector3TransformCoord(
        sphereCollider.objectCenterOffset, worldSpace);
    world_2 = DirectX::XMVector3TransformCoord(
        differentSphereCollider.objectCenterOffset, differentSphereWorldSpace);

    // Get the distance between the two objects
    objectsDistance = DirectX::XMVectorGetX(
        DirectX::XMVector3Length(DirectX::XMVectorSubtract(world_1, world_2)));

    // If the distance between the two objects is less than the sum of their
    // bounding spheres...
    if (objectsDistance <=
        (sphereCollider.radius + differentSphereCollider.radius))
        // Return true
        return true;

    // If the bounding spheres are not colliding, return false
    return false;
}

void ColliderSystem::filters() {
    filter<SphereCollider>();
    filter<Renderer>();
}

void ColliderSystem::setup() {}

void ColliderSystem::update(float deltaTime) {
    // Collider Test
    const auto dt = deltaTime * speed_factor;

    std::vector<std::vector<bool>> checkedCollisions;
    for (int i = 0; i < entities.size(); i++) {
        std::vector<bool> tempik;
        for (int j = 0; j < i + 1; j++) {
            tempik.push_back(false);
        }
        checkedCollisions.push_back(tempik);
    }

    for (auto iEntity : entities) {
        for (auto jEntity : entities) {
            if (iEntity.id == jEntity.id) {
                break;
            }
            auto iTransform =
                registry.system<RenderSystem>()->transformMatrix(iEntity);
            auto jTransform =
                registry.system<RenderSystem>()->transformMatrix(jEntity);
            if (CheckSpheresCollision(iEntity.get<SphereCollider>(), iTransform,
                                      jEntity.get<SphereCollider>(),
                                      jTransform)) {
                //checkedCollisions[i][j] = true;
                registry.send(OnCollisionEnter{.a = iEntity, .b = jEntity});
            }
        }
    }
}
