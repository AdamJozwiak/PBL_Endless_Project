#pragma once
#include <ECS/Entity.hpp>
#include <set>
#include <string>
class LevelParser {
  public:
    LevelParser();
    ~LevelParser();
    void load();

    Entity loadPrefab(std::string const &filename);
    void finalizeLoading(std::set<EntityId> const &recursivePrefabIds);
};
