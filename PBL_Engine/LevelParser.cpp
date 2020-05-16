#include "LevelParser.h"

#include <cassert>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <vector>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Mesh.h"
#include "Systems/Systems.hpp"
#include "Window.h"
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
    using FileGuid = std::string;
    using Path = std::string;
    using FileExtension = std::string;

    auto const &sceneNodes = YAML::LoadAllFromFile(
        "Assets/SceneFiles/SampleScene/SampleScene.unity");

    std::unordered_map<FileGuid, std::set<FileId>> prefabFileIds;

    std::unordered_map<FileExtension, std::vector<Path>> assetPaths;
    std::unordered_map<FileGuid, Path> guidPaths;
    std::unordered_map<Path, FileGuid> pathToGuid;

    // Creating vectors of files with .prefab, .mat, .meta extensions
    Path searchedDirectory{"Assets/SceneFiles/SampleScene"};
    std::vector<FileExtension> searchedFileExtensions{".meta", ".prefab",
                                                      ".mat"};

    for (auto const &entry :
         fs::recursive_directory_iterator(searchedDirectory)) {
        Path const &path = entry.path().string();
        FileExtension const &extension = entry.path().extension().string();

        if (std::find(searchedFileExtensions.begin(),
                      searchedFileExtensions.end(),
                      extension) != searchedFileExtensions.end()) {
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

    for (auto const &extension : searchedFileExtensions) {
        for (auto const &path : assetPaths[extension]) {
            std::vector<YAML::Node> assetNodes = YAML::LoadAllFromFile(path);
            for (auto const &node : assetNodes) {
                if (extension == ".meta") {
                    auto const pathWithoutExtension{
                        fs::path(path).parent_path().string() + "\\" +
                        fs::path(path).stem().string()};
                    guidPaths.insert(
                        {node["guid"].as<FileGuid>(), pathWithoutExtension});
                    pathToGuid.insert(
                        {pathWithoutExtension, node["guid"].as<FileGuid>()});
                    break;
                }
                yamlLoop(i, node) {
                    if (!i->second["id"]) {
                        continue;
                    }

                    // Check for duplicates
                    assert(nodes.find(i->second["id"].as<FileId>()) ==
                               nodes.end() &&
                           "Duplicate file identifiers!");

                    nodes.insert({i->second["id"].as<FileId>(), node});

                    if (extension == ".prefab") {
                        prefabFileIds[pathToGuid[path]].insert(
                            i->second["id"].as<FileId>());
                    }
                }
            }
        }
    }

    // Go through all prefabs in the scene file and save prefab GUIDs that
    // belong to the parsed scene
    auto findPrefabGuidsBelongingToScene =
        [&nodes, &prefabFileIds](
            std::set<FileId> const &fileIds) -> std::vector<FileGuid> {
        auto localFunction =
            [&nodes, &prefabFileIds](
                std::set<FileId> const &fileIds,
                auto const &localFunction) -> std::vector<FileGuid> {
            std::vector<FileGuid> guids;
            for (auto const &fileId : fileIds) {
                auto node{nodes[fileId]};

                if (auto const &nodePrefabInstance = node["PrefabInstance"];
                    nodePrefabInstance) {
                    assert(nodePrefabInstance["m_SourcePrefab"] &&
                           "Every property inside PrefabInstance component "
                           "must be "
                           "valid!");

                    yamlLoop(it, nodePrefabInstance["m_SourcePrefab"]) {
                        auto const key = it->first.as<std::string>();
                        auto const value = it->second.as<std::string>();

                        if (key == "guid") {
                            guids.push_back(value);
                        }
                    }
                }
            }
            for (auto const &guid : guids) {
                auto deeperGuids =
                    localFunction(prefabFileIds[guid], localFunction);
                guids.insert(guids.begin(), deeperGuids.begin(),
                             deeperGuids.end());
            }
            return guids;
        };

        return localFunction(fileIds, localFunction);
    };
    std::vector<FileGuid> scenePrefabGuids =
        findPrefabGuidsBelongingToScene(sceneFileIds);

    for (auto const &guid : scenePrefabGuids) {
        // Check for duplicates
        for (auto const &fileId : prefabFileIds[guid]) {
            assert(sceneFileIds.find(fileId) == sceneFileIds.end() &&
                   "Duplicate file identifiers!");
        }

        sceneFileIds.insert(prefabFileIds[guid].begin(),
                            prefabFileIds[guid].end());
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
            assert(nodeTransform["m_PrefabInstance"] &&
                   "Every property inside Transform component must be valid!");

            bool skipThisTransform = false;
            yamlLoop(i, nodeTransform["m_PrefabInstance"]) {
                auto prefabFileId = i->second.Scalar();
                skipThisTransform = prefabFileId != "0";
            }
            if (skipThisTransform) {
                continue;
            }

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
                    transform.rotation.x = value;
                }
                if (key == "y") {
                    transform.rotation.y = value;
                }
                if (key == "z") {
                    transform.rotation.z = value;
                }
                if (key == "w") {
                    transform.rotation.w = value;
                }
            }

            yamlLoop(it, nodeTransform["m_LocalPosition"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.position.x = value;
                }
                if (key == "y") {
                    transform.position.y = value;
                }
                if (key == "z") {
                    transform.position.z = value;
                }
            }

            yamlLoop(it, nodeTransform["m_LocalScale"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.scale.x = value;
                }
                if (key == "y") {
                    transform.scale.y = value;
                }
                if (key == "z") {
                    transform.scale.z = value;
                }
            }

            transform.root_Order = nodeTransform["m_RootOrder"].as<int>();

            yamlLoop(it, nodeTransform["m_LocalEulerAnglesHint"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    transform.euler.x = value;
                }
                if (key == "y") {
                    transform.euler.y = value;
                }
                if (key == "z") {
                    transform.euler.z = value;
                }
            }
        }

        if (auto const &nodeMonoBehaviour = node["MonoBehaviour"];
            nodeMonoBehaviour) {
            assert(
                nodeMonoBehaviour["m_GameObject"] &&
                nodeMonoBehaviour["m_Script"] &&
                "Every property inside MonoBehaviour component must be valid!");

            yamlLoop(i, nodeMonoBehaviour["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<Behaviour>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &behaviour = Entity(entityIds[fileId]).get<Behaviour>();

            yamlLoop(it, nodeMonoBehaviour["m_Script"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<std::string>();

                if (key == "guid") {
                    behaviour = registry.system<BehaviourSystem>()->behaviour(
                        fs::path(guidPaths[value]).stem().string(),
                        Entity(entityIds[fileId]));
                }
            }
        }

        if (auto const &nodeMeshRenderer = node["MeshRenderer"];
            nodeMeshRenderer) {
            assert(
                nodeMeshRenderer["m_GameObject"] &&
                "Every property inside MeshRenderer component must be valid!");

            yamlLoop(i, nodeMeshRenderer["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<Renderer>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &renderer = Entity(entityIds[fileId]).get<Renderer>();
        }

        if (auto const &nodeMeshFilter = node["MeshFilter"]; nodeMeshFilter) {
            assert(nodeMeshFilter["m_GameObject"] && nodeMeshFilter["m_Mesh"] &&
                   "Every property inside MeshFilter component must be valid!");

            yamlLoop(i, nodeMeshFilter["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<MeshFilter>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &meshFilter = Entity(entityIds[fileId]).get<MeshFilter>();

            yamlLoop(it, nodeMeshFilter["m_Mesh"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<std::string>();

                if (key == "guid") {
                    meshFilter.model = std::make_shared<Model>(
                        registry.system<RenderSystem>()->window->Gfx(),
                        guidPaths[value]);
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
                    boxCollider.size.x = value;
                }
                if (key == "y") {
                    boxCollider.size.y = value;
                }
                if (key == "z") {
                    boxCollider.size.z = value;
                }
            }

            yamlLoop(it, nodeBoxCollider["m_Center"]) {
                auto const key = it->first.as<std::string>();
                auto const value = it->second.as<float>();

                if (key == "x") {
                    boxCollider.center.x = value;
                }
                if (key == "y") {
                    boxCollider.center.y = value;
                }
                if (key == "z") {
                    boxCollider.center.z = value;
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
                    sphereCollider.center.x = value;
                }
                if (key == "y") {
                    sphereCollider.center.y = value;
                }
                if (key == "z") {
                    sphereCollider.center.z = value;
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
