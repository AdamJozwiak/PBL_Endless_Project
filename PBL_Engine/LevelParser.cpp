#include "LevelParser.h"

#include <Script.hpp>
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

using FileId = std::string;
using FileGuid = std::string;
using Path = std::string;
using FileExtension = std::string;

std::unordered_map<FileId, YAML::Node> nodes;
std::unordered_map<FileGuid, YAML::Node> materialNodes;
std::unordered_map<Path, FileGuid> pathToGuid;
std::unordered_map<FileGuid, Path> guidPaths;
std::unordered_map<FileGuid, std::set<FileId>> prefabFileIds;

auto &registry = Registry::instance();

#define yamlLoop(iterator, node)                         \
    for (YAML::const_iterator iterator = (node).begin(); \
         iterator != (node).end(); ++iterator)

LevelParser::LevelParser() {}

LevelParser::~LevelParser() {}

std::unordered_map<FileId, EntityId> spawnPrefab(
    std::set<FileId> const &fileIds,
    std::set<EntityId> *recursivePrefabIds = nullptr) {
    // Go through all game objects in scene/prefab file and create entities
    std::unordered_map<FileId, EntityId> entityIds;
    for (auto const &fileId : fileIds) {
        auto node{nodes[fileId]};

        if (auto const &nodeGameObject = node["GameObject"]; nodeGameObject) {
            auto entity{registry.createEntity()};
            entityIds.insert({fileId, entity.id});

            assert(nodeGameObject["m_Name"] && nodeGameObject["m_TagString"] &&
                   nodeGameObject["m_IsActive"] &&
                   "Every property inside GameObject component must be valid!");

            entity.add<Properties>(
                {.name = nodeGameObject["m_Name"].as<std::string>(),
                 .tag = nodeGameObject["m_TagString"].as<std::string>(),
                 .active = static_cast<bool>(
                     nodeGameObject["m_IsActive"].as<int>())});
            }
        }

    // Go through all other components in the scene/prefab file
    std::unordered_map<FileId, EntityId> entityIdsWithTransform;
    for (auto const &fileId : fileIds) {
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
                nodeMeshRenderer["m_Materials"] &&
                "Every property inside MeshRenderer component must be valid!");

            yamlLoop(i, nodeMeshRenderer["m_GameObject"]) {
                auto gameObjectFileId = i->second.Scalar();
                Entity(entityIds[gameObjectFileId]).add<Renderer>({});
                Entity(entityIds[gameObjectFileId]).add<AABB>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &renderer = Entity(entityIds[fileId]).get<Renderer>();

            yamlLoop(i, nodeMeshRenderer["m_Materials"][0]) {
                auto const key = i->first.as<std::string>();
                auto const value = i->second.as<std::string>();

                if (key == "guid") {
                    if (materialNodes.find(value) == materialNodes.end()) {
                        continue;
                    }
                    auto const &materialNode =
                        materialNodes[value]["Material"]["m_SavedProperties"]
                                     ["m_TexEnvs"];
                    auto const &propertiesNode =
                        materialNodes[value]["Material"]["m_SavedProperties"]
                                     ["m_Floats"];
                    renderer.material.albedoPath =
                        guidPaths[materialNode[5]["_MainTex"]["m_Texture"]
                                              ["guid"]
                                                  .as<std::string>()];
                    renderer.material.ambientOcclusionPath =
                        guidPaths[materialNode[7]["_OcclusionMap"]["m_Texture"]
                                              ["guid"]
                                                  .as<std::string>()];
                    renderer.material.metallicSmoothnessPath =
                        guidPaths[materialNode[6]["_MetallicGlossMap"]
                                              ["m_Texture"]["guid"]
                                                  .as<std::string>()];
                    renderer.material.normalPath =
                        guidPaths[materialNode[0]["_BumpMap"]["m_Texture"]
                                              ["guid"]
                                                  .as<std::string>()];
                    renderer.material.heightPath =
                        guidPaths[materialNode[8]["_ParallaxMap"]["m_Texture"]
                                              ["guid"]
                                                  .as<std::string>()];
                    renderer.material.parallaxHeight =
                        propertiesNode[10]["_Parallax"].as<float>();
                }
            }
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
                    meshFilter.path = guidPaths[value];
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

    /* std::set<FileId> usedFileIds; */
    for (auto const &fileId : fileIds) {
        auto const &node = nodes[fileId];

        if (auto const &nodePrefabInstance = node["PrefabInstance"];
            nodePrefabInstance) {
            assert(nodePrefabInstance["m_SourcePrefab"] &&
                   nodePrefabInstance["m_Modification"] &&
                   nodePrefabInstance["m_Modification"]["m_Modifications"] &&
                   "Every property inside PrefabInstance component must be "
                   "valid!");

            FileGuid prefabGuid =
                nodePrefabInstance["m_SourcePrefab"]["guid"].as<FileGuid>();

            auto prefabEntityIds =
                spawnPrefab(prefabFileIds.at(prefabGuid), recursivePrefabIds);

            FileId transformParentId =
                nodePrefabInstance["m_Modification"]["m_TransformParent"]
                                  ["fileID"]
                                      .as<FileId>();
            FileId targetTransformId =
                nodePrefabInstance["m_Modification"]["m_Modifications"][0]
                                  ["target"]["fileID"]
                                      .as<FileId>();

            if (transformParentId != "0") {
                Entity(prefabEntityIds[targetTransformId])
                    .get<Transform>()
                    .parent =
                    std::make_optional<EntityId>(entityIds[transformParentId]);
            }

            yamlLoop(i,
                     nodePrefabInstance["m_Modification"]["m_Modifications"]) {
                auto targetFileId = (*i)["target"]["fileID"].as<FileId>();
                auto property = (*i)["propertyPath"].as<std::string>();

                if (property == "m_RootOrder") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .root_Order = (*i)["value"].as<int>();
                } else if (property == "m_LocalEulerAnglesHint.x") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .euler.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalEulerAnglesHint.y") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .euler.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalEulerAnglesHint.z") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .euler.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.w") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .rotation.w = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.x") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .rotation.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.y") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .rotation.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.z") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .rotation.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.x") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .scale.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.y") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .scale.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.z") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .scale.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.x") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .position.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.y") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .position.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.z") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Transform>()
                        .position.z = (*i)["value"].as<float>();
                } else if (property == "m_Name") {
                    Entity(prefabEntityIds[targetFileId])
                        .get<Properties>()
                        .name = (*i)["value"].as<std::string>();
                }
            }
        }
    }

    // After creating Transform components, update parent references
    // TODO: Consider also adding child references, somehow
    for (auto const &[fileId, entityId] : entityIdsWithTransform) {
        auto const &node{nodes.at(fileId)};
        assert(node["Transform"] &&
               "This loop must operate on valid Transform components!");
        assert(node["Transform"]["m_Father"] &&
               node["Transform"]["m_Father"]["fileID"] &&
               "Transform components in game files must contain parent "
               "reference!");

        auto const &parentFileId =
            node["Transform"]["m_Father"]["fileID"].as<FileId>();
        if (parentFileId == "0") {
            continue;
        }
        if (!entityIdsWithTransform.contains(parentFileId)) {
            continue;
        }
        Entity{entityId}.get<Transform>().parent = std::make_optional<EntityId>(
            entityIdsWithTransform.at(parentFileId));
    }

    if (recursivePrefabIds) {
        for (auto const &[fileId, entityId] : entityIds) {
            recursivePrefabIds->insert(entityId);
        }
    }

    return entityIds;
}

void LevelParser::load() {
    auto const &sceneNodes = YAML::LoadAllFromFile(
        "Assets\\SceneFiles\\SampleScene\\SampleScene.unity");

    std::unordered_map<FileExtension, std::vector<Path>> assetPaths;

    // Creating vectors of files with .prefab, .mat, .meta extensions
    Path searchedDirectory{"Assets\\SceneFiles\\SampleScene"};
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
                } else if (extension == ".prefab") {
                    yamlLoop(i, node) {
                        if (!i->second["id"]) {
                            continue;
                        }

                        // Check for duplicates
                        assert(!nodes.contains(i->second["id"].as<FileId>()) &&
                               "Duplicate file identifiers!");

                        nodes.insert({i->second["id"].as<FileId>(), node});
                        prefabFileIds[pathToGuid[path]].insert(
                            i->second["id"].as<FileId>());
                    }
                } else if (extension == ".mat") {
                    // Check for duplicates
                    assert(materialNodes.find(pathToGuid[path]) ==
                               materialNodes.end() &&
                           "Duplicate file identifiers!");

                    materialNodes.insert({pathToGuid[path], node});
                }
            }
        }
    }

    // Spawn the scene and prefabs
    std::set<EntityId> recursivePrefabIds;
    spawnPrefab(sceneFileIds, &recursivePrefabIds);
    finalizeLoading(recursivePrefabIds);
}

Entity LevelParser::loadPrefab(std::string const &filename) {
    assert(pathToGuid.contains(filename) &&
           "There's no prefab with that name!");

    // Spawn the prefab
    std::set<EntityId> recursivePrefabIds;
    auto const &prefabGuid = pathToGuid.at(filename);
    auto prefabEntityIds =
        spawnPrefab(prefabFileIds.at(prefabGuid), &recursivePrefabIds);

    finalizeLoading(recursivePrefabIds);

    // Find and return the prefab's main entity
    for (auto const &[fileId, entityId] : prefabEntityIds) {
        if (Entity(entityId).get<Transform>().parent == std::nullopt) {
            return Entity(entityId);
        }
    }
}

void LevelParser::finalizeLoading(
    std::set<EntityId> const &recursivePrefabIds) {
    // Load further assets for the components
    for (auto const &entityId : recursivePrefabIds) {
        auto entity = Entity(entityId);

        // Models
        if (entity.has<Renderer>() && entity.has<MeshFilter>()) {
            auto &meshFilter = entity.get<MeshFilter>();
            auto &renderer = entity.get<Renderer>();

            meshFilter.model =
                Model::create(registry.system<WindowSystem>()->gfx(),
                              meshFilter.path, &renderer);
        }

        // Colliders
        if (entity.has<SphereCollider>()) {
            entity.get<SphereCollider>() =
                registry.system<ColliderSystem>()->AddSphereCollider(
                    entity.get<MeshFilter>().model->verticesForCollision);
        }

        // AABBs for the frustum culling
        if (entity.has<AABB>()) {
            entity.get<AABB>() = registry.system<ColliderSystem>()->AddAABB(
                entity.get<MeshFilter>().model->verticesForCollision);
        }

        // Scripts
        if (entity.has<Behaviour>()) {
            entity.get<Behaviour>().script->setup();
        }
    }
}
