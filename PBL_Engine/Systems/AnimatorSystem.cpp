// ///////////////////////////////////////////////////////////////// Includes //
#include "AnimatorSystem.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void AnimatorSystem::filters() { filter<Active>().filter<Animator>(); }

void AnimatorSystem::setup() {
    factors.insert({"Human Form", 32.5f});
    factors.insert({"Eagle Form", 65.0f});
}

void AnimatorSystem::update(float deltaTime) {
    // Advance the animation times
    for (Entity entity : entities) {
        auto const& name = entity.get<Properties>().name;
        auto& animationTime = entity.get<Animator>().animationTime;

        animationTime += factors.at(name) * deltaTime;
    }
};

void AnimatorSystem::release() {}

// ////////////////////////////////////////////////////////////////////////// //
