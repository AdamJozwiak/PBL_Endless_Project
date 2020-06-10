// ///////////////////////////////////////////////////////////////// Includes //
#include "GameManagerScript.hpp"

#include <filesystem>
#include <thread>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
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

    playerId =
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;
};

void GameManagerScript::update(float const deltaTime) {
    handleChunkSpawning();
};

// ------------------------------------------------------------- Events -- == //
void GameManagerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
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

void GameManagerScript::handleChunkSpawning() {
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
        auto chunk = Registry::instance().system<SceneSystem>()->spawnPrefab(
            CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab", false);

        presentChunks.push_back(
            Chunk{.name = nextChunk,
                  .entity = chunk.id,
                  .endPositionInParts = generatedLengthInParts});

        // Move the new chunk to its place
        chunk.get<Transform>().position.x = generatedLengthInWorldUnits;

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
        static std::default_random_engine generator;

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
    }
}

// ////////////////////////////////////////////////////////////////////////// //
