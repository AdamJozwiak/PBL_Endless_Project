#pragma once

#include "ECS/Event.hpp"
#include "ECS/Utilities.hpp"

ECS_EVENT(OnEntity) { EntityId entityId; };

ECS_EVENT_DERIVED(OnEntityCreate, OnEntity){};

ECS_EVENT_DERIVED(OnEntityDestroy, OnEntity){};
