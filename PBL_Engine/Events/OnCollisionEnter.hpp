#pragma once

#include "ECS/Entity.hpp"
#include "ECS/Event.hpp"

ECS_EVENT(OnCollisionEnter) { Entity a, b; };
