#include "LevelParser.h"

#include <cassert>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <vector>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "yaml-cpp/include/yaml-cpp/yaml.h"

namespace fs = std::filesystem;

auto &registry = Registry::instance();

#define yamlLoop(iterator, node)                         \
    for (YAML::const_iterator iterator = (node).begin(); \
         iterator != (node).end(); ++iterator)

LevelParser::LevelParser() {}

LevelParser::~LevelParser() {}

void LevelParser::load() {
    using FileId = std::string;
    using Path = std::string;
    using FileExtension = std::string;

    auto const &sceneNodes = YAML::LoadAllFromFile("SceneFiles/1.yaml");

    std::unordered_map<FileExtension, std::vector<Path>> assetPaths;

    // Creating vectors of files with .prefab, .mat, .meta extensions
    Path searchedDirectory{"."};
    std::set<FileExtension> searchedFileExtensions{".prefab", ".mat", ".meta"};

    for (auto const &entry :
         fs::recursive_directory_iterator(searchedDirectory)) {
        Path const &path = entry.path().string();
        FileExtension const &extension = entry.path().extension().string();

        if (searchedFileExtensions.find(extension) !=
            searchedFileExtensions.end()) {
            assetPaths[extension].emplace_back(path);
        }
    }

    // Map all possible nodes with corresponding identifiers and make a set
    // of all identifiers in scene file
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

    for (auto const &[extension, paths] : assetPaths) {
        for (auto const &path : paths) {
            std::vector<YAML::Node> assetNodes = YAML::LoadAllFromFile(path);
            for (auto const &node : assetNodes) {
                yamlLoop(i, node) {
                    if (!i->second["id"]) {
                        continue;
                    }
                    nodes.insert({i->second["id"].as<FileId>(), node});
                }
            }
        }
    }

    // Go through all game objects in scene file and create entities
    std::unordered_map<FileId, EntityId> entityIds;
    for (auto const &fileId : sceneFileIds) {
        auto node{nodes[fileId]};

        if (node["GameObject"]) {
            auto entity{registry.createEntity()};
            entityIds.insert({fileId, entity.id});
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
                Entity(entityIds[gameObjectFileId]).add<Transform>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
                entityIdsWithTransform.insert(
                    {fileId, entityIds[gameObjectFileId]});
            }

            auto &transform = Entity(entityIds[fileId]).get<Transform>();

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

        if (auto const &nodeBoxCollider = node["BoxCollider"];
            nodeBoxCollider) {
            assert(
                nodeBoxCollider["m_Size"] && nodeBoxCollider["m_Center"] &&
                "Every property inside BoxCollider component must be valid!");

            yamlLoop(i, nodeBoxCollider["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<BoxCollider>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &boxCollider = Entity(entityIds[fileId]).get<BoxCollider>();

            yamlLoop(it, nodeBoxCollider["m_Size"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    boxCollider.size_x = value;
                }
                if (key == "y") {
                    boxCollider.size_y = value;
                }
                if (key == "z") {
                    boxCollider.size_z = value;
                }
            }

            yamlLoop(it, nodeBoxCollider["m_Center"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    boxCollider.center_x = value;
                }
                if (key == "y") {
                    boxCollider.center_y = value;
                }
                if (key == "z") {
                    boxCollider.center_z = value;
                }
            }
        }

        if (auto const &nodeSphereCollider = node["SphereCollider"];
            nodeSphereCollider) {
            assert(nodeSphereCollider["m_Radius"] &&
                   nodeSphereCollider["m_Center"] &&
                   "Every property inside SphereCollider component must be "
                   "valid!");

            yamlLoop(i, nodeSphereCollider["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<SphereCollider>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &sphereCollider =
                Entity(entityIds[fileId]).get<SphereCollider>();

            sphereCollider.radius = nodeSphereCollider["m_Radius"].as<float>();

            yamlLoop(it, nodeSphereCollider["m_Center"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    sphereCollider.center_x = value;
                }
                if (key == "y") {
                    sphereCollider.center_y = value;
                }
                if (key == "z") {
                    sphereCollider.center_z = value;
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
