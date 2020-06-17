#pragma once

#include "Button.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Event.hpp"

ECS_EVENT(OnButtonHover) {
    Button *button;
    bool on;
};
