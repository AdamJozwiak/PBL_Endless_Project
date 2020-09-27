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
Entity SceneSystem::spawnPrefab(std::string const& path, bool cache) {
    return levelParser.loadPrefab(path, cache);
}

// ////////////////////////////////////////////////////////////////////////// //
