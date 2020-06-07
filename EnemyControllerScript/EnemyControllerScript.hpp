#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "EnemyControllerScriptAPI.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Script.hpp"

enum MovementType { Rook, Bishop };

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
    // void onTriggerEnter(OnTriggerEnter const &event);

    // -------------------------------------------------------- Methods -- == //
    void moveBishop(float deltaTime);
    void moveRook(float const deltaTime);
    void setMovementType(MovementType mt, bool movingS = true);

  private:
    // ============================================================== Data == //
    bool (*isKeyPressed)(int const key);

    EntityId loseText;

    /* IEnumerator waitToResetLvl() { */
    /*     yield return new WaitForSeconds(1.5f); */
    /*     SceneManager.LoadScene(0); */
    /* } */
    MovementType movementType = Bishop;
    float speed = 7.0f;
    float angle = 0.45f;
    bool movingLeft = true;
    // for Rook
    bool movingSideways = true;
    EntityId playerId;
    float playerDistance = 5.0f;
    float timeToBounce = 0.0f;
    float rookTimer = 0.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
