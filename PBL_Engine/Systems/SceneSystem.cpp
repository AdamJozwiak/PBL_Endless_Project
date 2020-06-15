// ///////////////////////////////////////////////////////////////// Includes //
#include "SceneSystem.hpp"

#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void SceneSystem::filters() {}

void SceneSystem::setup() {
    PointLight::initTorchNumbers();
    levelParser.initialize();
    levelParser.loadScene("Assets\\Unity\\Scenes\\Main.unity");
}

void SceneSystem::update(float deltaTime){};

void SceneSystem::release() {}

// --------------------------------------------------- Public interface -- == //
void SceneSystem::cachePrefab(std::string const& path) {
    levelParser.cachePrefab(path, true);
}

Entity SceneSystem::spawnPrefab(std::string const& path, bool cache) {
    if (cache) {
        cachePrefab(path);
    }
    return levelParser.loadPrefab(path);
}

// ////////////////////////////////////////////////////////////////////////// //
