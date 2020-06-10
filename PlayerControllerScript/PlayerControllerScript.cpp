// ///////////////////////////////////////////////////////////////// Includes //
#include "PlayerControllerScript.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

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

    entity.add<CheckCollisions>({});

    // Make all player forms refractive
    Entity(eagleForm).add<Refractive>({});
    Entity(humanForm).add<Refractive>({});
    Entity(catForm).add<Refractive>({});
};

void PlayerControllerScript::update(float const deltaTime) {
    static float wait = 2.0f;

    if (wait >= 0.0f) {
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
    inputAscend = isKeyPressed('A');
    inputDescend = isKeyPressed('D');
    inputChangeFormEagle = isKeyPressed('E');
    inputChangeFormCat = isKeyPressed('C');

    if (currentForm == eagleForm) {
        if (inputAscendKey) {
            moveInput.y = 2.0f;
        } else if (inputDescendKey) {
            moveInput.y = -2.0f;
        } else {
            moveInput.y = 0.0f;
        }
    } else {
        moveInput.y = 0.0f;
    }

    // Ascend
    if (inputAscend && !inputAscendKey) {
        inputAscendKey = true;

        if (currentForm == eagleForm) {
            moveInput.y = 2.0f;
        } else {
            moveInput.y = 0.0f;
        }
    }
    if (!inputAscend && inputAscendKey) {
        inputAscendKey = false;
    }

    // Descend
    if (inputDescend && !inputDescendKey) {
        inputDescendKey = true;

        if (currentForm == eagleForm) {
            moveInput.y = -2.0f;
        } else {
            moveInput.y = 0.0f;
        }
    }
    if (!inputDescend && inputDescendKey) {
        inputDescendKey = false;
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
    if (inputChangeFormEagle && !inputChangeFormEagleKey) {
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
            } else {
                changeForm(humanForm);
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
    if (!inputChangeFormEagle && inputChangeFormEagleKey) {
        inputChangeFormEagleKey = false;
    }

    // Change form - cat/human
    if (inputChangeFormCat && !inputChangeFormCatKey) {
        inputChangeFormCatKey = true;

        if (canChangeForm) {
            // TODO: Potential explosion effect
            // Object.Destroy(GameObject.Instantiate(
            //                   explosionPrefab,
            //                   currentForm.transform.position,
            //                   Quaternion.identity),
            //               10.0f);

            if (currentForm != catForm) {
                changeForm(catForm);
            } else if (currentForm == catForm) {
                changeForm(humanForm);
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
    if (!inputChangeFormCat && inputChangeFormCatKey) {
        inputChangeFormCatKey = false;
    }

    if (!canChangeForm) {
        // StartCoroutine(trapEntered());
    }

    // Move only in lane increments
    moveInput.x = 1.0f;  // Mathf.Clamp(Input.GetAxisRaw("Horizontal")
                         // + 1.0f, 0.5f, 1.5f);
    // moveInput.y = -Input.GetAxisRaw("Horizontal");
    moveInput.z = currentLane * laneWidth - entity.get<Transform>().position.z;

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

    // Update player's position
    entity.get<Transform>().position += (currentVelocity * deltaTime);

    // Update torch position
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
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
        auto otherTag = other.get<Properties>().tag;

        if (otherTag == "EnemySpawnPoint") {
            return;
        } else if (otherTag == "Torch") {
            // TODO: Collect the torch
        } else if (otherTag == "Trap") {
            // TODO: Handle the trap
        } else if (otherTag == "Waterfall") {
            // TODO: Handle the waterfall
        } else if (otherTag == "Boundary") {
            return;
        } else {
            // TODO: Resolve the collision
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

    // Activate appropriate forms
    Entity(previousForm).get<Properties>().active = false;
    Entity(newForm).get<Properties>().active = true;
}

void PlayerControllerScript::resetTorchLight() {
    // torchLight.range = 20;
    // torchLight.intensity = 4;
}

// ////////////////////////////////////////////////////////////////////////// //
