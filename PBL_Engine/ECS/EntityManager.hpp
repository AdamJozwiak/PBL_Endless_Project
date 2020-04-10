#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <array>
#include <queue>

#include "Utilities.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class EntityManager {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static EntityManager& instance();

    EntityManager(EntityManager const&) = delete;
    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(EntityManager const&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    // --------------------------------------------- Main functionality -- == //
    EntityId create();
    void destroy(EntityId entityId);

    void setSignature(EntityId entityId, Signature const& signature);
    Signature getSignature(EntityId entityId);

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    EntityManager();
    ~EntityManager() = default;

    // ============================================================== Data == //
    std::queue<EntityId> availableIdentifiers{};
    std::array<Signature, MAX_ENTITIES> signatures{};
};

// ////////////////////////////////////////////////////////////////////////// //
