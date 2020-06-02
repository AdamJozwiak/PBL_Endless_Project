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

    // temp
    humanForm = entity.id;
    eagleForm = entity.id;
    catForm = entity.id;
    currentForm = humanForm;
};

void PlayerControllerScript::update(float const deltaTime) {
    static bool inputLaneDownKey = false;
    static bool inputLaneUpKey = false;

    inputLaneDown = isKeyPressed('S');
    inputLaneUp = isKeyPressed('W');

    if (inputLaneDown && !inputLaneDownKey) {
        inputLaneDownKey = true;
        // Change current lane
        currentLane -= inputLaneDown ? 1 : 0;
    }
    if (!inputLaneDown && inputLaneDownKey) {
        inputLaneDownKey = false;
    }

    if (inputLaneUp && !inputLaneUpKey) {
        inputLaneUpKey = true;
        // Change current lane
        currentLane += inputLaneUp ? 1 : 0;
    }
    if (!inputLaneUp && inputLaneUpKey) {
        inputLaneUpKey = false;
    }

    if (isKeyPressed('E') && canChangeForm) {
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
    }

    if (isKeyPressed('E') && !canChangeForm) {
        // TODO: Potential explosion effect
        // Object.Destroy(
        //    GameObject.Instantiate(explosionPrefabFail,
        //                           currentForm.transform.position,
        //                           Quaternion.identity),
        //    10.0f);
    }

    if (isKeyPressed('C') && canChangeForm) {
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
    }

    if (isKeyPressed('C') && !canChangeForm) {
        // TODO: Potential explosion effect
        // Object.Destroy(
        //    GameObject.Instantiate(explosionPrefabFail,
        //                           currentForm.transform.position,
        //                           Quaternion.identity),
        //    10.0f);
    }

    if (!canChangeForm) {
        // StartCoroutine(trapEntered());
    }

    // Move only in lane increments
    moveInput.x = 0.0f;  // Mathf.Clamp(Input.GetAxisRaw("Horizontal")
                         // + 1.0f, 0.5f, 1.5f);
    // moveInput.y = -Input.GetAxisRaw("Horizontal");
    moveInput.z = currentLane * laneWidth -
                  Entity(currentForm).get<Transform>().position.z;

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
        currentVelocity = (moveInput * runSpeed) - currentVelocity;
        if (length(currentVelocity) > movementAcceleration * deltaTime) {
            currentVelocity /= length(currentVelocity);
            currentVelocity *= movementAcceleration * deltaTime;
        }
        if (length(currentVelocity) <= 10.0f * deltaTime) {
            currentVelocity *= 0.0f;
        }

        Entity(humanForm).get<Transform>().position +=
            (currentVelocity * deltaTime);
    } else if (currentForm == eagleForm) {
        // eagleForm.GetComponent<DemoController>().horizontal =
        //    Input.GetAxisRaw("Horizontal");
        // eagleForm.GetComponent<DemoController>().vertical =
        //    Input.GetAxisRaw("Vertical");

        // currentVelocity =
        //    Vector3.MoveTowards(currentVelocity, moveInput * runSpeed,
        //                        movementAcceleration * Time.deltaTime);
        // Entity(eagleForm).get<Transform>().position += currentVelocity *
        // deltaTime;
    } else {
        // moveInput.y = 0.0f;
        // currentVelocity =
        //    Vector3.MoveTowards(currentVelocity, moveInput * runSpeed,
        //                        movementAcceleration * Time.deltaTime);
        // catForm.transform.position += currentVelocity * Time.deltaTime;
    }

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

    EntityId previousForm = currentForm;
    currentForm = newForm;

    Entity(newForm).get<Transform>().position =
        Entity(previousForm).get<Transform>().position;

    registry.system<PropertySystem>()->activateEntity(newForm, true);
    registry.system<PropertySystem>()->activateEntity(previousForm, false);

    // mainCamera.player = newForm;
    // mainCamera.cameraTarget = newForm;
    // mainCamera.SetPosition();
}

void PlayerControllerScript::resetTorchLight() {
    // torchLight.range = 20;
    // torchLight.intensity = 4;
}

// ////////////////////////////////////////////////////////////////////////// //
