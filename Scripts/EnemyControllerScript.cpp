// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "EnemyControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"
#include "easings.hpp"
#include "math-operators.hpp"
#include "script-factory.hpp"

// ///////////////////////////////////////////////////////// Factory function //
SCRIPT_FACTORY(EnemyControllerScript)

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
EnemyControllerScript::EnemyControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void EnemyControllerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(EnemyControllerScript::onCollisionEnter));
    registry.listen<OnGameStateChange>(
        MethodListener(EnemyControllerScript::onGameStateChange));
    // registry.listen<OnTriggerEnter>(
    //    MethodListener(EnemyControllerScript::onTriggerEnter));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
    playerId =
        registry.system<PropertySystem>()->findEntityByTag("Player").at(0).id;

    entity.add<CheckCollisions>({});
};

void EnemyControllerScript::update(float const deltaTime) {
    switch (currentState) {
        case GAME_LAUNCH_FADE_IN: {
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
            if (entity.get<Transform>().position.x -
                    Entity(playerId).get<Transform>().position.x <=
                playerDistance) {
                if (movementType == Bishop)
                    moveBishop(deltaTime);
                else
                    moveRook(deltaTime);
            }
            timeToBounce += deltaTime;
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
void EnemyControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }

    // This should be inside onTriggerEnter handler, but maybe will work here,
    // too
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto const& other =
            Entity(event.a.id == entity.id ? event.b.id : event.a.id);
        auto const& otherTag = other.get<Properties>().tag;

        if (otherTag == "Boundary" && timeToBounce >= 0.3f) {
            movingLeft = !movingLeft;
            timeToBounce = 0.0f;

            auto& boxCollider = entity.get<BoxCollider>();
            boxCollider.separatingVectorSum += event.minSeparatingVector;
            if (std::abs(event.minSeparatingVector.x) > 0.0f) {
                boxCollider.numberOfCollisions.x++;
            }
            if (std::abs(event.minSeparatingVector.y) > 0.0f) {
                boxCollider.numberOfCollisions.y++;
            }
            if (std::abs(event.minSeparatingVector.z) > 0.0f) {
                boxCollider.numberOfCollisions.z++;
            }
        }
    }
}
void EnemyControllerScript::onGameStateChange(OnGameStateChange const& event) {
    currentState = event.nextState;
}

// void EnemyControllerScript::onTriggerEnter(OnTriggerEnter const& event) {}

// ------------------------------------------------------------ Methods -- == //
void EnemyControllerScript::moveBishop(float const deltaTime) {
    auto& transform = entity.get<Transform>();
    transform.position.x = interpolate(
        easeOutQuad, transform.position.x,
        transform.position.x + (-angle) * deltaTime * speed, 0.01f, deltaTime);
    transform.position.z = interpolate(
        easeOutQuad, transform.position.z,
        transform.position.z +
            ((movingLeft ? (-1.0f) : 1.0f) * angle) * deltaTime * speed,
        0.01f, deltaTime);
    entity.set<Transform>(transform);
}

void EnemyControllerScript::moveRook(float const deltaTime) {
    float tmp = deltaTime;
    auto& transform = entity.get<Transform>();
    // if ((int) tmp % 3 == 0) movingSideways = !movingSideways;
    if (movingSideways) {
        transform.position.z =
            interpolate(easeOutQuad, transform.position.z,
                        transform.position.z +
                            ((movingLeft ? (-1.0f) : 1.0f)) * deltaTime * speed,
                        0.01f, deltaTime);
    } else {
        transform.position.x =
            interpolate(easeOutQuad, transform.position.x,
                        transform.position.x +
                            ((movingLeft ? (-1.0f) : 1.0f)) * deltaTime * speed,
                        0.01f, deltaTime);
    }
    entity.set<Transform>(transform);
}

void EnemyControllerScript::setMovementType(MovementType mt, bool movingS) {
    movementType = mt;
    if (mt == Rook) {
        movingSideways = movingS;
    }
}

// //////////////////////////////////////////////////////////////////////////
// //
