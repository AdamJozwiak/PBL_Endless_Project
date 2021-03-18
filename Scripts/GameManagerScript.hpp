#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <map>
#include <memory>

#include "CameraControllerScript.hpp"
#include "ECS/Entity.hpp"
#include "EnemyControllerScript.hpp"
#include "Events/OnButtonClick.hpp"
#include "Events/OnButtonHover.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Events/OnGameExit.hpp"
#include "Events/OnGameStateChange.hpp"
#include "Script.hpp"
#include "ScriptsAPI.hpp"
#include "Timer.h"

class EnemyControllerScript;

// //////////////////////////////////////////////////////////////////// Class //
class SCRIPTS_API GameManagerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    GameManagerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);
    void onGameStateChange(OnGameStateChange const &event);
    void onButtonClick(OnButtonClick const &event);
    void onButtonHover(OnButtonHover const &event);

    // -------------------------------------------------------- Methods -- == //
    void method();
    void updateWaterfallRefraction();
    void updateTrapRefraction();
    void handleChunkSpawning(float deltaTime);

  private:
    // ============================================================== Data == //
    GameState currentState = NEW_GAME_SETUP;
    float screenFade = 0.0f;

    float score = 0.0f;
    int goalScore = 0, goalScoreTorches = 0, goalScorePosition = 0;

    float spawnChance = 0.0f;

    bool fadeInHelp = false;
    bool fadeInAuthors = false;

    float resultsTimer = 0.0f;

    EntityId menuChunk, menuCamera;
    DirectX::XMFLOAT3 menuOriginalOffset, menuCameraOriginalOffset;

    // UI
    EntityId menuPlayButton, menuHelpButton, menuAuthorsButton, menuExitButton;
    std::vector<EntityId> authorNamesText, helpText;
    EntityId gameScoreText, gameScoreValueText;
    EntityId resultsYouDiedText, resultsYourScoreText, resultsScoreValueText,
        resultsPressAnyKey;
    EntityId pauseMenuResumeButton, pauseMenuHelpButton, pauseMenuAuthorsButton,
        pauseMenuExitButton;
    std::vector<EntityId> menuGroup, gameGroup, resultsGroup, pauseMenuGroup;

    bool (*isKeyPressed)(int const key);
    void spawnTorches();
    void spawnRooks(int percentage, bool movingSideways = true);
    void spawnBishops(int percentage);
    void spawnEnemy(MovementType mt, Entity const &spawnPoint, int percentage,
                    bool movingSideways = true);
    bool shouldHappen(int percentage);
    void findSpawnPoints(MovementType mt);
    void shakeCamera(float deltaTime);
    std::string enumToString(MovementType mt);

    EntityId playerId;
    std::vector<EntityId> spawnPoints;
    std::shared_ptr<CameraControllerScript> cameraScript;
    EntityId cameraId;
    bool shake = false;
    float shakeTimer = 0.0f;
    Timer chunkSpawnTime;
    float spawnDuration;
    float const SHAKE_THRESHOLD_TIME_IN_SECONDS = 0.5f;
    int const SHAKE_PROBABILITY_UNDER_THRESHOLD = 50;
    float const SHAKE_DURATION = 0.75f;

    // --------------------------------------------------------- Chunks -- == //
    // Types
    using ChunkName = std::string;
    using ChunkLength = unsigned int;

    enum WayPosition { LEFT, CENTER, RIGHT };

    struct Chunk {
        ChunkName name;
        EntityId entity;
        int endPositionInParts;
    };

    // Properties
    std::vector<ChunkName> chunkNames;
    std::map<ChunkName, WayPosition> endingOfChunk;
    std::map<WayPosition, std::vector<ChunkName>> chunksBeginningWith;
    std::map<ChunkName, ChunkLength> lengthOfChunk;

    int generatedLengthInParts = 0;
    int spawnedChunks = 0;

    // Chunks
    std::vector<Chunk> presentChunks;
    ChunkName nextChunk;

    EntityId chunkId;

    // Constants
    static inline constexpr float SPAWN_PADDING_IN_WORLD_UNITS = 80.0f;
    static inline constexpr float PART_LENGTH_IN_WORLD_UNITS = 20.0f;
    static inline std::string const CHUNKS_DIRECTORY =
        "Assets\\Unity\\Prefabs\\Chunks Completely Unpacked";
};

// ////////////////////////////////////////////////////////////////////////// //
