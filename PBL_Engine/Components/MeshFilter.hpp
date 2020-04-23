#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ECS/Component.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class Model;

// //////////////////////////////////////////////////////////////// Component //
ECS_COMPONENT(MeshFilter) { std::shared_ptr<Model> model; };

// ////////////////////////////////////////////////////////////////////////// //