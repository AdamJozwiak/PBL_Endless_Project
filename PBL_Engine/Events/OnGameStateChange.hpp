#pragma once

#include "ECS/Event.hpp"
#include "game-states.hpp"

ECS_EVENT(OnGameStateChange) { GameState const nextState; };
