#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <array>
#include <mutex>
#include <queue>

#include "EngineAPI.hpp"
#include "Utilities.hpp"

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
    EntityId create(SceneId sceneId = DEFAULT_SCENE);
    void destroy(EntityId entityId);

    void setSignature(EntityId entityId, Signature const& signature);
    Signature getSignature(EntityId entityId);

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    EntityManager();
    ~EntityManager() = default;

    // ============================================================== Data == //
    std::recursive_mutex mutex;

    std::queue<EntityId> availableIdentifiers{};
    std::array<Signature, MAX_ENTITIES> signatures{};
};

// ////////////////////////////////////////////////////////////////////////// //
