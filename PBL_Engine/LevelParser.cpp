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

std::unordered_map<FileGuid, std::unordered_map<FileId, YAML::Node>> nodes;
std::unordered_map<FileGuid, YAML::Node> materialNodes, metaNodes;
std::unordered_map<Path, FileGuid> pathToGuid;
std::unordered_map<FileGuid, Path> guidPaths;
std::unordered_map<FileGuid, std::set<FileId>> prefabFileIds;

auto &registry = Registry::instance();

#define yamlLoop(iterator, node)                         \
    for (YAML::const_iterator iterator = (node).begin(); \
         iterator != (node).end(); ++iterator)

LevelParser::LevelParser() {}

LevelParser::~LevelParser() {}

void LevelParser::cachePrefab(std::string const &filename, bool clear) {
    std::vector<YAML::Node> assetNodes = YAML::LoadAllFromFile(filename);

    // Don't keep the nodes in memory for longer than necessary
    if (clear) {
        nodes.clear();
    }

    // Map all possible nodes with corresponding identifiers
    for (auto const &node : assetNodes) {
        yamlLoop(i, node) {
            nodes[pathToGuid[filename]].insert(
                {i->second["id"].as<FileId>(), node});
        }
    }
}

std::unordered_map<FileId, EntityId> spawnPrefab(
    FileGuid guid, std::set<FileId> const &fileIds,
    std::set<EntityId> *recursivePrefabIds = nullptr) {
    // Go through all game objects in scene/prefab file and create entities
    std::unordered_map<FileId, EntityId> entityIds;
    for (auto const &fileId : fileIds) {
        auto node{nodes.at(guid)[fileId]};

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
        auto const &node = nodes.at(guid)[fileId];

        if (auto const &nodeTransform = node["Transform"]; nodeTransform) {
            assert(nodeTransform["m_PrefabInstance"] &&
                   "Every property inside Transform component must be valid!");

            bool skipThisTransform = false;
            {
                auto &helper = nodeTransform["m_PrefabInstance"];
                yamlLoop(i, helper) {
                    auto prefabFileId = i->second.Scalar();
                    skipThisTransform = prefabFileId != "0";
                }
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

            {
                auto &helper = nodeTransform["m_GameObject"];
                auto gameObjectFileId = helper["fileID"].Scalar();
                Entity(entityIds[gameObjectFileId]).add<Transform>({});
                entityIds.insert({fileId, entityIds[gameObjectFileId]});
                entityIdsWithTransform.insert(
                    {fileId, entityIds[gameObjectFileId]});
            }

            auto &transform = Entity(entityIds[fileId]).get<Transform>();

            {
                auto &helper = nodeTransform["m_LocalRotation"];
                transform.rotation.x = helper["x"].as<float>();
                transform.rotation.y = helper["y"].as<float>();
                transform.rotation.z = helper["z"].as<float>();
                transform.rotation.w = helper["w"].as<float>();
            }

            {
                auto &helper = nodeTransform["m_LocalPosition"];
                transform.position.x = helper["x"].as<float>();
                transform.position.y = helper["y"].as<float>();
                transform.position.z = helper["z"].as<float>();
            }

            {
                auto &helper = nodeTransform["m_LocalScale"];
                transform.scale.x = helper["x"].as<float>();
                transform.scale.y = helper["y"].as<float>();
                transform.scale.z = helper["z"].as<float>();
            }

            {
                auto &helper = nodeTransform["m_RootOrder"];
                transform.root_Order = helper.as<int>();
            }

            {
                auto &helper = nodeTransform["m_LocalEulerAnglesHint"];
                transform.euler.x = helper["x"].as<float>();
                transform.euler.y = helper["y"].as<float>();
                transform.euler.z = helper["z"].as<float>();
            }
        } else if (auto const &nodeMonoBehaviour = node["MonoBehaviour"];
                   nodeMonoBehaviour) {
            assert(
                nodeMonoBehaviour["m_GameObject"] &&
                nodeMonoBehaviour["m_Script"] &&
                "Every property inside MonoBehaviour component must be valid!");

            {
                auto &helper = nodeMonoBehaviour["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<Behaviour>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &behaviour = Entity(entityIds[fileId]).get<Behaviour>();

            {
                auto &helper = nodeMonoBehaviour["m_Script"];
                behaviour = registry.system<BehaviourSystem>()->behaviour(
                    fs::path(guidPaths[helper["guid"].as<std::string>()])
                        .stem()
                        .string(),
                    Entity(entityIds[fileId]));
            }
        } else if (auto const &nodeMeshRenderer = node["MeshRenderer"];
                   nodeMeshRenderer) {
            assert(
                nodeMeshRenderer["m_GameObject"] &&
                nodeMeshRenderer["m_Materials"] &&
                "Every property inside MeshRenderer component must be valid!");

            {
                auto &helper = nodeMeshRenderer["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<Renderer>({});
                    Entity(entityIds[gameObjectFileId]).add<AABB>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &renderer = Entity(entityIds[fileId]).get<Renderer>();

            {
                auto &helper = nodeMeshRenderer["m_Materials"];
                yamlLoop(i, helper[0]) {
                    auto const key = i->first.as<std::string>();
                    auto const value = i->second.as<std::string>();

                    if (key == "guid") {
                        if (materialNodes.find(value) == materialNodes.end()) {
                            continue;
                        }
                        auto const &materialNode =
                            materialNodes[value]["Material"]
                                         ["m_SavedProperties"]["m_TexEnvs"];
                        auto const &propertiesNode =
                            materialNodes[value]["Material"]
                                         ["m_SavedProperties"]["m_Floats"];
                        yamlLoop(i, materialNode) {
                            if ((*i)["_MainTex"]) {
                                renderer.material.albedoPath =
                                    guidPaths[(*i)["_MainTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_OcclusionMap"]) {
                                renderer.material.ambientOcclusionPath =
                                    guidPaths[(*i)["_OcclusionMap"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_MetallicGlossMap"]) {
                                renderer.material.metallicSmoothnessPath =
                                    guidPaths[(*i)["_MetallicGlossMap"]
                                                  ["m_Texture"]["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_BumpMap"]) {
                                renderer.material.normalPath =
                                    guidPaths[(*i)["_BumpMap"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_ParallaxMap"]) {
                                renderer.material.heightPath =
                                    guidPaths[(*i)["_ParallaxMap"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                        }
                        yamlLoop(i, propertiesNode) {
                            if ((*i)["_Parallax"]) {
                                renderer.material.parallaxHeight =
                                    (*i)["_Parallax"].as<float>();
                            }
                        }
                    }
                }
            }
        } else if (auto const &nodeMeshFilter = node["MeshFilter"];
                   nodeMeshFilter) {
            assert(nodeMeshFilter["m_GameObject"] && nodeMeshFilter["m_Mesh"] &&
                   "Every property inside MeshFilter component must be valid!");

            {
                auto &helper = nodeMeshFilter["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<MeshFilter>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &meshFilter = Entity(entityIds[fileId]).get<MeshFilter>();

            {
                auto &helper = nodeMeshFilter["m_Mesh"];
                yamlLoop(it, helper) {
                    auto const key = it->first.as<std::string>();
                    auto const value = it->second.as<std::string>();

                    if (key == "guid") {
                        meshFilter.path = guidPaths[value];
                    }
                }
            }
        } else if (auto const &nodeBoxCollider = node["BoxCollider"];
                   nodeBoxCollider) {
            assert(
                nodeBoxCollider["m_Size"] && nodeBoxCollider["m_Center"] &&
                "Every property inside BoxCollider component must be valid!");

            {
                auto &helper = nodeBoxCollider["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<BoxCollider>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &boxCollider = Entity(entityIds[fileId]).get<BoxCollider>();

            {
                auto &helper = nodeBoxCollider["m_Size"];
                boxCollider.size.x = helper["x"].as<float>();
                boxCollider.size.y = helper["y"].as<float>();
                boxCollider.size.z = helper["z"].as<float>();
            }

            {
                auto &helper = nodeBoxCollider["m_Center"];
                boxCollider.center.x = helper["x"].as<float>();
                boxCollider.center.y = helper["y"].as<float>();
                boxCollider.center.z = helper["z"].as<float>();
            }
        } else if (auto const &nodeSphereCollider = node["SphereCollider"];
                   nodeSphereCollider) {
            assert(nodeSphereCollider["m_Radius"] &&
                   nodeSphereCollider["m_Center"] &&
                   "Every property inside SphereCollider component must be "
                   "valid!");

            {
                auto &helper = nodeSphereCollider["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<SphereCollider>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &sphereCollider =
                Entity(entityIds[fileId]).get<SphereCollider>();

            {
                auto &helper = nodeSphereCollider["m_Radius"];
                sphereCollider.radius = helper.as<float>();
            }

            {
                auto &helper = nodeSphereCollider["m_Center"];
                sphereCollider.center.x = helper["x"].as<float>();
                sphereCollider.center.y = helper["y"].as<float>();
                sphereCollider.center.z = helper["z"].as<float>();
            }
        } else if (auto const &nodeRigidbody = node["Rigidbody"];
                   nodeRigidbody) {
            assert(nodeRigidbody["m_Mass"] &&
                   "Every property inside Rigidbody component must be "
                   "valid!");

            {
                auto &helper = nodeRigidbody["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<Rigidbody>({});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &rigidbody = Entity(entityIds[fileId]).get<Rigidbody>();

            {
                auto &helper = nodeRigidbody["m_Mass"];
                rigidbody.mass = helper.as<float>();
            }
        } else if (auto const &nodeSkybox = node["Skybox"]; nodeSkybox) {
            assert(
                nodeSkybox["m_GameObject"] && nodeSkybox["m_CustomSkybox"] &&
                "Every property inside MeshRenderer component must be valid!");

            {
                auto &helper = nodeSkybox["m_GameObject"];
                yamlLoop(i, helper) {
                    auto gameObjectFileId = i->second.Scalar();
                    Entity(entityIds[gameObjectFileId]).add<Skybox>({});
                    // cachedSkyboxes.insert({entityIds[gameObjectFileId],
                    // {}});
                    entityIds.insert({fileId, entityIds[gameObjectFileId]});
                }
            }

            auto &skybox = Entity(entityIds[fileId]).get<Skybox>();
            // auto &renderer = cachedRenderers.at(entityIds[fileId]);

            {
                auto &helper = nodeSkybox["m_CustomSkybox"];
                yamlLoop(i, helper) {
                    auto const key = i->first.as<std::string>();
                    auto const value = i->second.as<std::string>();

                    if (key == "guid") {
                        if (materialNodes.find(value) == materialNodes.end()) {
                            continue;
                        }
                        auto const &materialNode =
                            materialNodes[value]["Material"]
                                         ["m_SavedProperties"]["m_TexEnvs"];
                        yamlLoop(i, materialNode) {
                            if ((*i)["_BackTex"]) {
                                skybox.material.backPath =
                                    guidPaths[(*i)["_BackTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_DownTex"]) {
                                skybox.material.bottomPath =
                                    guidPaths[(*i)["_DownTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_FrontTex"]) {
                                skybox.material.frontPath =
                                    guidPaths[(*i)["_FrontTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_LeftTex"]) {
                                skybox.material.leftPath =
                                    guidPaths[(*i)["_LeftTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_RightTex"]) {
                                skybox.material.rightPath =
                                    guidPaths[(*i)["_RightTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                            if ((*i)["_UpTex"]) {
                                skybox.material.topPath =
                                    guidPaths[(*i)["_UpTex"]["m_Texture"]
                                                  ["guid"]
                                                      .as<std::string>()];
                            }
                        }
                    }
                }
            }
        }
    }

    /* std::set<FileId> usedFileIds; */
    for (auto const &fileId : fileIds) {
        auto const &node = nodes.at(guid)[fileId];

        if (auto const &nodePrefabInstance = node["PrefabInstance"];
            nodePrefabInstance) {
            assert(nodePrefabInstance["m_SourcePrefab"] &&
                   nodePrefabInstance["m_Modification"] &&
                   nodePrefabInstance["m_Modification"]["m_Modifications"] &&
                   "Every property inside PrefabInstance component must be "
                   "valid!");

            FileGuid prefabGuid =
                nodePrefabInstance["m_SourcePrefab"]["guid"].as<FileGuid>();

            LevelParser::cachePrefab(guidPaths.at(prefabGuid));
            auto prefabEntityIds = spawnPrefab(
                prefabGuid, prefabFileIds.at(prefabGuid), recursivePrefabIds);

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

                auto &transform =
                    Entity(prefabEntityIds.at(targetFileId)).get<Transform>();
                auto &properties =
                    Entity(prefabEntityIds.at(targetFileId)).get<Properties>();

                if (property == "m_RootOrder") {
                    transform.root_Order = (*i)["value"].as<int>();
                } else if (property == "m_LocalEulerAnglesHint.x") {
                    transform.euler.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalEulerAnglesHint.y") {
                    transform.euler.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalEulerAnglesHint.z") {
                    transform.euler.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.w") {
                    transform.rotation.w = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.x") {
                    transform.rotation.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.y") {
                    transform.rotation.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalRotation.z") {
                    transform.rotation.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.x") {
                    transform.scale.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.y") {
                    transform.scale.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalScale.z") {
                    transform.scale.z = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.x") {
                    transform.position.x = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.y") {
                    transform.position.y = (*i)["value"].as<float>();
                } else if (property == "m_LocalPosition.z") {
                    transform.position.z = (*i)["value"].as<float>();
                } else if (property == "m_Name") {
                    properties.name = (*i)["value"].as<std::string>();
                } else if (property == "m_IsActive") {
                    properties.active =
                        static_cast<bool>((*i)["value"].as<int>());
                }
            }
        }
    }

    // After creating Transform components, update parent references
    // TODO: Consider also adding child references, somehow
    for (auto const &[fileId, entityId] : entityIdsWithTransform) {
        auto const &node{nodes.at(guid).at(fileId)};
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

void LevelParser::loadScene(std::string const &scenePath) {
    auto const &sceneNodes = YAML::LoadAllFromFile(scenePath);

    // Assume that the GUID for the scene file is the same as the path
    // guidPaths.insert({scenePath, scenePath});

    // Make a set of all identifiers in scene file
    std::set<FileId> sceneFileIds;
    for (auto const &node : sceneNodes) {
        yamlLoop(i, node) {
            if (!i->second["id"]) {
                continue;
            }
            sceneFileIds.insert(i->second["id"].as<FileId>());
        }
    }

    // Spawn the scene and prefabs
    std::set<EntityId> recursivePrefabIds;
    cachePrefab(scenePath);
    spawnPrefab(pathToGuid.at(scenePath), sceneFileIds, &recursivePrefabIds);
    finalizeLoading(recursivePrefabIds);
}

Entity LevelParser::loadPrefab(std::string const &filename) {
    assert(pathToGuid.contains(filename) &&
           "There's no prefab with that name!");

    // Spawn the prefab
    std::set<EntityId> recursivePrefabIds;
    auto const &prefabGuid = pathToGuid.at(filename);
    auto prefabEntityIds = spawnPrefab(prefabGuid, prefabFileIds.at(prefabGuid),
                                       &recursivePrefabIds);

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
            Skybox *skybox =
                entity.has<Skybox>() ? &entity.get<Skybox>() : nullptr;

            std::string replacedPath = meshFilter.path;
            if (fs::path(meshFilter.path).stem() == "bird" ||
                fs::path(meshFilter.path).stem() == "human" ||
                fs::path(meshFilter.path).stem() == "wolf") {
                replacedPath = fs::path(meshFilter.path)
                                   .replace_extension(fs::path("gltf"))
                                   .string();
                entity.add<Animator>({.animationTime = 0.0f});
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

        // Scripts
        if (entity.has<Behaviour>()) {
            entity.get<Behaviour>().script->setup();
        }

        // Zero the euler angles but leave them for the coded chunk start
        if (entity.get<Properties>().tag != "ChunkStartEndProperty") {
            auto &transform = entity.get<Transform>();
            transform.euler.x = 0.0f;
            transform.euler.y = 0.0f;
            transform.euler.z = 0.0f;
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
            // if (extension == ".prefab") {
            //     continue;
            // }
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

                    // Check for duplicates
                    assert(metaNodes.find(pathToGuid[pathWithoutExtension]) ==
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
                        /*            i->second["id"].as<FileId>()) && */
                        /*        "Duplicate file identifiers!"); */

                        /* nodes[pathToGuid[path]].insert( */
                        /*     {i->second["id"].as<FileId>(), node}); */
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
}
