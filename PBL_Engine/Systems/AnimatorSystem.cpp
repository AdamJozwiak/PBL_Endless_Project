// ///////////////////////////////////////////////////////////////// Includes //
#include "AnimatorSystem.hpp"

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void AnimatorSystem::filters() { filter<Active>().filter<Animator>(); }

void AnimatorSystem::setup() {}

void AnimatorSystem::update(float deltaTime) {
    // Advance the animation times
    for (Entity entity : entities) {
        auto& animator = entity.get<Animator>();
        animator.animationTime += animator.factor * deltaTime;
    }
};

void AnimatorSystem::release() {}

// ////////////////////////////////////////////////////////////////////////// //
