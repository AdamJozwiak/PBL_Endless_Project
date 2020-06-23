#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "EnemyControllerScriptAPI.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Events/OnGameStateChange.hpp"
#include "Script.hpp"

enum MovementType { Rook, RookUp, RookForward, Bishop };

// //////////////////////////////////////////////////////////////////// Class //
class ENEMYCONTROLLERSCRIPT_API EnemyControllerScript : public Script {
  public:
    // ========================================================= Behaviour == //
    EnemyControllerScript(Entity const &entity);

    // ------------------------------------- System's virtual functions -- == //
    void setup() override;
    void update(float const deltaTime) override;

    // --------------------------------------------------------- Events -- == //
    void onCollisionEnter(OnCollisionEnter const &event);
    void onGameStateChange(OnGameStateChange const &event);
    // void onTriggerEnter(OnTriggerEnter const &event);

    // -------------------------------------------------------- Methods -- == //
    void moveBishop(float deltaTime);
    void moveRook(float const deltaTime);
    void setMovementType(MovementType mt, bool movingS, bool movingU = false);

  private:
    // ============================================================== Data == //
    GameState currentState = GAME;

    bool (*isKeyPressed)(int const key);

    EntityId loseText;

    /* IEnumerator waitToResetLvl() { */
    /*     yield return new WaitForSeconds(1.5f); */
    /*     SceneManager.LoadScene(0); */
    /* } */
    MovementType movementType = Bishop;
    float speed = 30.0f;
    float angle = 0.45f;
    bool movingLeft = true;
    // for Rook
    bool movingSideways = true;
    // for RookUp
    bool movingUp = false;
    bool moveDown = false;
    float startPos = 0.0f;
    float maxHeight = 5.0f;

    EntityId playerId;
    float playerDistance = 10.0f;
    float timeToBounce = 0.0f;
    float rookTimer = 0.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
