// ///////////////////////////////////////////////////////////////// Includes //
#include "GameManagerScript.hpp"

#include <filesystem>
#include <random>
#include <thread>

#include "Camera.h"
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Timer.h"
#include "Window.h"

// //////////////////////////////////////////////////////// Namespace aliases //
namespace fs = std::filesystem;

// //////////////////////////////////////////////////////////////// Variables //
std::unique_ptr<std::thread> cacheThread;

// ///////////////////////////////////////////////////////// Factory function //
extern "C" GAMEMANAGERSCRIPT_API void create(std::shared_ptr<Script>& script,
                                             Entity entity) {
    script = std::make_shared<GameManagerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
GameManagerScript::GameManagerScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void GameManagerScript::setup() {
    // Set event listeners
    registry.listen<OnCollisionEnter>(
        MethodListener(GameManagerScript::onCollisionEnter));

    // Set helpers
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };

    // Load chunk names
    for (auto const& entry :
         fs::recursive_directory_iterator(CHUNKS_DIRECTORY)) {
        auto const& extension = entry.path().extension().string();
        if (extension != ".prefab") {
            continue;
        }

        auto const& name = entry.path().stem().string();
        chunkNames.emplace_back(name);
    }

    // Load the beginnings and endings of the chunks
    auto entities = registry.system<PropertySystem>()->findEntityByTag(
        "ChunkStartEndProperty");
    for (auto const& e : entities) {
        auto const& beginning = e.get<Transform>().euler;
        assert(beginning.x + beginning.y + beginning.z == 1 &&
               "Wrong chunk beginning!");

        auto const& ending = e.get<Transform>().scale;
        assert(ending.x + ending.y + ending.z == 1 && "Wrong chunk ending!");

        WayPosition const begin = beginning.x ? LEFT
                                              : (beginning.y ? CENTER : RIGHT),
                          end = ending.x ? LEFT : (ending.y ? CENTER : RIGHT);

        auto const& name = e.get<Properties>().name;

        endingOfChunk[name] = end;
        chunksBeginningWith[begin].emplace_back(name);
    }

    // Load the chunk lengths
    entities = registry.system<PropertySystem>()->findEntityByTag(
        "ChunkLengthProperty");
    for (auto const& e : entities) {
        auto const& length = static_cast<int>(e.get<Transform>().scale.x);
        assert(length != 0 && "Wrong chunk length!");

        auto const& name = e.get<Properties>().name;
        lengthOfChunk[name] = length;
    }

    // Spawn the starting chunk
    generatedLengthInParts = lengthOfChunk.at("Chunk Start");

    registry.system<SceneSystem>()->cachePrefab(CHUNKS_DIRECTORY +
                                                "\\Chunk Start.prefab");
    presentChunks.push_back(
        Chunk{.name = "Chunk Start",
              .entity = registry.system<SceneSystem>()
                            ->spawnPrefab(CHUNKS_DIRECTORY + "\\Chunk "
                                                             "Start.prefab",
                                          false)
                            .id,
              .endPositionInParts = generatedLengthInParts});
    cacheThread = std::make_unique<std::thread>([this] {
        Registry::instance().system<SceneSystem>()->cachePrefab(
            CHUNKS_DIRECTORY + "\\Chunk 1.prefab");
    });
    nextChunk = "Chunk 1";

    updateWaterfallRefraction();
    updateTrapRefraction();
    spawnTorches();
    spawnBishops(100);
    spawnRooks(100, false);

    playerId =
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;
};

void GameManagerScript::update(float const deltaTime) {
    handleChunkSpawning(deltaTime);
};

// ------------------------------------------------------------- Events -- == //
void GameManagerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}

void GameManagerScript::spawnTorches() {
    for (auto it :
         registry.system<PropertySystem>()->findEntityByTag("Torch")) {
        if (!it.has<Light>()) {
            it.add<Light>({.pointLight = std::make_shared<PointLight>(
                               registry.system<WindowSystem>()->gfx())});
        }
        if (!it.has<Flame>()) {
            it.add<Flame>({.fireParticle = std::make_shared<FireParticle>(
                               registry.system<WindowSystem>()->gfx(),
                               Registry::instance()
                                   .system<PropertySystem>()
                                   ->findEntityByTag("MainCamera")
                                   .at(0)
                                   .get<MainCamera>()
                                   .camera.get())});
            if (it.get<Properties>().name == "Menu Torch") {
                it.get<Flame>().fireParticle->scale = 0.02f;
            }
        }
    }
}

void GameManagerScript::spawnRooks(int percentage, bool movingSideways) {
    findSpawnPoints(Rook);
    for (auto it : spawnPoints) {
        spawnEnemy(Rook, it, percentage, movingSideways);
    }
}

void GameManagerScript::spawnBishops(int percentage) {
    findSpawnPoints(Bishop);
    for (auto it : spawnPoints) {
        spawnEnemy(Bishop, it, percentage);
    }
}

void GameManagerScript::spawnEnemy(MovementType mt, EntityId spawnPoint,
                                   int percentage, bool movingSideways) {
    if (shouldHappen(percentage)) {
        std::shared_ptr<Entity> enemy;
        if (mt == Bishop) {
            enemy = std::make_shared<Entity>(
                Registry::instance().system<SceneSystem>()->spawnPrefab(
                    "Assets\\Unity\\Prefabs\\Enemy.prefab"));
            enemy->add<Flame>({.fireParticle = std::make_shared<FireParticle>(
                                   registry.system<WindowSystem>()->gfx(),
                                   Registry::instance()
                                       .system<PropertySystem>()
                                       ->findEntityByTag("MainCamera")
                                       .at(0)
                                       .get<MainCamera>()
                                       .camera.get(),
                                   bishop)});
        } else if (mt == Rook) {
            enemy = std::make_shared<Entity>(
                Registry::instance().system<SceneSystem>()->spawnPrefab(
                    "Assets\\Unity\\Prefabs\\EnemyRook.prefab"));
            enemy->add<Flame>({.fireParticle = std::make_shared<FireParticle>(
                                   registry.system<WindowSystem>()->gfx(),
                                   Registry::instance()
                                       .system<PropertySystem>()
                                       ->findEntityByTag("MainCamera")
                                       .at(0)
                                       .get<MainCamera>()
                                       .camera.get(),
                                   rook)});
        }

        enemyScript = std::static_pointer_cast<EnemyControllerScript>(
            enemy->get<Behaviour>().script);
        enemyScript->setMovementType(mt, movingSideways);
        enemy->get<Transform>().position.x =
            Entity(spawnPoint).get<Transform>().position.x;
        enemy->get<Transform>().position.y =
            Entity(spawnPoint).get<Transform>().position.y + 1.5f;
        enemy->get<Transform>().position.z =
            Entity(spawnPoint).get<Transform>().position.z;
    }
    registry.destroyEntity(Entity(spawnPoint));
}

bool GameManagerScript::shouldHappen(int percentage) {
    std::random_device rnd;
    std::mt19937 rng(rnd());
    std::uniform_int_distribution<int> uni(1, 100 / percentage);
    return (uni(rng) == 1 ? true : false);
}

void GameManagerScript::findSpawnPoints(MovementType mt) {
    spawnPoints.clear();
    for (auto it : registry.system<PropertySystem>()->findEntityByTag(
             "EnemySpawnPoint" + enumToString(mt))) {
        spawnPoints.push_back(it.id);
    }
}

void GameManagerScript::shakeCamera(float deltaTime) {
    shakeTimer += deltaTime;
    auto camera = Registry::instance()
                      .system<PropertySystem>()
                      ->findEntityByTag("MainCamera")
                      .at(0)
                      .id;
    cameraScript = std::static_pointer_cast<CameraControllerScript>(
        Entity(camera).get<Behaviour>().script);
    cameraScript->shake(deltaTime);
    if (shakeTimer >= 0.75f) {
        shake = false;
        shakeTimer = 0.0f;
    }
}

std::string GameManagerScript::enumToString(MovementType mt) {
    switch (mt) {
        case Rook:
            return "Rook";
        case Bishop:
            return "";
        default:
            return "Invalid enemy type";
    }
}

// ------------------------------------------------------------ Methods -- == //
void GameManagerScript::method() {}

void GameManagerScript::updateWaterfallRefraction() {
    auto const& waterfalls =
        registry.system<PropertySystem>()->findEntityByTag("Waterfall");

    for (auto waterfall : waterfalls) {
        if (!waterfall.has<Refractive>()) {
            waterfall.add<Refractive>({});
        }
    }
}

void GameManagerScript::updateTrapRefraction() {
    auto const& soilModels =
        registry.system<PropertySystem>()->findEntityByName("Soil");
    for (auto model : soilModels) {
        if (!model.has<Refractive>()) {
            model.add<Refractive>({});
        }
    }

    auto const& spikesModels =
        registry.system<PropertySystem>()->findEntityByName("Spikes");
    for (auto model : spikesModels) {
        if (!model.has<Refractive>()) {
            model.add<Refractive>({});
        }
    }
}

void GameManagerScript::handleChunkSpawning(float deltaTime) {
    auto const& playerPositionInWorldUnits =
        Entity(playerId).get<Transform>().position.x;
    auto const& generatedLengthInWorldUnits =
        PART_LENGTH_IN_WORLD_UNITS * generatedLengthInParts;

    bool const hasPlayerPassedSpawningPoint =
        playerPositionInWorldUnits >=
        generatedLengthInWorldUnits - SPAWN_PADDING_IN_WORLD_UNITS;

    if (hasPlayerPassedSpawningPoint) {
        // Potentially wait for this chunk caching to finish
        cacheThread->join();

        // Update the length of all spawned chunks so far
        generatedLengthInParts += lengthOfChunk.at(nextChunk);

        // Spawn the new chunk
        shake = true;
        chunkSpawnTime = Timer();
        auto chunk = Registry::instance().system<SceneSystem>()->spawnPrefab(
            CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab", false);
        spawnDuration = chunkSpawnTime.Peek();
        presentChunks.push_back(
            Chunk{.name = nextChunk,
                  .entity = chunk.id,
                  .endPositionInParts = generatedLengthInParts});

        // Move the new chunk and the enemy spawn points to their place
        chunk.get<Transform>().position.x = generatedLengthInWorldUnits;
        for (auto it : registry.system<PropertySystem>()->findEntityByTag(
                 "EnemySpawnPoint")) {
            it.get<Transform>().position.x += generatedLengthInWorldUnits;
        }

        // Fix the adjacent box colliders problem by spawning the next chunk
        // slightly lower
        static int spawnedChunks = 0;
        chunk.get<Transform>().position.y = -0.1f * ++spawnedChunks;

        // Spawn the enemies
        spawnTorches();
        spawnBishops(50);
        spawnRooks(50, false);

        // Delete the chunks we've already passed
        std::vector<decltype(presentChunks.begin())> toDelete;
        auto i = presentChunks.begin();
        for (auto const& chunk : presentChunks) {
            if (chunk.endPositionInParts * PART_LENGTH_IN_WORLD_UNITS <=
                playerPositionInWorldUnits - SPAWN_PADDING_IN_WORLD_UNITS) {
                registry.system<GraphSystem>()->destroyEntityWithChildren(
                    chunk.entity);
                registry.system<GraphSystem>()->setup();
                toDelete.push_back(i);
            }
            ++i;
        }
        for (auto const& i : toDelete) {
            presentChunks.erase(i);
        }

        // Randomly find the next chunk
        std::random_device device;
        std::mt19937 generator(device());

        auto ending = endingOfChunk.at(presentChunks.back().name);
        auto potentialChunks = chunksBeginningWith.at(ending);
        do {
            std::uniform_int_distribution<int> distribution(
                0, potentialChunks.size() - 1);
            nextChunk = potentialChunks.at(distribution(generator));
        } while (nextChunk == "Chunk Start");

        cacheThread = std::make_unique<std::thread>([this] {
            registry.system<SceneSystem>()->cachePrefab(
                CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab");
        });

        // Update the spawned objects if needed
        updateWaterfallRefraction();
        updateTrapRefraction();
    }
    if (shake && spawnDuration >= 2.0f) {
        shakeCamera(deltaTime);
    } else if (shake && spawnDuration < 2.0f) {
        if (shouldHappen(40)) {
            shakeCamera(deltaTime);
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
