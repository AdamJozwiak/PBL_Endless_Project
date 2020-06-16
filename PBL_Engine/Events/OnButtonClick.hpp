#pragma once

#include "Button.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Event.hpp"

ECS_EVENT(OnButtonClick) { Button *button; };
