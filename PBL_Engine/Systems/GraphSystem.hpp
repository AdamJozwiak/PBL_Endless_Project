#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include <Components/Transform.hpp>
#include <Events/OnComponent.hpp>
#include <Events/OnEntity.hpp>
#include <memory>
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

    // ----------------------------------------------- Public interface -- == //
    void rebuildGraph();
    void refreshGraph();

    DirectX::XMMATRIX transform(Entity const &entity);
    void destroyEntityWithChildren(Entity const &entity);

  private:
    // ========================================================= Behaviour == //
    DirectX::XMMATRIX matrix(Transform const &transform);

    void resetGraph();
    void resetRootNode();
    void createNode(Entity const &entity);
    void destroyNode(Entity const &entity);
    void reconstructParentChildRelationship(Entity const &entity);
    void reconstructAllRelationships();

    void updateGraph();

    void setupEventListeners();
    void onTransformUpdate(OnComponent<Transform> const &event);
    void onPropertiesUpdate(OnComponent<Properties> const &event);
    void onEntityCreate(OnEntity const &event);
    void onEntityDestroy(OnEntity const &event);

    // ============================================================== Data == //
    struct GraphNode {
        GraphNode *parent;
        std::set<EntityId> children;

        std::optional<Entity> entity;
        DirectX::XMMATRIX cumulativeTransform;
        bool cumulativeActivity;
        bool recalculateTransforms, recalculateActivity;
    } root;
    std::unordered_map<EntityId, GraphNode> entityToGraphNode;
    std::unordered_map<EntityId, Transform> entityToPreviousTransform;
    std::unordered_map<EntityId, bool> entityToPreviousActivity;
    std::list<EntityId> createdEntities;
};

// ////////////////////////////////////////////////////////////////////////// //
