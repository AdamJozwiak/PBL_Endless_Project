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
    // Entity(torch).get<Light>().pointLight->setAttenuationQ(0.05f);

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
    canChangeFormCooldown = 3.0f;
};

void PlayerControllerScript::update(float const deltaTime) {
    static float wait = 2.0f;
    static float flightHeight = 1.0f;

    if (wait >= 0.0f) {
        entity.get<Transform>().position.y = 0.0f;
        wait -= deltaTime;
        return;
    }

    // Set key statuses
    static bool inputLaneDownKey = false, inputLaneUpKey = false,
                inputAscendKey = false, inputDescendKey = false,
                inputChangeFormEagleKey = false, inputChangeFormCatKey = false;

    // Get keyboard input
    inputLaneDown = isKeyPressed('S');
    inputLaneUp = isKeyPressed('W');
    inputAscend = isKeyPressed(' ');
    inputChangeFormCat = false;

    if (!firstThrust) {
        if (currentForm == eagleForm) {
            if (inputAscendKey) {
                // moveInput.y = ascend(deltaTime, 10.0f);
                moveInput.y = interpolate(easeOutQuad, moveInput.y, 0.75f, 0.1f,
                                          deltaTime);
            } else {
                // moveInput.y = -0.5f;
                moveInput.y = interpolate(easeOutSine, moveInput.y, -1.0f, 0.1f,
                                          deltaTime);
            }
        } else {
            moveInput.y = 0.0f;
            // moveInput.y =
            //     interpolate(easeOutSine, moveInput.y, 0.0f, 0.1f, deltaTime);
        }

        // Ascend
        if (inputAscend && !inputAscendKey) {
            inputAscendKey = true;
        }
        if (!inputAscend && inputAscendKey) {
            inputAscendKey = false;
        }
    } else {
        if (entity.get<Transform>().position.y - idlePosition.y <
            flightHeight) {
            // moveInput.y = ascend(deltaTime, 20.0f);
            moveInput.y =
                interpolate(easeOutCubic, moveInput.y, 3.0f, 0.1f, deltaTime);
            /* moveInput.y = */
            /*     interpolate(easeOutSine, moveInput.y, -1.0f, 0.1f,
             * deltaTime); */
        } else {
            firstThrust = false;
        }
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
    if (inputAscend && !inputChangeFormEagleKey) {
        inputChangeFormEagleKey = true;

        if (canChangeForm) {
            // TODO: Potential explosion effect
            // Object.Destroy(GameObject.Instantiate(
            //                   explosionPrefab,
            //                   currentForm.transform.position,
            //                   Quaternion.identity),
            //               10.0f);

            if (currentForm != eagleForm) {
                changeForm(eagleForm);
                rb = entity.get<Rigidbody>();
                entity.remove<Rigidbody>();
                firstThrust = true;
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
    }
    if (isGrounded && inputChangeFormEagleKey) {
        inputChangeFormEagleKey = false;
        if (canChangeForm) {
            changeForm(humanForm);
            if (!entity.has<Rigidbody>()) {
                entity.add<Rigidbody>(rb);
                entity.get<Rigidbody>().velocity = 0.0f;
            }
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
                    (moveInput.y + 0.5f > 0.0f ? 1.0f : -1.0f) *
                        (std::min)(15.0f * DirectX::XMConvertToRadians(
                                               std::abs(moveInput.y + 0.5f)),
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
};

// ------------------------------------------------------------- Events -- == //
void PlayerControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if ((event.a.id == groundCheck &&
         Entity(event.b.id).get<Properties>().name == "Collider Ground") ||
        (Entity(event.a.id).get<Properties>().name == "Collider Ground" &&
         event.b.id == groundCheck)) {
        if (!firstThrust) {
            isGrounded = true;
        }
    } else {
        isGrounded = false;
    }

    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
        auto otherTag = other.get<Properties>().tag;

        if (otherTag == "EnemySpawnPoint") {
            return;
        } else if (otherTag == "Torch") {
            registry.system<GraphSystem>()->destroyEntityWithChildren(other);
            resetTorchLight(other);
        } else if (otherTag == "Trap") {
            registry.system<GraphSystem>()->destroyEntityWithChildren(other);
            canChangeForm = false;
        } else if (otherTag == "Waterfall") {
            changeForm(humanForm);
            if (!entity.has<Rigidbody>()) {
                entity.add<Rigidbody>(rb);
            }
        } else if (otherTag == "Boundary") {
            return;
        } else if (other.id == groundCheck) {
            return;
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

// ------------------------------------------------------------ Methods -- == //
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

void PlayerControllerScript::resetTorchLight(Entity light) {
    light.get<Light>().pointLight->setIntensity(1.5f);
    // light.get<Light>().pointLight->setAttenuationQ(0.05f);
}

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
