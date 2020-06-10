#pragma once
#include <ECS/Entity.hpp>
#include <set>
#include <string>
class LevelParser {
  public:
    LevelParser();
    ~LevelParser();
    void loadScene(std::string const &scenePath);

    Entity loadPrefab(std::string const &filename);
    static void cachePrefab(std::string const &filename, bool clear = false);
    void finalizeLoading(std::set<EntityId> const &recursivePrefabIds);

    void initialize();
};
