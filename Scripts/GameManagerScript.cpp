// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "GameManagerScript.hpp"

#include <filesystem>
#include <future>
#include <random>
#include <thread>

#include "Camera.h"
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Timer.h"
#include "Window.h"
#include "easings.hpp"
#include "math-operators.hpp"
#include "script-factory.hpp"

// //////////////////////////////////////////////////////// Namespace aliases //
namespace fs = std::filesystem;

// ////////////////////////////////////////////////////////////////// Helpers //
void interpolateTextTo(EntityId entity, float const target,
                       float const deltaTime, float const smooth = 0.5f) {
    auto& alpha = Entity(entity).get<UIElement>().alpha;
    alpha = interpolate(easeOutSine, alpha, target, smooth, deltaTime);
}

EntityId cache(std::string const& prefabPath) {
    return Registry::instance()
        .system<SceneSystem>()
        ->spawnPrefab(prefabPath, true)
        .id;
};

// //////////////////////////////////////////////////////////////// Variables //
std::future<EntityId> cacheFuture;
std::thread cacheThread;

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(GameManagerScript)

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
    registry.listen<OnButtonClick>(
        MethodListener(GameManagerScript::onButtonClick));
    registry.listen<OnButtonHover>(
        MethodListener(GameManagerScript::onButtonHover));

    // Set helpers
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };

    // Register sounds
    registry.system<SoundSystem>()->registerMultisampleEffect(
        "ui-button-click", "Assets\\Audio\\gui", ".wav");
    registry.system<SoundSystem>()->registerMultisampleEffect(
        "ui-button-hover", "Assets\\Audio\\Airlock", ".wav");

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

    spawnTorches();

    playerId =
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;

    cameraId = registry.system<PropertySystem>()
                   ->findEntityByTag("MainCamera")
                   .at(0)
                   .id;
    cameraScript = std::static_pointer_cast<CameraControllerScript>(
        Entity(cameraId).get<Behaviour>().script);

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

    // Segregate the UI entities
    {
        menuGroup.push_back(menuPlayButton);
        menuGroup.push_back(menuHelpButton);
        menuGroup.push_back(menuAuthorsButton);
        menuGroup.push_back(menuExitButton);

        gameGroup.push_back(gameScoreText);
        gameGroup.push_back(gameScoreValueText);

        resultsGroup.push_back(resultsYouDiedText);
        resultsGroup.push_back(resultsYourScoreText);
        resultsGroup.push_back(resultsScoreValueText);
        resultsGroup.push_back(resultsPressAnyKey);

        pauseMenuGroup.push_back(pauseMenuResumeButton);
        pauseMenuGroup.push_back(pauseMenuHelpButton);
        pauseMenuGroup.push_back(pauseMenuAuthorsButton);
        pauseMenuGroup.push_back(pauseMenuExitButton);
    }

    menuChunk =
        registry.system<PropertySystem>()->findEntityByName("Menu").at(0).id;
    menuOriginalOffset = Entity(menuChunk).get<Transform>().position -
                         Entity(playerId).get<Transform>().position;
    menuCamera = registry.system<PropertySystem>()
                     ->findEntityByName("Menu Camera")
                     .at(0)
                     .id;
    menuCameraOriginalOffset = Entity(menuCamera).get<Transform>().position -
                               Entity(playerId).get<Transform>().position;
};

void GameManagerScript::update(float const deltaTime) {
    Entity(menuChunk).get<Transform>().position =
        Entity(playerId).get<Transform>().position + menuOriginalOffset;
    Entity(menuCamera).get<Transform>().position =
        Entity(playerId).get<Transform>().position + menuCameraOriginalOffset;

    goalScorePosition = 1.0f * Entity(playerId).get<Transform>().position.x;
    goalScore = goalScorePosition + goalScoreTorches;
    score = interpolate<float>(easeOutSine, score, goalScore, 0.1f, deltaTime);

    constexpr int CHUNKS_TO_OSCILLATE_FROM_MIN_TO_MAX = 5;
    spawnChance =
        49.5f * std::sin(M_PI_2 * 2.0f *
                             Entity(playerId).get<Transform>().position.x /
                             CHUNKS_TO_OSCILLATE_FROM_MIN_TO_MAX /
                             PART_LENGTH_IN_WORLD_UNITS -
                         M_PI_2) +
        51.0f;

    Entity(gameScoreValueText).get<UIElement>().content =
        std::to_string(int(score));
    Entity(resultsScoreValueText).get<UIElement>().content =
        std::to_string(int(score));

    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
            // Fade from black to the menu
            screenFade =
                interpolate(easeOutSine, screenFade, 1.0f, 0.5f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);

            // Change state
            if (screenFade > 0.8f) {
                registry.send(OnGameStateChange{.nextState = MENU});
            }
        } break;
        case MENU: {
            // Fade from black to the menu
            screenFade =
                interpolate(easeOutSine, screenFade, 1.0f, 0.5f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);

            // Manage UI fades
            for (auto const& entity : menuGroup) {
                interpolateTextTo(entity, 1.0f, deltaTime);
            }
            for (auto const& entity : helpText) {
                interpolateTextTo(
                    entity, fadeInHelp ? 1.0f : 0.0f, deltaTime,
                    fadeInHelp ? 0.2f : (fadeInAuthors ? 0.01f : 0.3f));
            }
            for (auto const& entity : authorNamesText) {
                interpolateTextTo(
                    entity, fadeInAuthors ? 1.0f : 0.0f, deltaTime,
                    fadeInAuthors ? 0.2f : (fadeInHelp ? 0.01f : 0.3f));
            }
            for (auto const& entity : gameGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : resultsGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : pauseMenuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
        } break;
        case CHANGE_MENU_TYPE_TO_MAIN: {
        } break;
        case CHANGE_MENU_TYPE_TO_PAUSE: {
        } break;
        case MENU_TO_GAME_FADE_OUT: {
            // Fade to black
            screenFade =
                interpolate(easeOutQuint, screenFade, 0.0f, 0.1f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);

            // Manage UI fades
            for (auto const& entity : menuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : helpText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : authorNamesText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : gameGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : resultsGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : pauseMenuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }

            // Change state
            if (screenFade < 0.25f) {
                registry.send(OnGameStateChange{.nextState = GAME_FADE_IN});
            }
        } break;
        case NEW_GAME_SETUP: {
            spawnedChunks = 0;
            score = 0.0f;
            goalScore = goalScoreTorches = goalScorePosition = 0;
            spawnChance = 0.0f;

            // Spawn the starting chunk
            generatedLengthInParts = lengthOfChunk.at("Chunk Start");

            presentChunks.push_back(Chunk{
                .name = "Chunk Start",
                .entity = registry.system<SceneSystem>()
                              ->spawnPrefab(CHUNKS_DIRECTORY + "\\Chunk "
                                                               "Start.prefab",
                                            false)
                              .id,
                .endPositionInParts = generatedLengthInParts});

            nextChunk = "chunk-tmw-a-1-cc-01";
            auto cachePackagedTask = std::packaged_task<EntityId()>([this]() {
                return cache(CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab");
            });
            cacheFuture = cachePackagedTask.get_future();
            cacheThread = std::thread(std::move(cachePackagedTask));

            updateWaterfallRefraction();
            updateTrapRefraction();
            spawnTorches();
            spawnBishops(spawnChance);
            spawnRooks(spawnChance, false);

            registry.system<GraphSystem>()->rebuildGraph();

            registry.send(OnGameStateChange{.nextState = GAME_LAUNCH_FADE_IN});
        } break;
        case GAME_FADE_IN: {
            // Fade in
            screenFade =
                interpolate(easeOutQuint, screenFade, 1.0f, 0.1f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);

            // Manage UI fades
            for (auto const& entity : menuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : helpText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : authorNamesText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : gameGroup) {
                interpolateTextTo(entity, 1.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : resultsGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : pauseMenuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }

            // Change state
            if (screenFade > 0.999f) {
                registry.send(OnGameStateChange{.nextState = GAME});
            }
        } break;
        case GAME: {
            handleChunkSpawning(deltaTime);

            // Manage UI fades
            for (auto const& entity : menuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : helpText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : authorNamesText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : gameGroup) {
                interpolateTextTo(entity, 1.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : resultsGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }
            for (auto const& entity : pauseMenuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.001f);
            }

            auto e =
                registry.system<WindowSystem>()->window().keyboard.ReadKey();
            if (e.IsPress() && e.GetCode() == VK_ESCAPE) {
                registry.send(OnGameStateChange{.nextState = MENU});
                registry.system<WindowSystem>()->window().keyboard.FlushKey();
            }
        } break;
        case DEATH_RESULTS: {
            // Fade to semi-black
            screenFade =
                interpolate(easeOutQuint, screenFade, 0.5f, 0.2f, deltaTime);
            registry.system<BillboardRenderSystem>()->setBlackProportion(
                screenFade);

            // Manage UI fades
            for (auto const& entity : menuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : helpText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : authorNamesText) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : gameGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }
            for (auto const& entity : resultsGroup) {
                interpolateTextTo(entity, 1.0f, deltaTime, 0.8f);
            }
            for (auto const& entity : pauseMenuGroup) {
                interpolateTextTo(entity, 0.0f, deltaTime, 0.01f);
            }

            resultsTimer += deltaTime;
            if (resultsTimer >= 1.0f) {
                bool isAnyKeyPressed = false;
                for (char key = 'A'; key <= 'Z'; ++key) {
                    isAnyKeyPressed |= isKeyPressed(key);
                }
                for (char key = '0'; key <= '9'; ++key) {
                    isAnyKeyPressed |= isKeyPressed(key);
                }

                if (isAnyKeyPressed) {
                    registry.send(OnGameStateChange{
                        .nextState = RESULTS_TO_GAME_FADE_OUT});
                    resultsTimer = 0.0f;
                }
            }
        } break;
        case RESULTS_TO_GAME_FADE_OUT: {
            spawnedChunks = 0;
            score = 0.0f;
            goalScore = goalScoreTorches = goalScorePosition = 0;
            spawnChance = 0.0f;

            // Cleanup
            generatedLengthInParts = lengthOfChunk.at("Chunk Start");

            // Delete the chunks we've already passed
            do {
                auto i = presentChunks.begin();
                registry.system<GraphSystem>()->destroyEntityWithChildren(
                    i->entity);
                presentChunks.erase(i);
            } while (!presentChunks.empty());

            presentChunks.push_back(Chunk{
                .name = "Chunk Start",
                .entity = registry.system<SceneSystem>()
                              ->spawnPrefab(CHUNKS_DIRECTORY + "\\Chunk "
                                                               "Start.prefab",
                                            false)
                              .id,
                .endPositionInParts = generatedLengthInParts});

            cacheThread.join();
            registry.clearCache();

            nextChunk = "chunk-tmw-a-1-cc-01";
            auto cachePackagedTask = std::packaged_task<EntityId()>([this]() {
                return cache(CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab");
            });
            cacheFuture = cachePackagedTask.get_future();
            cacheThread = std::thread(std::move(cachePackagedTask));

            updateWaterfallRefraction();
            updateTrapRefraction();
            spawnTorches();
            spawnBishops(spawnChance);
            spawnRooks(spawnChance, false);

            registry.system<GraphSystem>()->rebuildGraph();

            registry.send(OnGameStateChange{.nextState = GAME_FADE_IN});
        } break;
        case GAME_EXIT_FADE_OUT: {
        } break;
        default: {
        } break;
    };
};

// ------------------------------------------------------------- Events -- == //
void GameManagerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == playerId || event.b.id == playerId) {
        auto const& other =
            Entity(event.a.id == playerId ? event.b.id : event.a.id);
        auto const& otherTag = other.get<Properties>().tag;

        if (otherTag == "Torch") {
            goalScoreTorches += 100;
        }
    }
}
void GameManagerScript::onGameStateChange(OnGameStateChange const& event) {
    if (currentState != DEATH_RESULTS && event.nextState == DEATH_RESULTS) {
        resultsTimer = 0.0f;
    }
    currentState = event.nextState;
    registry.system<WindowSystem>()->window().keyboard.FlushKey();
}
void GameManagerScript::onButtonClick(OnButtonClick const& event) {
    auto isButtonClicked = [this, &event](EntityId const button) {
        return Entity(button).get<UIElement>().button.get() == event.button;
    };

    registry.system<SoundSystem>()->playRandomSample("ui-button-click");

    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
        } break;
        case MENU: {
            if (isButtonClicked(menuPlayButton)) {
                registry.send(
                    OnGameStateChange{.nextState = MENU_TO_GAME_FADE_OUT});
            } else if (isButtonClicked(menuExitButton)) {
                registry.send(OnGameExit{});
            }
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
}
void GameManagerScript::onButtonHover(OnButtonHover const& event) {
    auto isButtonHovered = [this, &event](EntityId const button) {
        return Entity(button).get<UIElement>().button.get() == event.button;
    };
    auto isMouseOnButton = [this, &event](EntityId const button) {
        return event.on;
    };

    registry.system<SoundSystem>()->playRandomSample(
        "ui-button-hover", 0.025f * (event.on ? 1.0f : 0.5f));

    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
        } break;
        case MENU: {
            if (isButtonHovered(menuHelpButton)) {
                fadeInHelp = isMouseOnButton(menuHelpButton);
            }
            if (isButtonHovered(menuAuthorsButton)) {
                fadeInAuthors = isMouseOnButton(menuAuthorsButton);
            }
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
                it.get<Flame>().fireParticle->scale = 0.03f;
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

void GameManagerScript::spawnEnemy(MovementType mt, Entity const& spawnPoint,
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

        auto const& enemyBehaviour = enemy->get<Behaviour>();
        auto const& enemyControllerScript =
            std::static_pointer_cast<EnemyControllerScript>(
                enemyBehaviour.script);
        enemyControllerScript->setMovementType(mt, movingSideways);

        auto& enemyTransform = enemy->get<Transform>();
        auto const& spawnPointTransform = spawnPoint.get<Transform>();
        enemyTransform.position = spawnPointTransform.position;
        enemyTransform.parent = spawnPointTransform.parent;
    }
    registry.destroyEntity(spawnPoint);
}

bool GameManagerScript::shouldHappen(int percentage) {
    if (percentage == 0) {
        return false;
    }
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

void GameManagerScript::shakeCamera(float const deltaTime) {
    shakeTimer += deltaTime;

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
    static float const VERTICAL_DELTA = 0.1f;
    static float const TILE_WIDTH = 2.0f;
    static float const ALPHA = std::asin(VERTICAL_DELTA / TILE_WIDTH);

    auto const& playerPositionInWorldUnits =
        Entity(playerId).get<Transform>().position.x;
    auto const& generatedLengthInWorldUnits =
        PART_LENGTH_IN_WORLD_UNITS * generatedLengthInParts -
        (spawnedChunks * TILE_WIDTH * (1.0f - std::cos(ALPHA)));

    bool const hasPlayerPassedSpawningPoint =
        playerPositionInWorldUnits >=
        generatedLengthInWorldUnits - SPAWN_PADDING_IN_WORLD_UNITS;

    if (hasPlayerPassedSpawningPoint &&
        cacheFuture.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready) {
        ++spawnedChunks;

        // Potentially wait for this chunk caching to finish
        cacheThread.join();

        // Update the length of all spawned chunks so far
        generatedLengthInParts += lengthOfChunk.at(nextChunk);

        // Spawn the new chunk
        chunkSpawnTime = Timer();

        registry.moveCacheToMainScene();
        auto chunk = Entity{cacheFuture.get()};
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

        // Fix the adjacent box colliders problem by spawning the next chunk
        // slightly lower
        chunk.get<Transform>().position.y = -VERTICAL_DELTA * spawnedChunks;

        // Rotate the last lanes to compensate for the lowered chunk
        auto lastLanes =
            registry.system<PropertySystem>()->findEntityByTag("LastLane");
        for (Entity& lane : lastLanes) {
            lane.get<Transform>().position.y = -VERTICAL_DELTA / 2.0f;
            lane.get<Transform>().euler.z = -ALPHA;
        }

        // Spawn the enemies
        spawnTorches();
        spawnBishops(spawnChance);
        spawnRooks(spawnChance, true);

        // Delete the chunks we've already passed
        auto i = presentChunks.begin();
        for (auto const& chunk : presentChunks) {
            if (chunk.endPositionInParts * PART_LENGTH_IN_WORLD_UNITS <=
                playerPositionInWorldUnits - SPAWN_PADDING_IN_WORLD_UNITS) {
                registry.system<GraphSystem>()->destroyEntityWithChildren(
                    chunk.entity);
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

        auto cachePackagedTask = std::packaged_task<EntityId()>([this]() {
            return cache(CHUNKS_DIRECTORY + "\\" + nextChunk + ".prefab");
        });
        cacheFuture = cachePackagedTask.get_future();
        cacheThread = std::thread(std::move(cachePackagedTask));

        // Update the spawned objects if needed
        updateWaterfallRefraction();
        updateTrapRefraction();

        registry.system<GraphSystem>()->rebuildGraph();
    }
    if (shake) {
        shakeCamera(deltaTime);
    }
}

// ////////////////////////////////////////////////////////////////////////// //
