#pragma once

#include "ECS/Event.hpp"
#include "ECS/Utilities.hpp"

template <typename ComponentType>
ECS_EVENT(OnComponent) {
    EntityId entityId;
};

template <typename ComponentType>
ECS_EVENT_DERIVED(OnComponentAdd, OnComponent<ComponentType>){};

template <typename ComponentType>
ECS_EVENT_DERIVED(OnComponentUpdate, OnComponent<ComponentType>){};

template <typename ComponentType>
ECS_EVENT_DERIVED(OnComponentRemove, OnComponent<ComponentType>){};
