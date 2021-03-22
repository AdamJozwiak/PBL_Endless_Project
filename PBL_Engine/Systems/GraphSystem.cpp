// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "GraphSystem.hpp"

#include <cmath>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "PropertySystem.hpp"
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
    resetRootNode();
}

void GraphSystem::update(float const deltaTime) {
    assert("Entity must have a graph node!" && [&]() {
        for (auto const& entity : entities) {
            if (!entityToGraphNode.contains(entity)) {
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
    assert("Graph node is not valid - parent == nullptr!" && [&]() {
        for (auto& [entityId, graphNode] : entityToGraphNode) {
            if (!graphNode.parent) {
                return false;
            }
        }
        return true;
    }());

    // TODO: weird bug with the menu not moving, this line with the root forces
    // the recalculation of transforms and fixes the bug, but it is not elegant
    // root.recalculateTransforms = true;

    // updateGraph();

    // recalculateActiveStatus(root);

    // for (auto const& node : recalculateActivityNodes) {
    //     recalculateActiveStatus(entityToGraphNode.at(node));
    // }
    // recalculateActivityNodes.clear();
    if (!recalculateActivityNodes.empty()) {
        updateGraph();
        recalculateActivityNodes.clear();
    }
};

void GraphSystem::release() {}

void GraphSystem::onEntityInsert(Entity const& entity, SceneId const& sceneId) {
    LOCK_GUARD(graphSystemMutex, true);

    createNode(entity);
}

void GraphSystem::onEntityErase(Entity const& entity, SceneId const& sceneId) {
    LOCK_GUARD(graphSystemMutex, true);

    destroyNode(entity);
}

DirectX::XMMATRIX GraphSystem::transform(Entity const& entity) {
    LOCK_GUARD(graphSystemMutex, true);

    assert(entityToGraphNode.contains(entity) &&
           "Entity must have a graph node!");

    // if (!entityToGraphNode.at(entity).valid) {
    //     return DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f);
    // }

    if (entityToGraphNode.at(entity).recalculateTransforms) {
        // reconstructParentChildRelationship(entity);
        recalculateTransform(entityToGraphNode.at(entity));
    }

    // assert(!entityToGraphNode.at(entity).recalculateTransforms &&
    //        "Node must have its transforms recalculated!");

    return entityToGraphNode.at(entity).cumulativeTransform;
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

// void GraphSystem::rebuildGraph() {
//     resetGraph();
//     updateGraph();
// }

void GraphSystem::resetGraph() {
    reconstructNodes();
    // reconstructAllRelationships();
    // for (auto& [entityId, graphNode] : entityToGraphNode) {
    //     reconstructParentChildRelationship(entityId);
    // }
}

void GraphSystem::reconstructNodes() {
    resetRootNode();

    entityToGraphNode.clear();
    for (auto const& entity : entities) {
        createNode(entity);
    }
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
    // recalculateActivityNodes.push_back(std::ref(root));
    root.valid = true;

    // recalculateTransform(root);
    // recalculateActiveStatus(root);
}

void GraphSystem::createNode(Entity const& entity) {
    assert(entities.contains(entity) &&
           "Entity must be compliant with the filters of the graph system!");
    // assert(!entityToGraphNode.contains(entity) &&
    //        "Entity must not have a graph node!");

    auto& node = entityToGraphNode[entity];

    node.parent = nullptr;
    // node.children = {}; // possibly contains sth defined in reconstruct
    node.entity.emplace(Entity(entity));
    node.cumulativeTransform = dx::XMMatrixIdentity();
    node.cumulativeActivity = entity.get<Properties>().active;
    node.recalculateTransforms = true;
    node.recalculateActivity = true;
    recalculateActivityNodes.push_back(entity);

    reconstructParentChildRelationship(entity);
    // recalculateTransform(node);
    // recalculateActiveStatus(node);
}

void GraphSystem::destroyNode(Entity const& entity) {
    assert(entityToGraphNode.contains(entity) &&
           "Entity must have a graph node!");

    auto const& entityId = entity;
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
        // childNode.parent = nullptr;
        // childNode.recalculateTransforms = false;
        // childNode.recalculateActivity = false;
    }

    // Node itself is destroyed
    entityToGraphNode.erase(entityId);
}

void GraphSystem::reconstructParentChildRelationship(Entity const& entity) {
    assert(entity.has<Transform>() &&
           "Entity must have a Transform component!");
    assert(entityToGraphNode.contains(entity) &&
           "Entity must have a graph node!");

    auto const& transform = entity.get<Transform>();
    auto& graphNode = entityToGraphNode.at(entity);
    auto& parentNode = transform.parent != std::nullopt
                           ? entityToGraphNode[*transform.parent]
                           : root;

    // Reset the relationship
    if (graphNode.parent) {
        graphNode.parent->children.erase(entity);
        graphNode.parent = nullptr;
    }

    // Recreate the relationship
    graphNode.parent = &parentNode;
    graphNode.parent->children.insert(entity);

    // graphNode.recalculateTransforms = true;
    // graphNode.recalculateActivity = true;
    // recalculateActivityNodes.push_back(entity);

    // if (!transform.parent)
    // recalculateTransform(graphNode);
    // recalculateActiveStatus(graphNode);
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

        // if (node.recalculateTransforms) {
        //     node.cumulativeTransform = dx::XMMatrixIdentity();
        //     node.cumulativeTransform *=
        //         matrix(std::as_const(node.entity)->get<Transform>());
        //     node.cumulativeTransform *= node.parent->cumulativeTransform;
        // }
        if (node.recalculateActivity) {
            if (node.parent->cumulativeActivity) {
                node.cumulativeActivity =
                    std::as_const(node.entity)->get<Properties>().active;
            } else {
                node.cumulativeActivity = false;
            }
            // if (node.cumulativeActivity) {
            //     node.entity->add<Active>({});
            // } else {
            //     node.entity->remove<Active>();
            // }
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
            // childGraphNode.recalculateTransforms |=
            // node.recalculateTransforms;
            childGraphNode.recalculateActivity |= node.recalculateActivity;
            nodes.push(std::ref(childGraphNode));
        }

        // node.recalculateTransforms = false;
        node.recalculateActivity = false;
    }
}

void GraphSystem::recalculateTransform(GraphNode& node) {
    auto const& isRoot = [](GraphNode const& node) {
        return node.entity == std::nullopt;
    };

    if (!isRoot(node) && node.entity->id == 20) {
        int a = 0;
        a++;
        Registry::instance();
    }

    assert(node.recalculateTransforms &&
           "Node must have its transforms to recalculate!");

    // Traverse the tree upwards and save the path
    std::list<std::reference_wrapper<GraphNode>> nodes{std::ref(node)};
    while (nodes.back().get().parent) {
        nodes.push_back(std::ref(*nodes.back().get().parent));
    }

    // Node is valid if it's connected to the root cumulatively with its parents
    bool const valid = isRoot(nodes.back().get());

    assert(valid &&
           "Node must be valid in terms of the cumulative connection to the "
           "root!");
    // if (!valid) {
    //     node.cumulativeTransform = DirectX::XMMatrixScaling(0.0f, 0.0f,
    //     0.0f); return;
    // }

    // Traverse the tree downwards to eliminate unnecessary nodes from the top
    // that don't need to be recalculated
    while (!nodes.back().get().recalculateTransforms) {
        // while (valid && !nodes.back().get().recalculateTransforms) {
        nodes.pop_back();
    }

    nodes.reverse();

    // Flag all the children as targets for further recalculations
    std::list<std::reference_wrapper<GraphNode>> children;
    auto const& pushChildrenBack = [&](auto const& node) {
        for (auto const& childEntityId : node.children) {
            children.push_back(std::ref(entityToGraphNode.at(childEntityId)));
        }
    };

    pushChildrenBack(nodes.front().get());
    while (!children.empty()) {
        auto& child = children.front().get();
        children.pop_front();

        pushChildrenBack(child);

        child.recalculateTransforms = true;
    }

    // Recalculate the transforms (top-down approach)
    for (auto const& node : nodes) {
        node.get().recalculateTransforms = false;

        // todo: don't know {
        // node.get().cumulativeTransform = dx::XMMatrixIdentity();
        //     node.cumulativeTransform *=
        //         matrix(std::as_const(node.entity)->get<Transform>());
        //     node.cumulativeTransform *= node.parent->cumulativeTransform;
        // }

        node.get().cumulativeTransform =
            isRoot(node.get())
                ? dx::XMMatrixIdentity()
                : matrix(std::as_const(node.get().entity)->get<Transform>());

        if (node.get().parent) {
            node.get().cumulativeTransform *=
                node.get().parent->cumulativeTransform;
        }

        // if (!valid) {
        //     node.get().cumulativeTransform =
        //         DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f);
        // }
    }
}

void GraphSystem::recalculateActiveStatus(GraphNode& node) {
    auto const& isRoot = [](GraphNode const& node) { return !node.entity; };

    // assert(node.recalculateActivity &&
    //        "Node must have its active status to recalculate!");

    // Traverse the tree upwards and save the path
    std::list<std::reference_wrapper<GraphNode>> nodes{std::ref(node)};
    while (nodes.back().get().parent) {
        nodes.push_back(std::ref(*nodes.back().get().parent));
    }

    // Traverse the tree downwards to eliminate unnecessary nodes from the top
    // that don't need to be recalculated
    while (!nodes.back().get().recalculateActivity) {
        nodes.pop_back();
    }

    // Recalculate the active statuses (top-down approach)
    nodes.reverse();
    // for (auto const& node : nodes) {
    //     node.get().cumulativeTransform =
    //         isRoot(node.get())
    //             ? dx::XMMatrixIdentity()
    //             : matrix(std::as_const(node.get().entity)->get<Transform>());

    //     if (node.get().parent) {
    //         node.get().cumulativeTransform *=
    //             node.get().parent->cumulativeTransform;
    //     }

    //     node.get().recalculateTransforms = false;
    // }

    // Flag all the children as targets for further recalculations
    std::list<std::reference_wrapper<GraphNode>> children;
    auto const& pushChildrenBack = [&](auto const& node) {
        for (auto const& childEntityId : node.children) {
            children.push_back(std::ref(entityToGraphNode.at(childEntityId)));
        }
    };

    pushChildrenBack(nodes.front().get());
    while (!children.empty()) {
        auto& child = children.front().get();
        children.pop_front();

        if (!isRoot(child)) {
            child.cumulativeActivity =
                child.parent && child.parent->cumulativeActivity &&
                std::as_const(child.entity)->get<Properties>().active;

            if (child.cumulativeActivity) {
                child.entity->add<Active>({});
            } else {
                child.entity->remove<Active>();
            }
        }

        pushChildrenBack(child);

        child.recalculateActivity = false;
    }
}

void GraphSystem::setupEventListeners() {
    registry.listen<OnComponentUpdate<Transform>>(
        MethodListener(GraphSystem::onTransformUpdate));
    registry.listen<OnComponentUpdate<Properties>>(
        MethodListener(GraphSystem::onPropertiesUpdate));
}

void GraphSystem::onTransformUpdate(OnComponent<Transform> const& event) {
    Entity const& entity = event.entityId;

    if (!entityToGraphNode.contains(entity)) {
        return;
    }

    entityToGraphNode.at(entity).recalculateTransforms = true;

    // if (entityToGraphNode.at(entity).parent->entity->id !=
    //     std::as_const(entityToGraphNode.at(entity))
    //         .entity->get<Transform>()
    //         .parent) {
    // reconstructParentChildRelationship(entity);
    // }

    reconstructParentChildRelationship(entity);
    // reconstructAllRelationships();
    // recalculateTransform(entityToGraphNode.at(entity));

    // TODO: weird bug with the menu not moving, don't know how to fix yet
    if (entity.id == 20) {
        recalculateTransform(entityToGraphNode.at(entity));
    }
}

void GraphSystem::onPropertiesUpdate(OnComponent<Properties> const& event) {
    auto const& entity = Entity(event.entityId);

    if (!entityToGraphNode.contains(entity)) {
        return;
    }

    entityToGraphNode.at(entity).recalculateActivity = true;
    recalculateActivityNodes.push_back(entity);

    // recalculateActiveStatus(entityToGraphNode.at(entity));
}

// ////////////////////////////////////////////////////////////////////////// //
