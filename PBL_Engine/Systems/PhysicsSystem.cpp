// ///////////////////////////////////////////////////////////////// Includes //
#include "PhysicsSystem.hpp"

// ECS
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //

void PhysicsSystem::filters() { filter<Active>().filter<Rigidbody>(); }

void PhysicsSystem::setup() {
    gravityFactor = 9.81f;
    maxVelocity = -1000.0f;
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
    if (entity.get<Rigidbody>().velocity < maxVelocity * deltaTime) {
        entity.get<Rigidbody>().velocity = maxVelocity * deltaTime;
    } else {
        entity.get<Rigidbody>().velocity -= gravityFactor * deltaTime;
    }

    entity.get<Transform>().position.y +=
        entity.get<Rigidbody>().velocity * deltaTime;
}
