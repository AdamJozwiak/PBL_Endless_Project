#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <bitset>

// ///////////////////////////////////////////////////// Usings and constants //
using EntityId = unsigned int;
constexpr EntityId MAX_ENTITIES = 16384u;
constexpr EntityId EMPTY_ENTITY = MAX_ENTITIES + 1u;

using ComponentId = unsigned int;
constexpr ComponentId MAX_COMPONENTS = 32u;
constexpr ComponentId EMPTY_COMPONENT = MAX_COMPONENTS + 1u;

using Signature = std::bitset<MAX_COMPONENTS>;

// ////////////////////////////////////////////////////////////////////////// //

