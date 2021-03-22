#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <Components/Transform.hpp>
#include <Events/OnComponent.hpp>
#include <Events/OnEntity.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(GraphSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    void onEntityInsert(Entity const &entity, SceneId const &sceneId) override;
    void onEntityErase(Entity const &entity, SceneId const &sceneId) override;

    // ----------------------------------------------- Public interface -- == //
    // void rebuildGraph();

    DirectX::XMMATRIX transform(Entity const &entity);

    template <bool threadSafe = true>
    void destroyEntityWithChildren(Entity const &entity) {
        LOCK_GUARD(graphSystemMutex, threadSafe);

        // Destroy the entity
        registry.destroyEntity(entity);

        // Destroy the entity's children if they exist
        if (!entityToGraphNode.contains(entity.id)) {
            return;
        }
        auto const &children = entityToGraphNode.at(entity.id).children;
        if (!children.empty()) {
            for (auto const &childEntityId : children) {
                destroyEntityWithChildren<false>(childEntityId);
            }
        }
    }

  private:
    struct GraphNode {
        GraphNode *parent;
        std::set<EntityId> children;

        std::optional<Entity> entity;
        DirectX::XMMATRIX cumulativeTransform;
        bool cumulativeActivity;
        bool recalculateTransforms, recalculateActivity;
        bool valid;
    };
    // ========================================================= Behaviour == //
    DirectX::XMMATRIX matrix(Transform const &transform);

    void resetGraph();
    void resetRootNode();
    void createNode(Entity const &entity);
    void destroyNode(Entity const &entity);
    void reconstructNodes();
    void reconstructParentChildRelationship(Entity const &entity);
    void reconstructAllRelationships();

    void updateGraph();
    void recalculateTransform(GraphNode & node);
    void recalculateActiveStatus(GraphNode & node);

    void setupEventListeners();
    void onTransformUpdate(OnComponent<Transform> const &event);
    void onPropertiesUpdate(OnComponent<Properties> const &event);

    // ============================================================== Data == //
    GraphNode root;
    std::unordered_map<EntityId, GraphNode> entityToGraphNode;
    std::unordered_map<EntityId, Transform> entityToPreviousTransform;
    std::unordered_map<EntityId, bool> entityToPreviousActivity;
    std::list<EntityId> recalculateActivityNodes;
    std::mutex graphSystemMutex;
};

// ////////////////////////////////////////////////////////////////////////// //
