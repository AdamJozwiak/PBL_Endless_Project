// ///////////////////////////////////////////////////////////////// Includes //
#include "TestScript.hpp"

#include <array>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Systems/Systems.hpp"
#include "Window.h"

int counter = 0;
constexpr int NUMBER_OF_MONKEYS = 10;
std::array<EntityId, NUMBER_OF_MONKEYS> monkeys;
void spawnMonkey(Entity entity) {
    if (counter < 10) {
        monkeys[counter] = Registry::instance().createEntity().id;
        Entity(monkeys[counter])
            .add<Transform>([&]() {
                auto transform = entity.get<Transform>();
                transform.position.x = 0.0f;
                transform.position.y = counter * 2.0f;
                transform.position.z = 0.0f;
                return transform;
            }())
            .add<MeshFilter>(entity.get<MeshFilter>())
            .add<Renderer>(entity.get<Renderer>())
            .add<AABB>(Registry::instance().system<ColliderSystem>()->AddAABB(
                entity.get<MeshFilter>().model->verticesForCollision));
        counter++;
    } else {
        counter = 0;
        for (auto const& entity : monkeys) {
            Registry::instance().destroyEntity(entity);
    }
    }
}

// ///////////////////////////////////////////////////////// Factory function //
extern "C" TESTSCRIPT_API void create(std::shared_ptr<Script>& script,
                                      Entity entity) {
    script = std::make_shared<TestScript>(entity);
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
TestScript::TestScript(Entity const& entity) : Script(entity){};

// ----------------------------------------- System's virtual functions -- == //
void TestScript::setup() {
    registry.listen<OnCollisionEnter>(
        MethodListener(TestScript::onCollisionEnter));
    registry.listen<OnButtonClick>(
        MethodListener(TestScript::onButtonClick));
    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };
};

void TestScript::update(float const deltaTime) {
    lastDeltaTime = deltaTime;
    auto& transform = entity.get<Transform>();
    // transform.eulerAngle_y += 45.0f * deltaTime;

    lastX = transform.position.x;
    lastY = transform.position.y;
    lastZ = transform.position.z;

    if (isKeyPressed(VK_SPACE)) {
        if (isKeyPressed('A')) {
            transform.position.x -= 5.0f * deltaTime;
        }
        if (isKeyPressed('D')) {
            transform.position.x += 5.0f * deltaTime;
        }
        if (isKeyPressed('K')) {
            transform.position.y += 5.0f * deltaTime;
        }
        if (isKeyPressed('J')) {
            transform.position.y -= 5.0f * deltaTime;
        }
        if (isKeyPressed('W')) {
            transform.position.z += 5.0f * deltaTime;
        }
        if (isKeyPressed('S')) {
            transform.position.z -= 5.0f * deltaTime;
        }
    }

    static float timer = 0.0f;
    timer += deltaTime;
    if (isKeyPressed('Q')) {
        if (timer > 0.2f) {
            timer = 0.0f;
            spawnMonkey(entity);
        }
    }
};

// ------------------------------------------------------------- Events -- == //
void TestScript::onCollisionEnter(OnCollisionEnter const& event) {
    if (event.a.id == entity.id || event.b.id == entity.id) {
        auto& transform = entity.get<Transform>();
        auto other = Entity(event.a.id == entity.id ? event.b.id : event.a.id);
        auto& otherTransform = other.get<Transform>();

        if (isKeyPressed(VK_SHIFT)) {
            otherTransform.euler.x += 45.0f * lastDeltaTime;
        }

        if (isKeyPressed(VK_CONTROL)) {
            registry.destroyEntity(other);
        }

        if (isKeyPressed(VK_SPACE)) {
            otherTransform.position.x += transform.position.x - lastX;
            otherTransform.position.y += transform.position.y - lastY;
            otherTransform.position.z += transform.position.z - lastZ;
        } else {
            transform.position.x = lastX;
            transform.position.y = lastY;
            transform.position.z = lastZ;
        }
    }
}

void TestScript::onButtonClick(OnButtonClick const& event) {}

// ------------------------------------------------------------ Methods -- == //
void TestScript::method() {}

// ////////////////////////////////////////////////////////////////////////// //
