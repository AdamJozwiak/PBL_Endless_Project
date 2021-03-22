// ///////////////////////////////////////////////////////////////// Includes //
#include "PhysicsSystem.hpp"

// ECS
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //

void PhysicsSystem::filters() {
    filter<Active>().filter<Transform>().filter<Rigidbody>();
}

void PhysicsSystem::setup() {
    gravityFactor = 9.81f;
    secondsToAchieveMaxVelocity = 0.5f;
    maxVelocity = secondsToAchieveMaxVelocity * (-gravityFactor);
    movementFactor = 4.0f;
}

void PhysicsSystem::update(float deltaTime) {
    static int pom = 0;
    if (pom++ < 10) {
        return;
    } else {
        pom = 100;
    }
    for (auto entity : entities) {
        gravity(entity, deltaTime);
    }
}

void PhysicsSystem::release() {}

void PhysicsSystem::gravity(Entity& entity, float deltaTime) {
    entity.get<Rigidbody>().velocity -= gravityFactor * deltaTime;
    if (entity.get<Rigidbody>().velocity < maxVelocity) {
        entity.get<Rigidbody>().velocity = maxVelocity;
    }

    auto& transform = entity.get<Transform>();
    transform.position.y +=
        movementFactor * entity.get<Rigidbody>().velocity * deltaTime;
    entity.set<Transform>(transform);
}
