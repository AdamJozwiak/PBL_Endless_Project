// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

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
#include "easings.hpp"

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
    registry.listen<OnGameStateChange>(
        MethodListener(GameManagerScript::onGameStateChange));

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
        auto const& beginning = e.get<Transform>().position;
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

    // Find the UI elements
    {
        auto findEntityByName = std::bind(
            &PropertySystem::findEntityByName,
            registry.system<PropertySystem>().get(), std::placeholders::_1);

        menuPlayButton = findEntityByName("Play Button").at(0).id;
        menuHelpButton = findEntityByName("Help Button").at(0).id;
        menuAuthorsButton = findEntityByName("Credits Button").at(0).id;
        menuExitButton = findEntityByName("Exit Button").at(0).id;

        authorNamesText.push_back(findEntityByName("AJ Text").at(0).id);
        authorNamesText.push_back(findEntityByName("JK Text").at(0).id);
        authorNamesText.push_back(findEntityByName("PP Text").at(0).id);
        authorNamesText.push_back(findEntityByName("MP Text").at(0).id);
        authorNamesText.push_back(findEntityByName("TW Text").at(0).id);

        helpText.push_back(findEntityByName("Key1 Text").at(0).id);
        helpText.push_back(findEntityByName("Key2 Text").at(0).id);
        helpText.push_back(findEntityByName("Key3 Text").at(0).id);
        helpText.push_back(findEntityByName("Explanation1 Text").at(0).id);
        helpText.push_back(findEntityByName("Explanation2 Text").at(0).id);
        helpText.push_back(findEntityByName("Explanation3 Text").at(0).id);

        gameScoreText = findEntityByName("Score Text").at(0).id;
        gameScoreValueText = findEntityByName("Score Value Text").at(0).id;

        resultsYouDiedText = findEntityByName("You Died Text").at(0).id;
        resultsYourScoreText = findEntityByName("Your Score Text").at(0).id;
        resultsScoreValueText =
            findEntityByName("Result Score Value Text").at(0).id;
        resultsPressAnyKey = findEntityByName("Restart Text").at(0).id;

        pauseMenuResumeButton = findEntityByName("Resume Button").at(0).id;
        pauseMenuHelpButton = findEntityByName("Help Button").at(1).id;
        pauseMenuAuthorsButton = findEntityByName("Credits Button").at(1).id;
        pauseMenuExitButton = findEntityByName("Exit Button").at(1).id;
    }

    // Fade out the UI elements
    {
        Entity(menuPlayButton).get<UIElement>().alpha = 0.0f;
        Entity(menuHelpButton).get<UIElement>().alpha = 0.0f;
        Entity(menuAuthorsButton).get<UIElement>().alpha = 0.0f;
        Entity(menuExitButton).get<UIElement>().alpha = 0.0f;

        for (auto const& text : authorNamesText) {
            Entity(text).get<UIElement>().alpha = 0.0f;
        }

        for (auto const& text : helpText) {
            Entity(text).get<UIElement>().alpha = 0.0f;
        }

        Entity(gameScoreText).get<UIElement>().alpha = 0.0f;
        Entity(gameScoreValueText).get<UIElement>().alpha = 0.0f;

        Entity(resultsYouDiedText).get<UIElement>().alpha = 0.0f;
        Entity(resultsYourScoreText).get<UIElement>().alpha = 0.0f;
        Entity(resultsScoreValueText).get<UIElement>().alpha = 0.0f;
        Entity(resultsPressAnyKey).get<UIElement>().alpha = 0.0f;

        Entity(pauseMenuResumeButton).get<UIElement>().alpha = 0.0f;
        Entity(pauseMenuHelpButton).get<UIElement>().alpha = 0.0f;
        Entity(pauseMenuAuthorsButton).get<UIElement>().alpha = 0.0f;
        Entity(pauseMenuExitButton).get<UIElement>().alpha = 0.0f;
    }
};

void GameManagerScript::update(float const deltaTime) {
    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
            // Fade from black to the menu
            screenFade =
                interpolate(easeOutSine, screenFade, 1.0f, 0.5f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);
        } break;
        case MENU: {
        } break;
        case CHANGE_MENU_TYPE_TO_MAIN: {
        } break;
        case CHANGE_MENU_TYPE_TO_PAUSE: {
        } break;
        case MENU_TO_GAME_FADE_OUT: {
        } break;
        case NEW_GAME_SETUP: {
        } break;
        case GAME_FADE_IN: {
        } break;
        case GAME: {
            handleChunkSpawning(deltaTime);
        } break;
        case DEATH_RESULTS: {
        } break;
        case RESULTS_TO_GAME_FADE_OUT: {
        } break;
        case GAME_EXIT_FADE_OUT: {
        } break;
        default: {
        } break;
    };
};

// ------------------------------------------------------------- Events -- == //
void GameManagerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }
}
void GameManagerScript::onGameStateChange(OnGameStateChange const& event) {
    currentState = event.nextState;
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
    if (shakeTimer >= SHAKE_DURATION) {
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
        chunkSpawnTime = Timer();

        auto chunk = Registry::instance().system<SceneSystem>()->spawnPrefab(
            CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab", false);
        presentChunks.push_back(
            Chunk{.name = nextChunk,
                  .entity = chunk.id,
                  .endPositionInParts = generatedLengthInParts});

        spawnDuration = chunkSpawnTime.Peek();

        shake = (spawnDuration >= SHAKE_THRESHOLD_TIME_IN_SECONDS) ||
                ((spawnDuration < SHAKE_THRESHOLD_TIME_IN_SECONDS) &&
                 shouldHappen(SHAKE_PROBABILITY_UNDER_THRESHOLD));

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
        auto i = presentChunks.begin();
        for (auto const& chunk : presentChunks) {
            if (chunk.endPositionInParts * PART_LENGTH_IN_WORLD_UNITS <=
                playerPositionInWorldUnits - SPAWN_PADDING_IN_WORLD_UNITS) {
                registry.system<GraphSystem>()->destroyEntityWithChildren(
                    chunk.entity);
                registry.system<GraphSystem>()->setup();
                presentChunks.erase(i);
                break;
            }
            ++i;
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
    if (shake) {
        shakeCamera(deltaTime);
    }
}

// ////////////////////////////////////////////////////////////////////////// //
