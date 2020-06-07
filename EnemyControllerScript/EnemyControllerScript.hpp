#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>

#include "ECS/Entity.hpp"
#include "EnemyControllerScriptAPI.hpp"
#include "Events/OnCollisionEnter.hpp"
#include "Script.hpp"

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
    void move(float deltaTime);

  private:
    // ============================================================== Data == //
    bool (*isKeyPressed)(int const key);

    EntityId loseText;

    /* IEnumerator waitToResetLvl() { */
    /*     yield return new WaitForSeconds(1.5f); */
    /*     SceneManager.LoadScene(0); */
    /* } */

    float speed = 8.0f;
    float angle = 0.45f;
    bool movingLeft = true;
    EntityId playerId;
    float playerDistance = 10.0f;
};

// ////////////////////////////////////////////////////////////////////////// //
