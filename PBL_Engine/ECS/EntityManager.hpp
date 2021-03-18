#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <array>
#include <mutex>
#include <queue>

#include "EngineAPI.hpp"
#include "Utilities.hpp"
#include "assert.hpp"

// /////////////////////////////////////////////////////////////////// Macros //
#define ASSERT_ENTITY_EXISTS()                           \
    assert((entityId >= 0 && entityId < MAX_ENTITIES) && \
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API EntityManager {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static EntityManager& instance();

    EntityManager(EntityManager const&) = delete;
    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(EntityManager const&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    // --------------------------------------------- Main functionality -- == //
    template <bool threadSafe = true>
    EntityId create(SceneId const sceneId) {
        assert((sceneId >= 0 && sceneId < MAX_SCENES) &&
               "Scene identifier must be in range [0, MAX_SCENES)!");

        LOCK_GUARD(entityManagerMutex, threadSafe);

        assert(availableIdentifiers.size() > 0 &&
               "Cannot create more entities!");

        EntityId entityId = availableIdentifiers.front();
        availableIdentifiers.pop();

        ASSERT_ENTITY_EXISTS();

        signatures.at(entityId).sceneId = sceneId;

        return entityId;
    }

    template <bool threadSafe = true>
    void destroy(EntityId const entityId) {
        ASSERT_ENTITY_EXISTS();

        LOCK_GUARD(entityManagerMutex, threadSafe);

        signatures.at(entityId).components.reset();
        signatures.at(entityId).sceneId = EMPTY_SCENE;
        availableIdentifiers.push(entityId);
    }

    template <bool threadSafe = true>
    void setSignature(EntityId const entityId, Signature const& signature) {
        ASSERT_ENTITY_EXISTS();

        LOCK_GUARD(entityManagerMutex, threadSafe);

        signatures.at(entityId) = signature;
    }

    template <bool threadSafe = true>
    Signature getSignature(EntityId const entityId) {
        ASSERT_ENTITY_EXISTS();

        LOCK_GUARD(entityManagerMutex, threadSafe);

        return signatures.at(entityId);
    }

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    EntityManager();
    ~EntityManager() = default;

    // ============================================================== Data == //
    std::mutex entityManagerMutex;

    std::queue<EntityId> availableIdentifiers{};
    std::array<Signature, MAX_ENTITIES> signatures{};
};

// ////////////////////////////////////////////////////////////////////////// //
