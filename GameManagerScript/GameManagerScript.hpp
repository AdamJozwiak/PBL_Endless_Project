#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <map>
#include <memory>

#include "ECS/Entity.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "GameManagerScriptAPI.hpp"
#include "Script.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class GAMEMANAGERSCRIPT_API GameManagerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    GameManagerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);

    // -------------------------------------------------------- Methods -- == //
    void method();
    void updateWaterfallRefraction();
    void handleChunkSpawning();

  private:
    // ============================================================== Data == //
    bool (*isKeyPressed)(int const key);

    EntityId playerId;

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

    // Chunks
    std::vector<Chunk> presentChunks;
    ChunkName nextChunk;

    // Constants
    static inline constexpr float SPAWN_PADDING_IN_WORLD_UNITS = 60.0f;
    static inline constexpr float PART_LENGTH_IN_WORLD_UNITS = 20.0f;
    static inline std::string const CHUNKS_DIRECTORY =
        "Assets\\Unity\\Prefabs\\Chunks Completely Unpacked";
};

// ////////////////////////////////////////////////////////////////////////// //
