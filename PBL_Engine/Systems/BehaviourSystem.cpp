// ///////////////////////////////////////////////////////////////// Includes //
#include "BehaviourSystem.hpp"

#include <Windows.h>

#include <cassert>

#include "Script.hpp"

// ECS
#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void BehaviourSystem::filters() { filter<Behaviour>(); }

void BehaviourSystem::setup() {}

void BehaviourSystem::update(float deltaTime) {
    for (auto const &entity : entities) {
        auto const &behaviour = entity.get<Behaviour>();

        behaviour.script->update(deltaTime);
    }
};
void BehaviourSystem::release() {}

Behaviour BehaviourSystem::behaviour(std::string name, Entity entity) {
    HINSTANCE dll = LoadLibrary((name + ".dll").c_str());
    assert(dll && "Behaviour's .dll script must be loaded correctly!");

    using ScriptCreationPtr =
        void(__cdecl *)(std::shared_ptr<Script> &, Entity);
    ScriptCreationPtr create =
        reinterpret_cast<ScriptCreationPtr>(GetProcAddress(dll, "create"));
    assert(create &&
           "Behaviour's .dll script creation function must be defined "
           "correctly!");

    Behaviour behaviour;
    create(behaviour.script, entity);
    return behaviour;
}

// ////////////////////////////////////////////////////////////////////////// //
