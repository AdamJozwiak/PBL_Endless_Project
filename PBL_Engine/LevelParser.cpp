#include "LevelParser.h"

#include <cassert>
#include <set>
#include <unordered_map>
#include <vector>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "yaml-cpp/include/yaml-cpp/yaml.h"

auto &registry = Registry::instance();

#define yamlLoop(iterator, node)                         \
    for (YAML::const_iterator iterator = (node).begin(); \
         iterator != (node).end(); ++iterator)

LevelParser::LevelParser() {}

LevelParser::~LevelParser() {}

void LevelParser::load() {
    using FileId = std::string;

    auto const &sceneNodes = YAML::LoadAllFromFile("1.yaml");

    // Map all possible nodes with corresponding identifiers and make a set of
    // all identifiers in scene file
    std::unordered_map<FileId, YAML::Node> nodes;
    std::set<FileId> sceneFileIds;
    for (auto const &node : sceneNodes) {
        yamlLoop(i, node) {
            if (!i->second["id"]) {
                continue;
            }
            nodes.insert({i->second["id"].as<FileId>(), node});
            sceneFileIds.insert(i->second["id"].as<FileId>());
        }
    }

    // Go through all game objects in scene file and create entities
    std::unordered_map<FileId, EntityId> entities;
    for (auto const &fileId : sceneFileIds) {
        auto node{nodes[fileId]};

        if (node["GameObject"]) {
            auto entity{registry.createEntity()};
            entities.insert({fileId, entity.id});
        }
    }

    // Go through all other components in the scene file
    // For now, only process Transform
    // TODO: Expand this only when new components are needed
    std::unordered_map<FileId, EntityId> entityIdsWithTransform;
    for (auto const &fileId : sceneFileIds) {
        auto const &node = nodes[fileId];

        if (auto const &nodeTransform = node["Transform"]; nodeTransform) {
            assert(nodeTransform["m_GameObject"] &&
                   nodeTransform["m_LocalRotation"] &&
                   nodeTransform["m_LocalPosition"] &&
                   nodeTransform["m_LocalScale"] &&
                   nodeTransform["m_RootOrder"] &&
                   nodeTransform["m_LocalEulerAnglesHint"] &&
                   "Every property inside Transform component must be valid!");

            yamlLoop(i, nodeTransform["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entities[gameObjectFileId]).add<Transform>({});
                entityIdsWithTransform.insert(
                    {fileId, entities[gameObjectFileId]});
            }

            auto &transform =
                Entity(entityIdsWithTransform[fileId]).get<Transform>();

            yamlLoop(it, nodeTransform["m_LocalRotation"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.rotation_x = value;
                }
                if (key == "y") {
                    transform.rotation_y = value;
                }
                if (key == "z") {
                    transform.rotation_z = value;
                }
                if (key == "w") {
                    transform.rotation_w = value;
                }
            }

            yamlLoop(it, nodeTransform["m_LocalPosition"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.position_x = value;
                }
                if (key == "y") {
                    transform.position_y = value;
                }
                if (key == "z") {
                    transform.position_z = value;
                }
            }

            yamlLoop(it, nodeTransform["m_LocalScale"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.scale_x = value;
                }
                if (key == "y") {
                    transform.scale_y = value;
                }
                if (key == "z") {
                    transform.scale_z = value;
                }
            }

            transform.root_Order = nodeTransform["m_RootOrder"].as<int>();

            yamlLoop(it, nodeTransform["m_LocalEulerAnglesHint"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.eulerAngle_x = value;
                }
                if (key == "y") {
                    transform.eulerAngle_y = value;
                }
                if (key == "z") {
                    transform.eulerAngle_z = value;
                }
            }
        }
    }

    // After creating Transform components, update parent references
    // TODO: Consider also adding child references, somehow
    for (auto const &[fileId, entityId] : entityIdsWithTransform) {
        auto const &node{nodes[fileId]};
        assert(node["Transform"] &&
               "This loop must operate on valid Transform components!");
        assert(node["Transform"]["m_Father"] &&
               node["Transform"]["m_Father"]["fileID"] &&
               "Transform components in game files must contain parent "
               "reference!");

        auto const &parentFileId =
            node["Transform"]["m_Father"]["fileID"].Scalar();
        if (!entityIdsWithTransform.contains(parentFileId)) {
            continue;
        }
        Entity{entityId}.get<Transform>().parent =
            &Entity{entityIdsWithTransform[parentFileId]}.get<Transform>();
    }
}
