// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include "PlayerControllerScript.hpp"

#include <cmath>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "PointLight.h"
#include "Systems/Systems.hpp"
#include "Window.h"
#include "easings.hpp"

// //////////////////////////////////////////////////////////////// Utilities //
DirectX::XMFLOAT3 operator+(DirectX::XMFLOAT3 const& a,
                            DirectX::XMFLOAT3 const& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

DirectX::XMFLOAT3& operator+=(DirectX::XMFLOAT3& a,
                              DirectX::XMFLOAT3 const& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

DirectX::XMFLOAT3 operator-(DirectX::XMFLOAT3 const& a,
                            DirectX::XMFLOAT3 const& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

DirectX::XMFLOAT3& operator-=(DirectX::XMFLOAT3& a,
                              DirectX::XMFLOAT3 const& b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

DirectX::XMFLOAT3 operator*(DirectX::XMFLOAT3 const& a, float const b) {
    return {a.x * b, a.y * b, a.z * b};
}

DirectX::XMFLOAT3& operator*=(DirectX::XMFLOAT3& a, float const b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

DirectX::XMFLOAT3 operator/(DirectX::XMFLOAT3 const& a, float const b) {
    return {a.x / b, a.y / b, a.z / b};
}

DirectX::XMFLOAT3& operator/=(DirectX::XMFLOAT3& a, float const b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
    return a;
}

float length(DirectX::XMFLOAT3 const& a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// ///////////////////////////////////////////////////////// Factory function //
extern "C" PLAYERCONTROLLERSCRIPT_API void create(
    std::shared_ptr<Script>& script, Entity entity) {
    script = std::make_shared<PlayerControllerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
PlayerControllerScript::PlayerControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void PlayerControllerScript::setup() {
    // Set event listeners
    registry.listen<OnCollisionEnter>(
        MethodListener(PlayerControllerScript::onCollisionEnter));
    registry.listen<OnGameStateChange>(
        MethodListener(PlayerControllerScript::onGameStateChange));

    // Set utility functors
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };

    // Setup the controller
    currentForm = humanForm;
    canChangeForm = true;
    // torchLight = Entity(torch).get<Light>(); // TODO

    // Set the forms
    eagleForm = registry.system<PropertySystem>()
                    ->findEntityByName("Eagle Form")
                    .at(0)
                    .id;
    humanForm = registry.system<PropertySystem>()
                    ->findEntityByName("Human Form")
                    .at(0)
                    .id;
    catForm = registry.system<PropertySystem>()
                  ->findEntityByName("Cat Form")
                  .at(0)
                  .id;
    currentForm = humanForm;
    torch = registry.system<PropertySystem>()
                ->findEntityByName("Player Torch")
                .at(0)
                .id;

    groundCheck = registry.system<PropertySystem>()
                      ->findEntityByName("GroundCollision")
                      .at(0)
                      .id;

    Entity(torch).add<Light>({.pointLight = std::make_shared<PointLight>(
                                  registry.system<WindowSystem>()->gfx())});
    Entity(torch).get<Light>().pointLight->setIntensity(lightValue);

    // Activate all forms
    Entity(humanForm).get<Properties>().active = true;
    Entity(eagleForm).get<Properties>().active = true;

    originalScaleWolf = Entity(humanForm).get<Transform>().scale;
    originalScaleEagle = Entity(eagleForm).get<Transform>().scale;
    originalOffsetWolf = entity.get<Transform>().position.y -
                         Entity(humanForm).get<Transform>().position.y;
    originalOffsetEagle = entity.get<Transform>().position.y -
                          Entity(eagleForm).get<Transform>().position.y;

    entity.add<CheckCollisions>({});
    Entity(eagleForm).add<CheckCollisions>({});
    Entity(humanForm).add<CheckCollisions>({});
    Entity(catForm).add<CheckCollisions>({});
    Entity(groundCheck).add<CheckCollisions>({});

    // Make all player forms refractive
    Entity(eagleForm).add<Refractive>({});
    Entity(humanForm).add<Refractive>({});
    Entity(catForm).add<Refractive>({});

    originalTorchColor = Entity(torch).get<Light>().pointLight->getColor();
};

void PlayerControllerScript::update(float const deltaTime) {
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
            doGameLogic(deltaTime);
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

void PlayerControllerScript::doGameLogic(float const deltaTime) {
    static float flightHeight = 1.5f;

    // Set key statuses
    static bool inputLaneDownKey = false, inputLaneUpKey = false,
                inputAscendKey = false, inputChangeFormCatKey = false;

    // Get keyboard input
    inputLaneDown = isKeyPressed('S');
    inputLaneUp = isKeyPressed('W');
    inputAscend = isKeyPressed(' ');
    inputChangeFormCat = false;

    registry.system<PropertySystem>()->activateEntity(groundCheck,
                                                      !inputAscend);
    isGrounded |= inputAscend;

    if (firstThrust) {
        if (entity.get<Transform>().position.y - idlePosition.y <
            flightHeight) {
            moveInput.y =
                interpolate(easeOutQuad, moveInput.y, 3.0f, 0.1f, deltaTime);
        } else {
            firstThrust = false;
        }
    } else {
        if (currentForm == eagleForm) {
            if (inputAscend) {
                moveInput.y = interpolate(easeOutQuint, moveInput.y, 0.75f,
                                          0.01f, deltaTime);
            } else {
                moveInput.y = interpolate(easeOutSine, moveInput.y, -1.0f, 0.1f,
                                          deltaTime);
            }
        } else {
            moveInput.y = 0.0f;
        }
    }

    // Ascend
    if (inputAscend && !inputAscendKey) {
        inputAscendKey = true;
    }
    if (!inputAscend && inputAscendKey) {
        inputAscendKey = false;
    }

    // Change current lane - down
    if (inputLaneDown && !inputLaneDownKey) {
        inputLaneDownKey = true;
        currentLane -= inputLaneDown ? 1 : 0;
    }
    if (!inputLaneDown && inputLaneDownKey) {
        inputLaneDownKey = false;
    }

    // Change current lane - up
    if (inputLaneUp && !inputLaneUpKey) {
        inputLaneUpKey = true;
        currentLane += inputLaneUp ? 1 : 0;
    }
    if (!inputLaneUp && inputLaneUpKey) {
        inputLaneUpKey = false;
    }

    // Change form - eagle/human
    if (inputAscend) {
        if (canChangeForm) {
            // TODO: Potential explosion effect
            // Object.Destroy(GameObject.Instantiate(
            //                   explosionPrefab,
            //                   currentForm.transform.position,
            //                   Quaternion.identity),
            //               10.0f);

            if (currentForm != eagleForm) {
                changeForm(eagleForm);
                Entity(eagleForm).get<Animator>().animationTime = 15.0f;
                rb = entity.get<Rigidbody>();
                entity.remove<Rigidbody>();
                firstThrust = true;
                isGrounded = false;
                idlePosition = entity.get<Transform>().position;
            }
        } else {
            // TODO: Potential explosion effect
            // Object.Destroy(
            //    GameObject.Instantiate(explosionPrefabFail,
            //                           currentForm.transform.position,
            //                           Quaternion.identity),
            //    10.0f);
        }
    } else if (isGrounded && !firstThrust) {
        changeForm(humanForm);

        if (!entity.has<Rigidbody>()) {
            entity.add<Rigidbody>(rb);
            entity.get<Rigidbody>().velocity = 0.0f;
        }
    }

    // Change form - cat/human
    // if (inputChangeFormCat && !inputChangeFormCatKey) {
    //    inputChangeFormCatKey = true;

    //    if (canChangeForm) {
    //        // TODO: Potential explosion effect
    //        // Object.Destroy(GameObject.Instantiate(
    //        //                   explosionPrefab,
    //        //                   currentForm.transform.position,
    //        //                   Quaternion.identity),
    //        //               10.0f);

    //        if (currentForm != catForm) {
    //            changeForm(catForm);
    //        } else if (currentForm == catForm) {
    //            changeForm(humanForm);
    //        }
    //    } else {
    //        // TODO: Potential explosion effect
    //        // Object.Destroy(
    //        //    GameObject.Instantiate(explosionPrefabFail,
    //        //                           currentForm.transform.position,
    //        //                           Quaternion.identity),
    //        //    10.0f);
    //    }
    //}
    // if (!inputChangeFormCat && inputChangeFormCatKey) {
    //    inputChangeFormCatKey = false;
    //}

    if (!canChangeForm) {
        // StartCoroutine(trapEntered());
        if (previousAnimSpeed == 0.0f) {
            previousAnimSpeed =
                Entity(currentForm).get<Skybox>().animationSpeed;
        }

        Entity(currentForm).get<Skybox>().animationSpeed = 2.0f;

        canChangeFormTimer += deltaTime;
        if (canChangeFormTimer >= canChangeFormCooldown) {
            canChangeForm = true;
            canChangeFormTimer = 0.0f;
            Entity(currentForm).get<Skybox>().animationSpeed =
                previousAnimSpeed;
            previousAnimSpeed = 0.0f;
        }
    }

    transitionForms(deltaTime);

    // Move only in lane increments
    moveInput.x = 1.0f;  // Mathf.Clamp(Input.GetAxisRaw("Horizontal")
                         // + 1.0f, 0.5f, 1.5f);
    // moveInput.y = -Input.GetAxisRaw("Horizontal");
    moveInput.z = currentLane * laneWidth - entity.get<Transform>().position.z;

    // Rotate slightly when changing the lane
    entity.get<Transform>().euler.y = interpolate(
        easeOutSine, entity.get<Transform>().euler.y,
        (moveInput.z > 0.0f ? -1.0f : 1.0f) *
            (std::min)(
                35.0f * DirectX::XMConvertToRadians(std::abs(moveInput.z)),
                DirectX::XMConvertToRadians(30.0f)),
        0.004f, deltaTime);

    entity.get<Transform>().euler.z =
        interpolate(easeOutSine, entity.get<Transform>().euler.z,
                    (moveInput.y + 0.25f > 0.0f ? 1.0f : -1.0f) *
                        (std::min)(15.0f * DirectX::XMConvertToRadians(
                                               std::abs(moveInput.y + 0.25f)),
                                   DirectX::XMConvertToRadians(15.0f)),
                    0.04f, deltaTime);

    if (currentForm == humanForm) {
        moveInput.y = 0.0f;

        // humanForm
        //    .GetComponent<RPGCharacterAnims.RPGCharacterInputControllerFREE>()
        //    .inputHorizontal = Input.GetAxisRaw("Horizontal");
        // humanForm
        //    .GetComponent<RPGCharacterAnims.RPGCharacterInputControllerFREE>()
        //    .inputVertical = Input.GetAxisRaw("Vertical");
        // humanForm
        //    .GetComponent<RPGCharacterAnims.RPGCharacterInputControllerFREE>()
        //    .moveInput = moveInput;
    } else if (currentForm == eagleForm) {
        // eagleForm.GetComponent<DemoController>().horizontal =
        //    Input.GetAxisRaw("Horizontal");
        // eagleForm.GetComponent<DemoController>().vertical =
        //    Input.GetAxisRaw("Vertical");

        // currentVelocity =
        //    Vector3.MoveTowards(currentVelocity, moveInput * runSpeed,
        //                        movementAcceleration * Time.deltaTime);
    } else if (currentForm == catForm) {
        moveInput.y = 0.0f;

        // currentVelocity =
        //    Vector3.MoveTowards(currentVelocity, moveInput * runSpeed,
        //                        movementAcceleration * Time.deltaTime);
    }

    // Calculate the velocity
    // currentVelocity = (moveInput * runSpeed) - currentVelocity;
    currentVelocity = (moveInput * runSpeed);
    /* if (length(currentVelocity) > runSpeed) { */
    /*     currentVelocity /= length(currentVelocity); */
    /*     currentVelocity *= runSpeed; */
    /* } */
    /* if (length(currentVelocity) <= 1.0f) { */
    /*     currentVelocity *= 0.0f; */
    /* } */
    /* currentVelocity.x = std::lerp(currentVelocity.x, 0.0f, 0.5f); */
    /* currentVelocity.y = std::lerp(currentVelocity.y, 0.0f, 0.5f); */
    /* currentVelocity.z = std::lerp(currentVelocity.z, 0.0f, 0.5f); */

    entity.get<Transform>().position += (currentVelocity * deltaTime);

    // Update torch range

    // torch.transform.position = new Vector3(
    //    currentForm.transform.position.x, currentForm.transform.position.y +
    //    3,
    //    // torch.transform.position.y,
    //    currentForm.transform.position.z - 2.0f);

    // if (torchLight.range > 7) {
    //    torchLight.range -= Time.deltaTime * 1.4f;
    //} else {
    //    torchLight.range = 7;
    //    torchLight.intensity =
    //        Mathf.Lerp(3f, 3.5f, Mathf.PingPong(Time.time, 1f));
    //}

    auto lightProportion = [](float const x) {
        return 1.0f / (3.0f * (x + 0.25f)) - 0.25f;
    };
    aCValue = std::lerp(minC, maxC, lightProportion(lightValue));
    aQValue = std::lerp(minQ, maxQ, lightProportion(lightValue));
    intensityValue =
        std::lerp(maxIntensity, minIntensity, lightProportion(lightValue));

    lightValue += LIGHT_VALUE_CHANGE_PER_SECOND * deltaTime;

    // Turn off screen after light reaches certain value
    if (lightValue < LIGHT_VALUE_BLACK_FADE_START && lightValue >= 0.0f) {
        blackProportion = std::lerp(
            1.0f, 0.0f,
            std::pow(1.0f - lightValue / LIGHT_VALUE_BLACK_FADE_START, 1.5f));
    } else if (lightValue > 0.0f) {
        // This line reasures that when player picks up flame after screen goes
        // black it will turn it back on
        blackProportion =
            interpolate(easeOutQuint, blackProportion, 1.0f, 0.1f, deltaTime);
    } else {
        blackProportion = 0.0f;
    }
    registry.system<BillboardRenderSystem>()->setBlackProportion(
        blackProportion);
    if (lightValue < 0.0f) {
        lightValue = 0.0f;
        die();
    }

    Entity(torch).get<Light>().pointLight->setIntensity(intensityValue);
    Entity(torch).get<Light>().pointLight->setAttenuationC(aCValue);
    Entity(torch).get<Light>().pointLight->setAttenuationQ(aQValue);

    DirectX::XMFLOAT4 torchColor = originalTorchColor;
    torchColor.x = std::lerp(torchColor.x + 0.5f, torchColor.x, lightValue);
    Entity(torch).get<Light>().pointLight->setColor(torchColor);

    isGrounded = false;
};

// ------------------------------------------------------------- Events -- == //
void PlayerControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == groundCheck || event.b.id == groundCheck) {
        auto other =
            Entity(event.a.id == groundCheck ? event.b.id : event.a.id);
        auto otherTag = other.get<Properties>().tag;

        if (other.id == entity.id) {
            return;
        } else if (otherTag == "Ground") {
            isGrounded |= true;
        }
    }

    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
        auto otherTag = other.get<Properties>().tag;

        if (otherTag == "EnemySpawnPoint") {
            return;
        } else if (otherTag == "Torch") {
            registry.system<GraphSystem>()->destroyEntityWithChildren(other);
            resetTorchLight();
        } else if (otherTag == "Trap") {
            registry.system<GraphSystem>()->destroyEntityWithChildren(other);
            canChangeForm = false;
        } else if (otherTag == "Waterfall") {
            changeForm(humanForm);
            if (!entity.has<Rigidbody>()) {
                entity.add<Rigidbody>(rb);
            }
            canChangeForm = false;
        } else if (otherTag == "DeathCollider") {
            die();
        } else if (otherTag == "Boundary") {
            return;
        } else if (other.id == groundCheck) {
            return;
        } else if (otherTag == "Enemy" || otherTag == "Rook") {
            die();

            // registry.destroyEntity(other);
            // registry.system<PropertySystem>()->activateEntity(loseText,
            // true);

            // StartCoroutine(waitToResetLvl());
        } else {
            auto& boxCollider = entity.get<BoxCollider>();
            boxCollider.separatingVectorSum += event.minSeparatingVector;

            if (std::abs(event.minSeparatingVector.x) > 0.0f) {
                boxCollider.numberOfCollisions.x++;
            }
            if (std::abs(event.minSeparatingVector.y) > 0.0f) {
                boxCollider.numberOfCollisions.y++;
                if (entity.has<Rigidbody>()) {
                    entity.get<Rigidbody>().velocity = 0.0f;
                }
            }
            if (std::abs(event.minSeparatingVector.z) > 0.0f) {
                boxCollider.numberOfCollisions.z++;
            }

            // Possible solution to the adjacent box colliders problem
            // if (boxCollider.numberOfCollisions.x > 1 &&
            //     event.minSeparatingVector.x > 0.0f) {
            //     entity.get<Transform>().position.x +=
            //         std::abs(event.minSeparatingVector.x) * 0.5f;
            // }
        }
    }
}
void PlayerControllerScript::onGameStateChange(OnGameStateChange const& event) {
    currentState = event.nextState;
}

// ------------------------------------------------------------ Methods -- == //
void PlayerControllerScript::die() {
    registry.send(OnGameStateChange{.nextState = DEATH_RESULTS});
}

void PlayerControllerScript::changeForm(EntityId const& newForm) {
    // TODO: Add explosion after 10 seconds, needs particle system, low
    // priority
    // Object.Destroy(GameObject.Instantiate(explosionPrefab,
    //                                      currentForm.transform.position,
    //                                      Quaternion.identity),
    //               10.0f);

    // Change form
    EntityId previousForm = currentForm;
    currentForm = newForm;
}

void PlayerControllerScript::resetTorchLight() { lightValue = 1.0f; }

void PlayerControllerScript::transitionForms(float const deltaTime) {
    auto& currentTransform = Entity(currentForm).get<Transform>();
    auto& otherTransform =
        Entity(currentForm == eagleForm ? humanForm : eagleForm)
            .get<Transform>();

    // Upscale the current form
    constexpr float UPSCALE_SMOOTHING = 0.1f;
    currentTransform.scale.x = interpolate(
        easeOutQuint, currentTransform.scale.x,
        currentForm == humanForm ? originalScaleWolf.x : originalScaleEagle.x,
        UPSCALE_SMOOTHING, deltaTime);
    currentTransform.scale.y = interpolate(
        easeOutQuint, currentTransform.scale.y,
        currentForm == humanForm ? originalScaleWolf.y : originalScaleEagle.y,
        UPSCALE_SMOOTHING, deltaTime);
    currentTransform.scale.z = interpolate(
        easeOutQuint, currentTransform.scale.z,
        currentForm == humanForm ? originalScaleWolf.z : originalScaleEagle.z,
        UPSCALE_SMOOTHING, deltaTime);

    // Downscale the other form
    constexpr float DOWNSCALE_SMOOTHING = 0.1f;
    otherTransform.scale.x = interpolate(easeOutQuint, otherTransform.scale.x,
                                         0.0f, DOWNSCALE_SMOOTHING, deltaTime);
    otherTransform.scale.y = interpolate(easeOutQuint, otherTransform.scale.y,
                                         0.0f, DOWNSCALE_SMOOTHING, deltaTime);
    otherTransform.scale.z = interpolate(easeOutQuint, otherTransform.scale.z,
                                         0.0f, DOWNSCALE_SMOOTHING, deltaTime);

    // Additionally move the wolf when scaling
    if (currentForm == humanForm) {
        currentTransform.position.y = interpolate(
            easeOutQuint, currentTransform.position.y,
            currentForm == humanForm ? originalOffsetWolf : originalOffsetEagle,
            0.1f, deltaTime);
    } else {
        otherTransform.position.y = interpolate(
            easeOutQuad, otherTransform.position.y, 2.0f, 0.1f, deltaTime);
    }
}

float PlayerControllerScript::ascend(float const x, float const thrustForce) {
    static constexpr float A = -1.0f, B = 4.5f, C = 0.0f;
    return thrustForce * (A * std::pow(x, 2) + B * x + C);
}

// ////////////////////////////////////////////////////////////////////////// //
