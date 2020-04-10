#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <bitset>

// ///////////////////////////////////////////////////// Usings and constants //
using EntityId = unsigned int;
constexpr EntityId MAX_ENTITIES = 256u;

using ComponentId = unsigned int;
constexpr ComponentId MAX_COMPONENTS = 32u;

using Signature = std::bitset<MAX_COMPONENTS>;

// ////////////////////////////////////////////////////////////////////////// //

