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
void GraphSystem::filters() { filter<Properties>().filter<Transform>(); }

void GraphSystem::setup() { rebuildGraph(); }

void GraphSystem::update(float const deltaTime) { updateGraph(); };

void GraphSystem::release() {}

DirectX::XMMATRIX GraphSystem::transform(Entity const& entity) {
    return entityToGraphNode.at(entity.id).cumulativeTransform;
}

void GraphSystem::destroyEntityWithChildren(Entity const& entity) {
    // Destroy the entity
    registry.destroyEntity(entity);

    // Destroy the entity's children if they exist
    if (!entityToGraphNode.contains(entity.id)) {
        return;
    }
    auto const& children = entityToGraphNode.at(entity.id).children;
    if (!children.empty()) {
        for (auto const& childEntityId : children) {
            destroyEntityWithChildren(childEntityId);
        }
    }
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
    // Rotation (Unity)
    if (angle) {
        result *= dx::XMMatrixRotationAxis(axis, angle);
    }

    // Rotation (game)
    result *= dx::XMMatrixRotationRollPitchYaw(
        transform.euler.x, transform.euler.y, transform.euler.z);

    // Translation
    result *= dx::XMMatrixTranslation(
        transform.position.x, transform.position.y, transform.position.z);

    return result;
}

void GraphSystem::rebuildGraph() {
    resetGraph();
    updateGraph();
}

void GraphSystem::resetGraph() {
    resetRootNode();

    entityToGraphNode.clear();
    entityToPreviousTransform.clear();
    entityToPreviousActivity.clear();

    // Create map entries for all entities (only when needed)
    for (Entity entity : entities) {
        if (!entityToGraphNode.contains(entity.id)) {
            entityToGraphNode.insert(
                {entity.id,
                 {.parent = nullptr,
                  .children = {},
                  .entity = std::make_optional<Entity>(entity.id),
                  .transform = &entity.get<Transform>(),
                  .activity = &entity.get<Properties>().active,
                  .cumulativeTransform = dx::XMMatrixIdentity(),
                  .cumulativeActivity = entity.get<Properties>().active,
                  .recalculateTransforms = true,
                  .recalculateActivity = true}});
            entityToPreviousTransform.insert(
                {entity.id, entity.get<Transform>()});
            entityToPreviousActivity.insert(
                {entity.id, entity.get<Properties>().active});
        }
    }

    // Construct the parent-child relationships between the graph nodes
    for (auto& [entityId, graphNode] : entityToGraphNode) {
        auto const& entity = Entity(entityId);
        // if (!entities.contains(entity)) {
        //    continue;
        //}

        auto const& transform = entity.get<Transform>();

        if (transform.parent) {
            auto const& parentId = *transform.parent;
            auto const& parent = Entity(parentId);

            if (!graphNode.parent) {
                graphNode.parent = &entityToGraphNode.at(parentId);
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
}

void GraphSystem::resetRootNode() {
    root.parent = nullptr;
    root.children.clear();
    root.entity = std::nullopt;
    root.transform = nullptr;
    root.activity = nullptr;
    root.cumulativeTransform = dx::XMMatrixIdentity();
    root.cumulativeActivity = true;
    root.recalculateTransforms = true;
    root.recalculateActivity = true;
}

void GraphSystem::updateGraph() {
    // Check for transformations that need to be recalculated
    for (auto const& [entityId, previousTransform] :
         entityToPreviousTransform) {
        auto const& entity = Entity(entityId);
        // if (!entities.contains(entity)) {
        //    continue;
        //}

        auto const& currentTransform = entity.get<Transform>();

        if (currentTransform != previousTransform) {
            entityToGraphNode.at(entityId).recalculateTransforms = true;
            entityToPreviousTransform.at(entityId) = currentTransform;
        }
    }

    // Check for activities that need to be recalculated
    for (auto const& [entityId, previousActivity] : entityToPreviousActivity) {
        auto const& entity = Entity(entityId);
        // if (!entities.contains(entity)) {
        //    continue;
        //}

        auto const& currentActivity = entity.get<Properties>().active;

        if (currentActivity != previousActivity) {
            entityToGraphNode.at(entityId).recalculateActivity = true;
            entityToPreviousActivity.at(entityId) = currentActivity;
        }
    }

    // Update transformations and activities
    std::queue<std::reference_wrapper<GraphNode>> nodes;
    for (auto const& childEntityId : root.children) {
        nodes.push(std::ref(entityToGraphNode.at(childEntityId)));
    }

    while (!nodes.empty()) {
        auto& node = nodes.front().get();
        nodes.pop();

        if (node.recalculateTransforms) {
            node.cumulativeTransform = dx::XMMatrixIdentity();
            node.cumulativeTransform *= matrix(*node.transform);
            node.cumulativeTransform *= node.parent->cumulativeTransform;
        }
        if (node.recalculateActivity) {
            if (node.parent->cumulativeActivity) {
                node.cumulativeActivity = *node.activity;
            } else {
                node.cumulativeActivity = false;
            }
            if (node.entity->has<Active>()) {
                if (!node.cumulativeActivity) {
                    node.entity->remove<Active>();
                }
            } else {
                if (node.cumulativeActivity) {
                    node.entity->add<Active>({});
                }
            }
        }

        for (auto const& childEntityId : node.children) {
            auto& childGraphNode = entityToGraphNode.at(childEntityId);
            childGraphNode.recalculateTransforms |= node.recalculateTransforms;
            childGraphNode.recalculateActivity |= node.recalculateActivity;
            nodes.push(std::ref(childGraphNode));
        }

        node.recalculateTransforms = false;
        node.recalculateActivity = false;
    }
}

// ////////////////////////////////////////////////////////////////////////// //
