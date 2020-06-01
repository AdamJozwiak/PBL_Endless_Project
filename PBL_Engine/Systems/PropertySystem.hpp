#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(PropertySystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ----------------------------------------------- Public interface -- == //
    std::vector<Entity> findEntityByName(std::string const &name);
    std::vector<Entity> findEntityByTag(std::string const &tag);
};

// ////////////////////////////////////////////////////////////////////////// //
