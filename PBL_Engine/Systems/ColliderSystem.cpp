// ///////////////////////////////////////////////////////////////// Includes //
#include "ColliderSystem.hpp"

#include "Cube.h"
#include "Renderable.h"
#include "Window.h"
#include "math.h"

// ECS
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Systems/GraphSystem.hpp"
#include "Systems/RenderSystem.hpp"

DirectX::XMFLOAT3& operator-=(DirectX::XMFLOAT3& a,
                              DirectX::XMFLOAT3 const& b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

DirectX::XMFLOAT3 operator/(DirectX::XMFLOAT3 const& a,
                            DirectX::XMFLOAT3 const& b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
AABB ColliderSystem::AddAABB(
    std::vector<DirectX::XMFLOAT3> const& objectVertPos) {
    DirectX::XMFLOAT3 minVertex = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    AABB aabb;
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
    aabb.vertexMin =
        DirectX::XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
    aabb.vertexMax =
        DirectX::XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);

    // Front Vertices
    aabb.vertices[0] = DirectX::XMFLOAT3(minVertex.x, minVertex.y, minVertex.z);
    aabb.vertices[1] = DirectX::XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z);
    aabb.vertices[2] = DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z);
    aabb.vertices[3] = DirectX::XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z);

    // Back Vertices
    aabb.vertices[4] = DirectX::XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z);
    aabb.vertices[5] = DirectX::XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z);
    aabb.vertices[6] = DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z);
    aabb.vertices[7] = DirectX::XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z);

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

    if (aabbColliderIndices.empty()) {
        for (int j = 0; j < 36; j++) aabbColliderIndices.push_back(i[j]);
    }

    return aabb;
}

void ColliderSystem::CalculateAABB(AABB& aabb,
                                   DirectX::XMMATRIX const& worldSpace) {
    DirectX::XMFLOAT3 minVertex = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    // Loop through the 8 vertices describing the bounding box
    for (size_t i = 0; i < aabb.vertices.size(); i++) {
        // Transform the bounding boxes vertices to the objects world space
        DirectX::XMVECTOR Vert = DirectX::XMVectorSet(
            aabb.vertices[i].x, aabb.vertices[i].y, aabb.vertices[i].z, 0.0f);
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
    aabb.vertexMin =
        DirectX::XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
    aabb.vertexMax =
        DirectX::XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);
}

BoxCollider ColliderSystem::AddBoxCollider(BoxCollider boxCollider) {
    BoxCollider tmpBox = boxCollider;
    DirectX::XMFLOAT3 minVertex =
        DirectX::XMFLOAT3(tmpBox.center.x - 0.5f * tmpBox.size.x,
                          tmpBox.center.y - 0.5f * tmpBox.size.y,
                          tmpBox.center.z - 0.5f * tmpBox.size.z);
    DirectX::XMFLOAT3 maxVertex =
        DirectX::XMFLOAT3(tmpBox.center.x + 0.5f * tmpBox.size.x,
                          tmpBox.center.y + 0.5f * tmpBox.size.y,
                          tmpBox.center.z + 0.5f * tmpBox.size.z);

    // Store Bounding Box's min and max vertices
    tmpBox.aabb.vertexMin =
        DirectX::XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
    tmpBox.aabb.vertexMax =
        DirectX::XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);

    // Front Vertices
    tmpBox.aabb.vertices[0] =
        DirectX::XMFLOAT3(minVertex.x, minVertex.y, minVertex.z);
    tmpBox.aabb.vertices[1] =
        DirectX::XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z);
    tmpBox.aabb.vertices[2] =
        DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z);
    tmpBox.aabb.vertices[3] =
        DirectX::XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z);

    // Back Vertices
    tmpBox.aabb.vertices[4] =
        DirectX::XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z);
    tmpBox.aabb.vertices[5] =
        DirectX::XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z);
    tmpBox.aabb.vertices[6] =
        DirectX::XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z);
    tmpBox.aabb.vertices[7] =
        DirectX::XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z);

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

    if (boxColliderIndices.empty()) {
        for (int j = 0; j < 36; j++) boxColliderIndices.push_back(i[j]);
    }
    return tmpBox;
}

bool ColliderSystem::CheckBoxesCollision(
    BoxCollider const& boxCollider, BoxCollider const& differentBoxCollider) {
    // Is obj1's max X greater than obj2's min X? If not, obj1 is to the
    // LEFT of obj2
    if (DirectX::XMVectorGetX(boxCollider.aabb.vertexMax) >
        DirectX::XMVectorGetX(differentBoxCollider.aabb.vertexMin))

        // Is obj1's min X less than obj2's max X? If not, obj1 is to the
        // RIGHT of obj2
        if (DirectX::XMVectorGetX(boxCollider.aabb.vertexMin) <
            DirectX::XMVectorGetX(differentBoxCollider.aabb.vertexMax))

            // Is obj1's max Y greater than obj2's min Y? If not, obj1 is
            // UNDER obj2
            if (DirectX::XMVectorGetY(boxCollider.aabb.vertexMax) >
                DirectX::XMVectorGetY(differentBoxCollider.aabb.vertexMin))

                // Is obj1's min Y less than obj2's max Y? If not, obj1 is
                // ABOVE obj2
                if (DirectX::XMVectorGetY(boxCollider.aabb.vertexMin) <
                    DirectX::XMVectorGetY(differentBoxCollider.aabb.vertexMax))

                    // Is obj1's max Z greater than obj2's min Z? If not,
                    // obj1 is IN FRONT OF obj2
                    if (DirectX::XMVectorGetZ(boxCollider.aabb.vertexMax) >
                        DirectX::XMVectorGetZ(
                            differentBoxCollider.aabb.vertexMin))

                        // Is obj1's min Z less than obj2's max Z? If not,
                        // obj1 is BEHIND obj2
                        if (DirectX::XMVectorGetZ(boxCollider.aabb.vertexMin) <
                            DirectX::XMVectorGetZ(
                                differentBoxCollider.aabb.vertexMax))

                            // If we've made it this far, then the two
                            // bounding boxes are colliding
                            return true;

    // If the two bounding boxes are not colliding, then return false
    return false;
}

SphereCollider ColliderSystem::AddSphereCollider(
    std::vector<DirectX::XMFLOAT3> const& objectVertPos) {
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
    sphereCollider.radius =
        DirectX::XMVectorGetX(DirectX::XMVector3Length(
            DirectX::XMVectorSet(distX, distY, distZ, 0.0f))) /
        2.0f;

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

DirectX::XMFLOAT3 ColliderSystem::CalculateSeparatingVector(
    BoxCollider const& a, BoxCollider const& b) {
    std::array<float, 2> x, y, z;
    x[0] = (std::abs(DirectX::XMVectorGetX(a.aabb.vertexMax) -
                     DirectX::XMVectorGetX(b.aabb.vertexMin)));
    x[1] = (std::abs(DirectX::XMVectorGetX(b.aabb.vertexMax) -
                     DirectX::XMVectorGetX(a.aabb.vertexMin)));

    y[0] = (std::abs(DirectX::XMVectorGetY(a.aabb.vertexMax) -
                     DirectX::XMVectorGetY(b.aabb.vertexMin)));
    y[1] = (std::abs(DirectX::XMVectorGetY(b.aabb.vertexMax) -
                     DirectX::XMVectorGetY(a.aabb.vertexMin)));

    z[0] = (std::abs(DirectX::XMVectorGetZ(a.aabb.vertexMax) -
                     DirectX::XMVectorGetZ(b.aabb.vertexMin)));
    z[1] = (std::abs(DirectX::XMVectorGetZ(b.aabb.vertexMax) -
                     DirectX::XMVectorGetZ(a.aabb.vertexMin)));

    float minX = std::min(x[0], x[1]), minY = std::min(y[0], y[1]),
          minZ = std::min(z[0], z[1]);

    if (minX < minY && minX < minZ) {
        minY = 0.0f;
        minZ = 0.0f;
    } else if (minY < minX && minY < minZ) {
        minX = 0.0f;
        minZ = 0.0f;
    } else if (minZ < minX && minZ < minY) {
        minX = 0.0f;
        minY = 0.0f;
    }

    return DirectX::XMFLOAT3(((x[0] < x[1]) ? 1 : -1) * minX,
                             ((y[0] < y[1]) ? 1 : -1) * minY,
                             ((z[0] < z[1]) ? 1 : -1) * minZ);
}

void ColliderSystem::filters() {
    filter<Active>();
    filter<BoxCollider>();
}

void ColliderSystem::setup() { graphSystem = registry.system<GraphSystem>(); }

void ColliderSystem::release() {}

void ColliderSystem::update(float deltaTime) {
    for (auto entity : entities) {
        auto& boxCollider = entity.get<BoxCollider>();

        CalculateAABB(boxCollider.aabb, graphSystem->transform(entity));

        boxCollider.separatingVectorSum = {0.0f, 0.0f, 0.0f};
        boxCollider.numberOfCollisions = {0.0f, 0.0f, 0.0f};
    }

    std::set<std::pair<Entity, Entity>> checks;
    for (auto iEntity : entities) {
        for (auto jEntity : entities) {
            if (iEntity.id == jEntity.id) {
                break;
            }
            if (!iEntity.has<CheckCollisions>() &&
                !jEntity.has<CheckCollisions>()) {
                continue;
            }

            checks.insert({iEntity < jEntity ? iEntity : jEntity,
                           iEntity < jEntity ? jEntity : iEntity});
        }
    }

    for (auto [iEntity, jEntity] : checks) {
        auto iBoxCollider = iEntity.get<BoxCollider>();
        auto jBoxCollider = jEntity.get<BoxCollider>();
        if (CheckBoxesCollision(iBoxCollider, jBoxCollider)) {
            registry.send(OnCollisionEnter{
                .a = iEntity,
                .b = jEntity,
                .minSeparatingVector =
                    CalculateSeparatingVector(iBoxCollider, jBoxCollider)});
        }
    }

    for (auto entity : entities) {
        auto& boxCollider = entity.get<BoxCollider>();
        auto& transform = entity.get<Transform>();

        if (boxCollider.numberOfCollisions.x == 0.0f) {
            boxCollider.numberOfCollisions.x = 1.0f;
        }
        if (boxCollider.numberOfCollisions.y == 0.0f) {
            boxCollider.numberOfCollisions.y = 1.0f;
        }
        if (boxCollider.numberOfCollisions.z == 0.0f) {
            boxCollider.numberOfCollisions.z = 1.0f;
        }

        transform.position -=
            boxCollider.separatingVectorSum / boxCollider.numberOfCollisions;
    }
}
