// ///////////////////////////////////////////////////////////////// Includes //
#include "CheckCollisionsSystem.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ----------------------------------------- System's virtual functions -- == //
void CheckCollisionsSystem::filters() {
    filter<Active>().filter<BoxCollider>().filter<CheckCollisions>();
}

void CheckCollisionsSystem::setup() {}

void CheckCollisionsSystem::release() {}

void CheckCollisionsSystem::update(float deltaTime) {}

// ////////////////////////////////////////////////////////////////////////// //
