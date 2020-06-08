// ///////////////////////////////////////////////////////////////// Includes //
#include <Components/Components.hpp>
#include <ECS/Entity.hpp>

// ////////////////////////////////////////////////// Template instantiations //
// AABB
template Entity& Entity::add<AABB>(AABB const&);
template Entity& Entity::remove<AABB>();
template AABB& Entity::get<AABB>();
template AABB const& Entity::get<AABB>() const;
template bool Entity::has<AABB>() const;
template Entity& Entity::set<AABB>(AABB const&);

// Animator
template Entity& Entity::add<Animator>(Animator const&);
template Entity& Entity::remove<Animator>();
template Animator& Entity::get<Animator>();
template Animator const& Entity::get<Animator>() const;
template bool Entity::has<Animator>() const;
template Entity& Entity::set<Animator>(Animator const&);

// Behaviour
template Entity& Entity::add<Behaviour>(Behaviour const&);
template Entity& Entity::remove<Behaviour>();
template Behaviour& Entity::get<Behaviour>();
template Behaviour const& Entity::get<Behaviour>() const;
template bool Entity::has<Behaviour>() const;
template Entity& Entity::set<Behaviour>(Behaviour const&);

// BoxCollider
template Entity& Entity::add<BoxCollider>(BoxCollider const&);
template Entity& Entity::remove<BoxCollider>();
template BoxCollider& Entity::get<BoxCollider>();
template BoxCollider const& Entity::get<BoxCollider>() const;
template bool Entity::has<BoxCollider>() const;
template Entity& Entity::set<BoxCollider>(BoxCollider const&);

// MainCamera
template Entity& Entity::add<MainCamera>(MainCamera const&);
template Entity& Entity::remove<MainCamera>();
template MainCamera& Entity::get<MainCamera>();
template MainCamera const& Entity::get<MainCamera>() const;
template bool Entity::has<MainCamera>() const;
template Entity& Entity::set<MainCamera>(MainCamera const&);

// MeshFilter
template Entity& Entity::add<MeshFilter>(MeshFilter const&);
template Entity& Entity::remove<MeshFilter>();
template MeshFilter& Entity::get<MeshFilter>();
template MeshFilter const& Entity::get<MeshFilter>() const;
template bool Entity::has<MeshFilter>() const;
template Entity& Entity::set<MeshFilter>(MeshFilter const&);

// Properties
template Entity& Entity::add<Properties>(Properties const&);
template Entity& Entity::remove<Properties>();
template Properties& Entity::get<Properties>();
template Properties const& Entity::get<Properties>() const;
template bool Entity::has<Properties>() const;
template Entity& Entity::set<Properties>(Properties const&);

// Renderer
template Entity& Entity::add<Renderer>(Renderer const&);
template Entity& Entity::remove<Renderer>();
template Renderer& Entity::get<Renderer>();
template Renderer const& Entity::get<Renderer>() const;
template bool Entity::has<Renderer>() const;
template Entity& Entity::set<Renderer>(Renderer const&);

// Rigidbody
template Entity& Entity::add<Rigidbody>(Rigidbody const&);
template Entity& Entity::remove<Rigidbody>();
template Rigidbody& Entity::get<Rigidbody>();
template Rigidbody const& Entity::get<Rigidbody>() const;
template bool Entity::has<Rigidbody>() const;
template Entity& Entity::set<Rigidbody>(Rigidbody const&);

// SphereCollider
template Entity& Entity::add<SphereCollider>(SphereCollider const&);
template Entity& Entity::remove<SphereCollider>();
template SphereCollider& Entity::get<SphereCollider>();
template SphereCollider const& Entity::get<SphereCollider>() const;
template bool Entity::has<SphereCollider>() const;
template Entity& Entity::set<SphereCollider>(SphereCollider const&);

// Tags - Active
template Entity& Entity::add<Active>(Active const&);
template Entity& Entity::remove<Active>();
template Active& Entity::get<Active>();
template Active const& Entity::get<Active>() const;
template bool Entity::has<Active>() const;
template Entity& Entity::set<Active>(Active const&);

// Tags - Refractive
template Entity& Entity::add<Refractive>(Refractive const&);
template Entity& Entity::remove<Refractive>();
template Refractive& Entity::get<Refractive>();
template Refractive const& Entity::get<Refractive>() const;
template bool Entity::has<Refractive>() const;
template Entity& Entity::set<Refractive>(Refractive const&);

// Transform
template Entity& Entity::add<Transform>(Transform const&);
template Entity& Entity::remove<Transform>();
template Transform& Entity::get<Transform>();
template Transform const& Entity::get<Transform>() const;
template bool Entity::has<Transform>() const;
template Entity& Entity::set<Transform>(Transform const&);

// ////////////////////////////////////////////////////////////////////////// //
