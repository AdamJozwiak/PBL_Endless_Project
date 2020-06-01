// ///////////////////////////////////////////////////////////////// Includes //
#include "PropertySystem.hpp"

#include "Components/Properties.hpp"
#include "Components/Tags.hpp"
#include "ECS/ECS.hpp"

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void PropertySystem::filters() { filter<Properties>(); }

void PropertySystem::setup() {}

void PropertySystem::update(float const deltaTime){};

void PropertySystem::release() {}

// --------------------------------------------------- Public interface -- == //
std::vector<Entity> PropertySystem::findEntityByName(std::string const &name) {
    std::vector<Entity> foundEntities;

    for (auto const &entity : entities) {
        auto const &properties = entity.get<Properties>();

        if (properties.name == name) {
            foundEntities.push_back(entity);
        }
    }

    return foundEntities;
}

std::vector<Entity> PropertySystem::findEntityByTag(std::string const &tag) {
    std::vector<Entity> foundEntities;

    for (auto const &entity : entities) {
        auto const &properties = entity.get<Properties>();

        if (properties.tag == tag) {
            foundEntities.push_back(entity);
        }
    }

    return foundEntities;
}

void PropertySystem::activateEntity(Entity entity, bool const active) {
    auto &properties = entity.get<Properties>();

    properties.active = active;
    entity.remove<Active>();
    if (active) {
        entity.add<Active>({});
    }
}

// ////////////////////////////////////////////////////////////////////////// //
