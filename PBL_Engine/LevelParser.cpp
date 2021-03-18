#include "LevelParser.h"

#include <yaml-cpp/include/yaml-cpp/yaml.h>

#include <Script.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <unordered_map>
#include <vector>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"
#include "Mesh.h"
#include "Systems/Systems.hpp"
#include "Window.h"
#include "assert.hpp"

namespace fs = std::filesystem;

using FileId = int;
using FileGuid = std::string;
using Path = std::string;
using FileExtension = std::string;

std::unordered_map<FileGuid, std::unordered_map<FileId, nlohmann::json>> nodes;
std::unordered_map<FileGuid, YAML::Node> metaNodes;
std::unordered_map<FileGuid, nlohmann::json> materialNodes;
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
    FileGuid guid, std::set<FileId> const &fileIds, bool cache,
    std::set<EntityId> *recursivePrefabIds = nullptr) {
    // Go through all game objects in scene/prefab file and create entities
    std::unordered_map<FileId, EntityId> entityIds;
    for (auto const &fileId : fileIds) {
        auto node{nodes.at(guid)[fileId]};

        if (node.contains("GameObject")) {
            auto const &nodeGameObject = node.at("GameObject");

            auto entity{
                registry.createEntity(cache ? CACHE_SCENE : DEFAULT_SCENE)};
            entityIds.insert({fileId, entity});

            assert(nodeGameObject.contains("m_Name") &&
                   nodeGameObject.contains("m_TagString") &&
                   nodeGameObject.contains("m_IsActive") &&
                   "Every property inside GameObject component must be valid!");

            entity.add<Properties>(
                {.name = nodeGameObject.at("m_Name").get<std::string>(),
                 .tag = nodeGameObject.at("m_TagString").get<std::string>(),
                 .active = static_cast<bool>(
                     nodeGameObject.at("m_IsActive").get<int>())});
        }
    }

    // Go through all other components in the scene/prefab file
    std::unordered_map<FileId, EntityId> entityIdsWithTransform;
    for (auto const &fileId : fileIds) {
        auto const &node = nodes.at(guid)[fileId];

        if (node.contains("Transform")) {
            auto const &nodeTransform = node.at("Transform");

            assert(nodeTransform.contains("m_PrefabInstance") &&
                   nodeTransform.at("m_PrefabInstance").contains("fileID") &&
                   "Every property inside Transform component must be valid!");

            bool skipThisTransform = false;
            {
                auto &helper = nodeTransform.at("m_PrefabInstance");

                auto prefabFileId = helper.at("fileID").get<int>();
                skipThisTransform = prefabFileId != 0;
            }
            if (skipThisTransform) {
                continue;
            }

            assert(nodeTransform.contains("m_GameObject") &&
                   nodeTransform.at("m_GameObject").contains("fileID") &&
                   nodeTransform.contains("m_LocalRotation") &&
                   nodeTransform.at("m_LocalRotation").contains("x") &&
                   nodeTransform.at("m_LocalRotation").contains("y") &&
                   nodeTransform.at("m_LocalRotation").contains("z") &&
                   nodeTransform.at("m_LocalRotation").contains("w") &&
                   nodeTransform.contains("m_LocalPosition") &&
                   nodeTransform.at("m_LocalPosition").contains("x") &&
                   nodeTransform.at("m_LocalPosition").contains("y") &&
                   nodeTransform.at("m_LocalPosition").contains("z") &&
                   nodeTransform.contains("m_LocalScale") &&
                   nodeTransform.at("m_LocalScale").contains("x") &&
                   nodeTransform.at("m_LocalScale").contains("y") &&
                   nodeTransform.at("m_LocalScale").contains("z") &&
                   nodeTransform.contains("m_RootOrder") &&
                   nodeTransform.contains("m_LocalEulerAnglesHint") &&
                   nodeTransform.at("m_LocalEulerAnglesHint").contains("x") &&
                   nodeTransform.at("m_LocalEulerAnglesHint").contains("y") &&
                   nodeTransform.at("m_LocalEulerAnglesHint").contains("z") &&
                   "Every property inside Transform component must be valid!");

            {
                auto &helper = nodeTransform.at("m_GameObject");
                auto const &gameObjectFileId = helper.at("fileID");

                auto entity = Entity(entityIds.at(gameObjectFileId));
                entity.add<Transform>({});

                entityIds.insert({fileId, entity});
                entityIdsWithTransform.insert({fileId, entity});
            }

            auto &transform = Entity(entityIds.at(fileId)).get<Transform>();

            {
                auto &helper = nodeTransform.at("m_LocalRotation");
                transform.rotation.x = helper.at("x").get<float>();
                transform.rotation.y = helper.at("y").get<float>();
                transform.rotation.z = helper.at("z").get<float>();
                transform.rotation.w = helper.at("w").get<float>();
            }

            {
                auto &helper = nodeTransform.at("m_LocalPosition");
                transform.position.x = helper.at("x").get<float>();
                transform.position.y = helper.at("y").get<float>();
                transform.position.z = helper.at("z").get<float>();
            }

            {
                auto &helper = nodeTransform.at("m_LocalScale");
                transform.scale.x = helper.at("x").get<float>();
                transform.scale.y = helper.at("y").get<float>();
                transform.scale.z = helper.at("z").get<float>();
            }

            {
                auto &helper = nodeTransform.at("m_RootOrder");
                transform.root_Order = helper.get<int>();
            }

            {
                auto &helper = nodeTransform.at("m_LocalEulerAnglesHint");
                transform.euler.x = helper.at("x").get<float>();
                transform.euler.y = helper.at("y").get<float>();
                transform.euler.z = helper.at("z").get<float>();
            }
        } else if (node.contains("RectTransform")) {
            auto const &nodeRectTransform = node.at("RectTransform");

            assert(
                nodeRectTransform.contains("m_GameObject") &&
                nodeRectTransform.at("m_GameObject").contains("fileID") &&
                nodeRectTransform.contains("m_AnchoredPosition") &&
                nodeRectTransform.at("m_AnchoredPosition").contains("x") &&
                nodeRectTransform.at("m_AnchoredPosition").contains("y") &&
                nodeRectTransform.contains("m_SizeDelta") &&
                nodeRectTransform.at("m_SizeDelta").contains("x") &&
                nodeRectTransform.at("m_SizeDelta").contains("y") &&
                "Every property inside RectTransform component must be valid!");

            {
                auto &helper = nodeRectTransform.at("m_GameObject");
                auto const &gameObjectFileId = helper.at("fileID");

                auto entity = Entity(entityIds.at(gameObjectFileId));
                entity.add<RectTransform>({});

                entityIds.insert({fileId, entity});
            }

            auto &rectTransform =
                Entity(entityIds.at(fileId)).get<RectTransform>();

            {
                auto &helper = nodeRectTransform.at("m_AnchoredPosition");
                rectTransform.position.x = helper.at("x").get<float>();
                rectTransform.position.y = -helper.at("y").get<float>();
            }

            {
                auto &helper = nodeRectTransform.at("m_SizeDelta");
                rectTransform.size.x = helper.at("x").get<float>();
                rectTransform.size.y = helper.at("y").get<float>();
            }
        } else if (node.contains("MonoBehaviour")) {
            auto const &nodeMonoBehaviour = node.at("MonoBehaviour");

            assert(
                nodeMonoBehaviour.contains("m_GameObject") &&
                nodeMonoBehaviour.at("m_GameObject").contains("fileID") &&
                nodeMonoBehaviour.contains("m_Script") &&
                nodeMonoBehaviour.at("m_Script").contains("guid") &&
                "Every property inside MonoBehaviour component must be valid!");

            {
                auto &helper = nodeMonoBehaviour.at("m_GameObject");
                auto const &gameObjectFileId = helper.at("fileID");

                auto entity = Entity(entityIds.at(gameObjectFileId));

                if (nodeMonoBehaviour.contains("m_Text")) {
                    entity.add<UIElement>({.alpha = 1.0f});
                } else if (guidPaths.contains(nodeMonoBehaviour.at("m_Script")
                                                  .at("guid")
                                                  .get<std::string>())) {
                    entity.add<Behaviour>({});
                }
                entityIds.insert({fileId, entityIds.at(gameObjectFileId)});
            }

            if (Entity(entityIds.at(fileId)).has<Behaviour>()) {
                auto &behaviour = Entity(entityIds[fileId]).get<Behaviour>();

                {
                    auto &helper = nodeMonoBehaviour["m_Script"];
                    behaviour = registry.system<BehaviourSystem>()->behaviour(
                        fs::path(guidPaths[helper["guid"].get<std::string>()])
                            .stem()
                            .string(),
                        Entity(entityIds[fileId]));
                }
            } else if (Entity(entityIds[fileId]).has<UIElement>()) {
                auto &uiElement = Entity(entityIds[fileId]).get<UIElement>();

                {
                    auto &helper = nodeMonoBehaviour["m_Text"];
                    if (helper.is_string()) {
                        uiElement.content = helper.get<std::string>();
                    }
                    if (helper.is_number()) {
                        uiElement.content = std::to_string(helper.get<int>());
                    }
                }
                {
                    auto &helper = nodeMonoBehaviour["m_FontData"];
                    uiElement.fontSize = helper["m_FontSize"].get<int>();
                }
            }
        } else if (node.contains("MeshRenderer")) {
            auto const &nodeMeshRenderer = node.at("MeshRenderer");

            assert(
                nodeMeshRenderer.contains("m_GameObject") &&
                nodeMeshRenderer.at("m_GameObject").contains("fileID") &&
                nodeMeshRenderer.contains("m_Materials") &&
                "Every property inside MeshRenderer component must be valid!");

            {
                auto &helper = nodeMeshRenderer.at("m_GameObject");
                auto const &gameObjectFileId = helper.at("fileID");

                auto entity = Entity(entityIds[gameObjectFileId]);

                entity.add<Renderer>({});
                entity.add<AABB>({});

                entityIds.insert({fileId, entity});
            }

            auto &renderer = Entity(entityIds[fileId]).get<Renderer>();

            {
                auto &helper = nodeMeshRenderer["m_Materials"];
                auto const value = helper.at(0).at("guid").get<std::string>();

                if (materialNodes.find(value) == materialNodes.end()) {
                    continue;
                }
                auto const &materialNode = materialNodes.at(value)
                                               .at("Material")
                                               .at("m_SavedProperties")
                                               .at("m_TexEnvs");
                auto const &propertiesNode = materialNodes.at(value)
                                                 .at("Material")
                                                 .at("m_SavedProperties")
                                                 .at("m_Floats");
                for (auto const &i : materialNode) {
                    if (i.contains("_MainTex")) {
                        renderer.material.albedoPath =
                            guidPaths[i.at("_MainTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_OcclusionMap")) {
                        renderer.material.ambientOcclusionPath =
                            guidPaths[i.at("_OcclusionMap")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_MetallicGlossMap")) {
                        renderer.material.metallicSmoothnessPath =
                            guidPaths[i.at("_MetallicGlossMap")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_BumpMap")) {
                        renderer.material.normalPath =
                            guidPaths[i.at("_BumpMap")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_ParallaxMap")) {
                        renderer.material.heightPath =
                            guidPaths[i.at("_ParallaxMap")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                }
                for (auto const &i : propertiesNode) {
                    if (i.contains("_Parallax")) {
                        renderer.material.parallaxHeight =
                            i.at("_Parallax").get<float>();
                    }
                }
            }
        } else if (node.contains("MeshFilter")) {
            auto const &nodeMeshFilter = node["MeshFilter"];

            assert(nodeMeshFilter.contains("m_GameObject") &&
                   nodeMeshFilter.at("m_GameObject").contains("fileID") &&
                   nodeMeshFilter.contains("m_Mesh") &&
                   nodeMeshFilter.at("m_Mesh").contains("guid") &&
                   "Every property inside MeshFilter component must be valid!");

            {
                auto &helper = nodeMeshFilter["m_GameObject"];
                auto const &gameObjectFileId = helper.at("fileID");
                Entity(entityIds[gameObjectFileId]).add<MeshFilter>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &meshFilter = Entity(entityIds[fileId]).get<MeshFilter>();

            {
                auto &helper = nodeMeshFilter["m_Mesh"];
                auto const value = helper.at("guid");

                meshFilter.path = guidPaths[value];
            }
        } else if (node.contains("BoxCollider")) {
            auto const &nodeBoxCollider = node["BoxCollider"];

            assert(
                nodeBoxCollider.contains("m_Size") &&
                nodeBoxCollider.at("m_Size").contains("x") &&
                nodeBoxCollider.at("m_Size").contains("y") &&
                nodeBoxCollider.at("m_Size").contains("z") &&
                nodeBoxCollider.contains("m_Center") &&
                nodeBoxCollider.at("m_Center").contains("x") &&
                nodeBoxCollider.at("m_Center").contains("y") &&
                nodeBoxCollider.at("m_Center").contains("z") &&
                "Every property inside BoxCollider component must be valid!");

            {
                auto &helper = nodeBoxCollider["m_GameObject"];
                auto const &gameObjectFileId = helper.at("fileID");
                Entity(entityIds[gameObjectFileId]).add<BoxCollider>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &boxCollider = Entity(entityIds[fileId]).get<BoxCollider>();

            {
                auto &helper = nodeBoxCollider["m_Size"];
                boxCollider.size.x = helper["x"].get<float>();
                boxCollider.size.y = helper["y"].get<float>();
                boxCollider.size.z = helper["z"].get<float>();
            }

            {
                auto &helper = nodeBoxCollider["m_Center"];
                boxCollider.center.x = helper["x"].get<float>();
                boxCollider.center.y = helper["y"].get<float>();
                boxCollider.center.z = helper["z"].get<float>();
            }
        } else if (node.contains("SphereCollider")) {
            auto const &nodeSphereCollider = node["SphereCollider"];

            assert(nodeSphereCollider.contains("m_Radius") &&
                   nodeSphereCollider.contains("m_Center") &&
                   nodeSphereCollider.at("m_Center").contains("x") &&
                   nodeSphereCollider.at("m_Center").contains("y") &&
                   nodeSphereCollider.at("m_Center").contains("z") &&
                   "Every property inside SphereCollider component must be "
                   "valid!");

            {
                auto &helper = nodeSphereCollider["m_GameObject"];
                auto const &gameObjectFileId = helper.at("fileID");
                Entity(entityIds[gameObjectFileId]).add<SphereCollider>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &sphereCollider =
                Entity(entityIds[fileId]).get<SphereCollider>();

            {
                auto &helper = nodeSphereCollider["m_Radius"];
                sphereCollider.radius = helper.get<float>();
            }

            {
                auto &helper = nodeSphereCollider["m_Center"];
                sphereCollider.center.x = helper["x"].get<float>();
                sphereCollider.center.y = helper["y"].get<float>();
                sphereCollider.center.z = helper["z"].get<float>();
            }
        } else if (node.contains("Rigidbody")) {
            auto const &nodeRigidbody = node["Rigidbody"];

            assert(nodeRigidbody.contains("m_Mass") &&
                   "Every property inside Rigidbody component must be "
                   "valid!");

            {
                auto &helper = nodeRigidbody["m_GameObject"];
                auto const &gameObjectFileId = helper.at("fileID");
                Entity(entityIds[gameObjectFileId]).add<Rigidbody>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &rigidbody = Entity(entityIds[fileId]).get<Rigidbody>();

            {
                auto &helper = nodeRigidbody["m_Mass"];
                rigidbody.mass = helper.get<float>();
            }
        } else if (node.contains("Skybox")) {
            auto const &nodeSkybox = node["Skybox"];

            assert(
                nodeSkybox.contains("m_GameObject") &&
                nodeSkybox.contains("m_CustomSkybox") &&
                "Every property inside MeshRenderer component must be valid!");

            {
                auto &helper = nodeSkybox["m_GameObject"];
                auto const &gameObjectFileId = helper.at("fileID");
                Entity(entityIds[gameObjectFileId]).add<Skybox>({});
                // cachedSkyboxes.insert({entityIds[gameObjectFileId],
                // {}});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
            }

            auto &skybox = Entity(entityIds[fileId]).get<Skybox>();
            // auto &renderer = cachedRenderers.at(entityIds[fileId]);

            {
                auto &helper = nodeSkybox["m_CustomSkybox"];
                auto const value = helper.at("guid");

                if (materialNodes.find(value) == materialNodes.end()) {
                    continue;
                }
                auto const &materialNode =
                    materialNodes[value]["Material"]["m_SavedProperties"]
                                 ["m_TexEnvs"];
                for (auto const &i : materialNode) {
                    if (i.contains("_BackTex")) {
                        skybox.material.backPath =
                            guidPaths[i.at("_BackTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_DownTex")) {
                        skybox.material.bottomPath =
                            guidPaths[i.at("_DownTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_FrontTex")) {
                        skybox.material.frontPath =
                            guidPaths[i.at("_FrontTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_LeftTex")) {
                        skybox.material.leftPath =
                            guidPaths[i.at("_LeftTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_RightTex")) {
                        skybox.material.rightPath =
                            guidPaths[i.at("_RightTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                    if (i.contains("_UpTex")) {
                        skybox.material.topPath =
                            guidPaths[i.at("_UpTex")
                                          .at("m_Texture")
                                          .at("guid")
                                          .get<std::string>()];
                    }
                }
            }
        }
    }

    /* std::set<FileId> usedFileIds; */
    for (auto const &fileId : fileIds) {
        auto const &node = nodes.at(guid)[fileId];

        if (node.contains("PrefabInstance")) {
            auto const &nodePrefabInstance = node["PrefabInstance"];

            assert(nodePrefabInstance.contains("m_SourcePrefab") &&
                   nodePrefabInstance.contains("m_Modification") &&
                   nodePrefabInstance.at("m_Modification")
                       .contains("m_Modifications") &&
                   "Every property inside PrefabInstance component must be "
                   "valid!");

            FileGuid prefabGuid =
                nodePrefabInstance["m_SourcePrefab"]["guid"].get<FileGuid>();

            auto prefabEntityIds =
                spawnPrefab(prefabGuid, prefabFileIds.at(prefabGuid), cache,
                            recursivePrefabIds);

            FileId transformParentId =
                nodePrefabInstance["m_Modification"]["m_TransformParent"]
                                  ["fileID"]
                                      .get<FileId>();
            FileId targetTransformId =
                nodePrefabInstance["m_Modification"]["m_Modifications"][0]
                                  ["target"]["fileID"]
                                      .get<FileId>();

            if (transformParentId != 0) {
                Entity(prefabEntityIds[targetTransformId])
                    .get<Transform>()
                    .parent =
                    std::make_optional<EntityId>(entityIds[transformParentId]);
            }

            for (auto const &i : nodePrefabInstance.at("m_Modification")
                                     .at("m_Modifications")) {
                auto targetFileId = i.at("target").at("fileID").get<FileId>();
                auto property = i.at("propertyPath").get<std::string>();

                auto &transform =
                    Entity(prefabEntityIds.at(targetFileId)).get<Transform>();
                auto &properties =
                    Entity(prefabEntityIds.at(targetFileId)).get<Properties>();

                std::map<std::string, std::function<void()>> actions;
                actions["m_RootOrder"] = [&]() {
                    transform.root_Order = i.at("value").get<int>();
                };
                actions["m_LocalEulerAnglesHint.x"] = [&]() {
                    transform.euler.x = i.at("value").get<float>();
                };
                actions["m_LocalEulerAnglesHint.y"] = [&]() {
                    transform.euler.y = i.at("value").get<float>();
                };
                actions["m_LocalEulerAnglesHint.z"] = [&]() {
                    transform.euler.z = i.at("value").get<float>();
                };
                actions["m_LocalRotation.w"] = [&]() {
                    transform.rotation.w = i.at("value").get<float>();
                };
                actions["m_LocalRotation.x"] = [&]() {
                    transform.rotation.x = i.at("value").get<float>();
                };
                actions["m_LocalRotation.y"] = [&]() {
                    transform.rotation.y = i.at("value").get<float>();
                };
                actions["m_LocalRotation.z"] = [&]() {
                    transform.rotation.z = i.at("value").get<float>();
                };
                actions["m_LocalScale.x"] = [&]() {
                    transform.scale.x = i.at("value").get<float>();
                };
                actions["m_LocalScale.y"] = [&]() {
                    transform.scale.y = i.at("value").get<float>();
                };
                actions["m_LocalScale.z"] = [&]() {
                    transform.scale.z = i.at("value").get<float>();
                };
                actions["m_LocalPosition.x"] = [&]() {
                    transform.position.x = i.at("value").get<float>();
                };
                actions["m_LocalPosition.y"] = [&]() {
                    transform.position.y = i.at("value").get<float>();
                };
                actions["m_LocalPosition.z"] = [&]() {
                    transform.position.z = i.at("value").get<float>();
                };
                actions["m_Name"] = [&]() {
                    properties.name = i.at("value").get<std::string>();
                };
                actions["m_IsActive"] = [&]() {
                    properties.active =
                        static_cast<bool>(i.at("value").get<int>());
                };
                if (actions.contains(property)) {
                    actions.at(property)();
                }
            }
        }
    }

    // After creating Transform components, update parent references
    // TODO: Consider also adding child references, somehow
    for (auto const &[fileId, entityId] : entityIdsWithTransform) {
        auto const &node{nodes.at(guid).at(fileId)};
        assert(node.contains("Transform") &&
               "This loop must operate on valid Transform components!");
        assert(node.at("Transform").contains("m_Father") &&
               node.at("Transform").at("m_Father").contains("fileID") &&
               "Transform components in game files must contain parent "
               "reference!");

        auto const &parentFileId =
            node["Transform"]["m_Father"]["fileID"].get<FileId>();
        if (parentFileId == 0) {
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

void LevelParser::loadScene(std::string const &scenePath) {
    nlohmann::json sceneNodes;
    std::ifstream(scenePath) >> sceneNodes;

    // Assume that the GUID for the scene file is the same as the path
    // guidPaths.insert({scenePath, scenePath});

    // Make a set of all identifiers in scene file
    std::set<FileId> sceneFileIds;
    for (auto const &node : sceneNodes) {
        for (auto const &[key, value] : node.items()) {
            if (!value.contains("id")) {
                continue;
            }
            sceneFileIds.insert(value.at("id").get<FileId>());
        }
    }

    // Don't keep the nodes in memory for longer than necessary
    nodes.clear();

    // Map all possible nodes with corresponding identifiers
    for (auto const &node : sceneNodes) {
        for (auto const &[key, value] : node.items()) {
            nodes[pathToGuid[scenePath]].insert(
                {value.at("id").get<FileId>(), node});
        }
    }

    // Spawn the scene and prefabs
    std::set<EntityId> recursivePrefabIds;
    spawnPrefab(pathToGuid.at(scenePath), sceneFileIds, false,
                &recursivePrefabIds);
    finalizeLoading(recursivePrefabIds);
}

Entity LevelParser::loadPrefab(std::string const &filename, bool cache) {
    assert(pathToGuid.contains(filename) &&
           "There's no prefab with that name!");

    nlohmann::json assetNodes;
    std::ifstream(filename) >> assetNodes;

    for (auto const &node : assetNodes) {
        for (auto const &[key, value] : node.items()) {
            if (!value.contains("id")) {
                continue;
            }
            prefabFileIds[pathToGuid[filename]].insert(
                value.at("id").get<FileId>());
        }
    }

    // Don't keep the nodes in memory for longer than necessary
    nodes.clear();

    // Map all possible nodes with corresponding identifiers
    for (auto const &node : assetNodes) {
        for (auto const &[key, value] : node.items()) {
            nodes[pathToGuid[filename]].insert(
                {value.at("id").get<FileId>(), node});
        }
    }

    // Spawn the prefab
    std::set<EntityId> recursivePrefabIds;
    auto const &prefabGuid = pathToGuid.at(filename);
    auto prefabEntityIds = spawnPrefab(prefabGuid, prefabFileIds.at(prefabGuid),
                                       cache, &recursivePrefabIds);

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
    auto camera =
        registry.system<PropertySystem>()->findEntityByTag("MainCamera").at(0);
    if (!camera.has<MainCamera>()) {
        camera.add<MainCamera>({.camera = std::make_shared<Camera>()});
    }
    for (auto const &entityId : recursivePrefabIds) {
        auto entity = Entity(entityId);

        // Models
        if (entity.has<Renderer>() && entity.has<MeshFilter>()) {
            auto &meshFilter = entity.get<MeshFilter>();
            auto &renderer = entity.get<Renderer>();
            Skybox *skybox =
                entity.has<Skybox>() ? &entity.get<Skybox>() : nullptr;

            // Set Skybox animation speed
            if ((entity.get<Properties>().tag == "Waterfall" ||
                 entity.get<Properties>().tag == "Trap") &&
                skybox) {
                skybox->animationSpeed = 0.0f;
            } else if (skybox) {
                skybox->animationSpeed = 0.5f;
            }

            std::string replacedPath = meshFilter.path;
            if (fs::path(meshFilter.path).stem() == "bird" ||
                fs::path(meshFilter.path).stem() == "human" ||
                fs::path(meshFilter.path).stem() == "wolf") {
                replacedPath = fs::path(meshFilter.path)
                                   .replace_extension(fs::path("gltf"))
                                   .string();
                entity.add<Animator>({.animationTime = 0.0f, .factor = 1.0f});
            }

            meshFilter.model = Model::create(
                registry.system<WindowSystem>()->gfx(), replacedPath, &renderer,
                skybox,
                entity.has<Animator>() ? &entity.get<Animator>().animationTime
                                       : nullptr);

            assert(entity.has<Transform>());
            auto &transform = entity.get<Transform>();

            auto const &meta = metaNodes.at(pathToGuid.at(meshFilter.path));
            auto const scale =
                meta["ModelImporter"]["meshes"]["globalScale"].as<float>();

            transform.scale.x *= scale;
            transform.scale.y *= scale;
            transform.scale.z *= scale;

            meshFilter.path = replacedPath;
        }

        // Colliders
        if (entity.has<SphereCollider>()) {
            entity.get<SphereCollider>() =
                registry.system<ColliderSystem>()->AddSphereCollider(
                    entity.get<MeshFilter>().model->verticesForCollision);
        }
        if (entity.has<BoxCollider>()) {
            entity.get<BoxCollider>() =
                registry.system<ColliderSystem>()->AddBoxCollider(
                    entity.get<BoxCollider>());
        }

        // AABBs for the frustum culling
        if (entity.has<AABB>()) {
            entity.get<AABB>() = registry.system<ColliderSystem>()->AddAABB(
                entity.get<MeshFilter>().model->verticesForCollision);
        }

        // UI
        if (entity.has<RectTransform>() && entity.has<UIElement>()) {
            auto const &tag = entity.get<Properties>().tag;
            auto const &rectTransform = entity.get<RectTransform>();
            auto &uiElement = entity.get<UIElement>();

            if (tag == "TextUI") {
                uiElement.text = std::make_shared<Text>(
                    registry.system<WindowSystem>()->gfx(), L"Montserrat",
                    L"Assets\\Unity\\Fonts\\montserrat-bold.otf",
                    uiElement.fontSize);

            } else if (tag == "ButtonUI") {
                uiElement.button = std::make_shared<Button>(
                    registry.system<WindowSystem>()->window(), L"Montserrat",
                    L"Assets\\Unity\\Fonts\\montserrat-bold.otf",
                    uiElement.fontSize, rectTransform.position,
                    rectTransform.size);
            }
        }

        // Zero the euler angles but leave them for the coded chunk start
        if (entity.get<Properties>().tag != "ChunkStartEndProperty") {
            if (entity.has<Transform>()) {
                auto &transform = entity.get<Transform>();
                transform.euler.x = 0.0f;
                transform.euler.y = 0.0f;
                transform.euler.z = 0.0f;
            }
        }
    }

    for (auto const &entityId : recursivePrefabIds) {
        auto entity = Entity(entityId);
        // Scripts
        if (entity.has<Behaviour>()) {
            entity.get<Behaviour>().script->setup();
        }
    }
}

void LevelParser::initialize() {
    std::unordered_map<FileExtension, std::vector<Path>> assetPaths;

    // Creating vectors of files with .prefab, .mat, .meta extensions
    Path searchedDirectory{"Assets\\Unity"};
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

    for (auto const &extension : searchedFileExtensions) {
        for (auto const &path : assetPaths[extension]) {
            if (extension == ".prefab") {
                continue;
            }
            if (extension == ".mat") {
                nlohmann::json assetNodes;
                std::ifstream(path) >> assetNodes;
                for (auto const &node : assetNodes) {
                    if (extension == ".mat") {
                        // Check for duplicates
                        assert(materialNodes.find(pathToGuid[path]) ==
                                   materialNodes.end() &&
                               "Duplicate file identifiers!");

                        materialNodes.insert({pathToGuid[path], node});
                    }
                }
            } else {
                std::vector<YAML::Node> assetNodes =
                    YAML::LoadAllFromFile(path);
                for (auto const &node : assetNodes) {
                    if (extension == ".meta") {
                        auto const pathWithoutExtension{
                            fs::path(path).parent_path().string() + "\\" +
                            fs::path(path).stem().string()};
                        guidPaths.insert({node["guid"].as<FileGuid>(),
                                          pathWithoutExtension});
                        pathToGuid.insert({pathWithoutExtension,
                                           node["guid"].as<FileGuid>()});

                        // Check for duplicates
                        assert(
                            metaNodes.find(pathToGuid[pathWithoutExtension]) ==
                                metaNodes.end() &&
                            "Duplicate file identifiers!");

                        if (!pathToGuid[pathWithoutExtension].empty()) {
                            metaNodes.insert(
                                {pathToGuid[pathWithoutExtension], node});
                        }
                    } else if (extension == ".prefab") {
                        yamlLoop(i, node) {
                            if (!i->second["id"]) {
                                continue;
                            }

                            // Check for duplicates
                            /* assert(!nodes[pathToGuid[path]].contains( */
                            /*            i->second["id"].get<FileId>()) && */
                            /*        "Duplicate file identifiers!"); */

                            /* nodes[pathToGuid[path]].insert( */
                            /*     {i->second["id"].get<FileId>(), node}); */
                            prefabFileIds[pathToGuid[path]].insert(
                                i->second["id"].as<FileId>());
                        }
                    }
                }
            }
        }
    }
}
