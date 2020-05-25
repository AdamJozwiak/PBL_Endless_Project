// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "GraphSystem.hpp"

#include <cmath>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////// Namespaces //
namespace dx = DirectX;

// //////////////////////////////////////////////////////////////// Operators //
bool operator==(dx::XMFLOAT3 const& a, dx::XMFLOAT3 const& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator==(dx::XMFLOAT4 const& a, dx::XMFLOAT4 const& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool operator==(Transform const& a, Transform const& b) {
    return a.parent == b.parent && a.rotation == b.rotation &&
           a.position == b.position && a.scale == b.scale && a.euler == b.euler;
}

bool operator!=(Transform const& a, Transform const& b) { return !(a == b); }

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void GraphSystem::filters() { filter<Transform>(); }

void GraphSystem::setup() {
    // Set default values for the root node
    root.parent = nullptr;
    root.children.clear();
    root.transform = nullptr;
    root.cumulativeTransform = dx::XMMatrixIdentity();
    root.recalculate = true;
}

void GraphSystem::update(float const deltaTime) {
    // Create map entries for all entities (only when needed)
    for (Entity entity : entities) {
        if (!entityToGraphNode.contains(entity.id)) {
            entityToGraphNode.insert(
                {entity.id,
                 {.parent = nullptr,
                  .children = {},
                  .transform = &entity.get<Transform>(),
                  .cumulativeTransform = dx::XMMatrixIdentity(),
                  .recalculate = true}});
            entityToPreviousTransform.insert(
                {entity.id, entity.get<Transform>()});
        }
    }

    // Construct the parent-child relationships between the graph nodes
    for (auto& [entityId, graphNode] : entityToGraphNode) {
        auto const& entity = Entity(entityId);
        if (!entities.contains(entity)) {
            continue;
        }

        auto const& transform = entity.get<Transform>();

        if (transform.parent) {
            auto const& parentId = *transform.parent;
            auto const& parent = Entity(parentId);

            if (!graphNode.parent) {
                graphNode.parent = &entityToGraphNode[parentId];
            }
            if (!graphNode.parent->children.contains(entityId)) {
                graphNode.parent->children.insert(entityId);
            }
        } else {
            if (!graphNode.parent) {
                graphNode.parent = &root;
            }
            if (!root.children.contains(entityId)) {
                root.children.insert(entityId);
            }
        }
    }

    // Check for transformations that need to be recalculated
    for (auto const& [entityId, previousTransform] :
         entityToPreviousTransform) {
        auto const& entity = Entity(entityId);
        if (!entities.contains(entity)) {
            continue;
        }

        auto const& currentTransform = entity.get<Transform>();

        if (currentTransform != previousTransform) {
            entityToGraphNode.at(entityId).recalculate = true;
        }
    }

    // Update transformations
    std::queue<std::reference_wrapper<GraphNode>> nodes;
    for (auto const& childEntityId : root.children) {
        nodes.push(std::ref(entityToGraphNode[childEntityId]));
    }

    while (!nodes.empty()) {
        auto& node = nodes.front().get();
        nodes.pop();

        if (node.recalculate) {
            node.cumulativeTransform = dx::XMMatrixIdentity();
            if (node.children.empty()) {
                node.cumulativeTransform *= dx::XMMatrixRotationRollPitchYaw(
                    dx::XMConvertToRadians(270.0f),
                    dx::XMConvertToRadians(180.0f),
                    dx::XMConvertToRadians(0.0f));
            }
            node.cumulativeTransform *= matrix(*node.transform);
            node.cumulativeTransform *= node.parent->cumulativeTransform;
        }

        for (auto const& childEntityId : node.children) {
            auto& childGraphNode = entityToGraphNode[childEntityId];
            childGraphNode.recalculate |= node.recalculate;
            nodes.push(std::ref(childGraphNode));
        }

        node.recalculate = false;
    }
};

DirectX::XMMATRIX GraphSystem::transform(Entity const& entity) {
    return entityToGraphNode[entity.id].cumulativeTransform;
}

DirectX::XMMATRIX GraphSystem::matrix(Transform const& transform) {
    // Convert rotation quaternion into axis-angle representation
    dx::XMVECTOR quaternion =
        dx::XMVectorSet(transform.rotation.x, transform.rotation.y,
                        transform.rotation.z, transform.rotation.w);

    dx::XMVECTOR axis;
    float angle;
    dx::XMQuaternionToAxisAngle(&axis, &angle, quaternion);

    // Combine transformations
    dx::XMMATRIX result = dx::XMMatrixIdentity();

    // Scaling
    result *= dx::XMMatrixScaling(transform.scale.x, transform.scale.y,
                                  transform.scale.z);
    // Rotation
    if (angle) {
        result *= dx::XMMatrixRotationAxis(axis, angle);
    }

    // Translation
    result *= dx::XMMatrixTranslation(
        transform.position.x, transform.position.y, transform.position.z);

    return result;
}

// ////////////////////////////////////////////////////////////////////////// //
