// ///////////////////////////////////////////////////////////////// Includes //
#include "EnemyControllerScript.hpp"

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

// ///////////////////////////////////////////////////////// Factory function //
extern "C" ENEMYCONTROLLERSCRIPT_API void create(
    std::shared_ptr<Script>& script, Entity entity) {
    script = std::make_shared<EnemyControllerScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
EnemyControllerScript::EnemyControllerScript(Entity const& entity)
    : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void EnemyControllerScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(EnemyControllerScript::onCollisionEnter));
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
    if (entity.get<Transform>().position.x -
            Entity(playerId).get<Transform>().position.x <=
        playerDistance) {
        move(deltaTime);
    }
};

// ------------------------------------------------------------- Events -- == //
void EnemyControllerScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
    }

    // This should be inside onTriggerEnter handler, but maybe will work here,
    // too
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);

        if (other.get<Properties>().tag == "Boundary") {
            movingLeft = !movingLeft;
        }
        if (other.get<Properties>().tag == "Player") {
            registry.destroyEntity(other);
            registry.system<PropertySystem>()->activateEntity(loseText, true);

            // StartCoroutine(waitToResetLvl());
        }
    }
}

// void EnemyControllerScript::onTriggerEnter(OnTriggerEnter const& event) {}

// ------------------------------------------------------------ Methods -- == //
void EnemyControllerScript::move(float const deltaTime) {
    entity.get<Transform>().position +=
        DirectX::XMFLOAT3(-angle, 0.0f, (movingLeft ? (-1.0f) : 1.0f) * angle) *
        deltaTime * speed;
}

// ////////////////////////////////////////////////////////////////////////// //
