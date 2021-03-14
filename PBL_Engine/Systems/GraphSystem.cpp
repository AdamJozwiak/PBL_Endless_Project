// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "GraphSystem.hpp"

#include <cmath>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "assert.hpp"

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

void GraphSystem::setup() {
    setupEventListeners();
    rebuildGraph();
}

void GraphSystem::update(float const deltaTime) {
    refreshGraph();
    updateGraph();
};

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

    for (auto const& entity : entities) {
        createNode(entity);
    }

    for (auto& [entityId, graphNode] : entityToGraphNode) {
        reconstructParentChildRelationship(entityId);
    }
}

void GraphSystem::refreshGraph() {
    // Create nodes for new entities
    for (auto const& entity : createdEntities) {
        if (entities.contains(entity)) {
            if (entityToGraphNode.contains(entity)) {
                destroyNode(entity);
            }
            createNode(entity);
        }
    }

    // Reconstruct the parent-child relationships for new entities
    if (!createdEntities.empty()) {
        createdEntities.clear();
        reconstructAllRelationships();
    }

    assert("Entity must have a graph node!" && [&]() {
        for (auto const& entity : entities) {
            if (!entityToGraphNode.contains(entity.id)) {
                return false;
            }
        }
        return true;
    }());
    assert("Graph node is not valid - entity!" && [&]() {
        for (auto& [entityId, graphNode] : entityToGraphNode) {
            if (entityId != graphNode.entity->id) {
                return false;
            }
        }
        return true;
    }());
}

void GraphSystem::reconstructAllRelationships() {
    root.parent = nullptr;
    root.children.clear();

    for (auto& [entityId, graphNode] : entityToGraphNode) {
        graphNode.parent = nullptr;
        graphNode.children.clear();
    }

    for (auto& [entityId, graphNode] : entityToGraphNode) {
        reconstructParentChildRelationship(entityId);
    }
}

void GraphSystem::resetRootNode() {
    root.parent = nullptr;
    root.children.clear();
    root.entity = std::nullopt;
    root.cumulativeTransform = dx::XMMatrixIdentity();
    root.cumulativeActivity = true;
    root.recalculateTransforms = true;
    root.recalculateActivity = true;
}

void GraphSystem::createNode(Entity const& entity) {
    assert(entities.contains(entity.id) &&
           "Entity must be compliant with the filters of the graph system!");
    assert(!entityToGraphNode.contains(entity.id) &&
           "Entity must not have a graph node!");

    entityToGraphNode.insert(
        {entity.id,
         {.parent = nullptr,
          .children = {},
          .entity = std::make_optional<Entity>(entity.id),
          .cumulativeTransform = dx::XMMatrixIdentity(),
          .cumulativeActivity = entity.get<Properties>().active,
          .recalculateTransforms = true,
          .recalculateActivity = true}});
}

void GraphSystem::destroyNode(Entity const& entity) {
    assert(entities.contains(entity.id) &&
           "Entity must be compliant with the filters of the graph system!");
    assert(entityToGraphNode.contains(entity.id) &&
           "Entity must have a graph node!");

    auto const& entityId = entity.id;
    auto const& node = entityToGraphNode.at(entityId);

    // Parent loses a child
    if (node.parent) {
        node.parent->children.erase(entityId);
    }

    // Children get a new parent
    for (auto const& childEntityId : node.children) {
        auto& childNode = entityToGraphNode.at(childEntityId);
        childNode.parent = node.parent;
        childNode.recalculateTransforms = true;
        childNode.recalculateActivity = true;
    }

    // Node itself is destroyed
    entityToGraphNode.erase(entityId);
}

void GraphSystem::reconstructParentChildRelationship(Entity const& entity) {
    assert(entity.has<Transform>() &&
           "Entity must have a Transform component!");
    assert(entityToGraphNode.contains(entity.id) &&
           "Entity must have a graph node!");

    auto const& transform = entity.get<Transform>();
    auto& graphNode = entityToGraphNode.at(entity.id);
    auto& parentNode =
        transform.parent ? entityToGraphNode.at(*transform.parent) : root;

    graphNode.parent = &parentNode;
    graphNode.parent->children.insert(entity.id);

    graphNode.recalculateTransforms = true;
    graphNode.recalculateActivity = true;
}

void GraphSystem::updateGraph() {
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
            node.cumulativeTransform *=
                matrix(std::as_const(node.entity)->get<Transform>());
            node.cumulativeTransform *= node.parent->cumulativeTransform;
        }
        if (node.recalculateActivity) {
            if (node.parent->cumulativeActivity) {
                node.cumulativeActivity =
                    std::as_const(node.entity)->get<Properties>().active;
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

void GraphSystem::setupEventListeners() {
    registry.listen<OnComponentUpdate<Transform>>(
        MethodListener(GraphSystem::onTransformUpdate));
    registry.listen<OnComponentUpdate<Properties>>(
        MethodListener(GraphSystem::onPropertiesUpdate));

    registry.listen<OnEntityCreate>(
        MethodListener(GraphSystem::onEntityCreate));
    registry.listen<OnEntityDestroy>(
        MethodListener(GraphSystem::onEntityDestroy));
}

void GraphSystem::onTransformUpdate(OnComponent<Transform> const& event) {
    auto const& entity = Entity(event.entityId);

    if (!entityToGraphNode.contains(entity.id)) {
        return;
    }

    entityToGraphNode.at(entity.id).recalculateTransforms = true;
}

void GraphSystem::onPropertiesUpdate(OnComponent<Properties> const& event) {
    auto const& entity = Entity(event.entityId);

    if (!entityToGraphNode.contains(entity.id)) {
        return;
    }

    entityToGraphNode.at(entity.id).recalculateActivity = true;
}

void GraphSystem::onEntityCreate(OnEntity const& event) {
    createdEntities.push_back(event.entityId);
}

void GraphSystem::onEntityDestroy(OnEntity const& event) {
    if (entities.contains(event.entityId)) {
        destroyNode(event.entityId);
    }
}

// ////////////////////////////////////////////////////////////////////////// //
