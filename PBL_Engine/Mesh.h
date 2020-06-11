#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <map>
#include <optional>
#include <string_view>

#include "BindableBase.h"
#include "Components/Components.hpp"
#include "RenderableBase.h"
#include "Vertex.h"

class Bone {
  public:
    aiMatrix4x4 boneOffset;
    DirectX::XMMATRIX FinalTransform;
    Bone() = default;
};
class Model;
class Mesh : public RenderableBase<Mesh> {
  public:
    struct VertexBoneData {
        UINT IDs[4];
        float weights[4];
        VertexBoneData() = default;
        void AddBoneData(UINT boneID, float boneWeight);
    };
    Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs,
         Model& parent, float* animationTime,
         std::vector<Mesh::VertexBoneData> Bones = {});
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform,
              PassType passType,
              const std::vector<std::shared_ptr<Bindable>>& refShaders,
              const std::vector<std::shared_ptr<Bindable>>& normalShaders,
              const std::vector<std::shared_ptr<Bindable>>& animatedRefShaders,
              const std::vector<std::shared_ptr<Bindable>>&
                  animatedNormalShaders) const noexcept(!IS_DEBUG);
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
    static void LoadBones(UINT meshIndex, aiMesh* pMesh,
                          std::vector<Mesh::VertexBoneData>& Bones,
                          std::vector<std::pair<std::string, Bone>>& bonesMap);
    std::vector<VertexBoneData> getBones();

  private:
    std::vector<VertexBoneData> Bones;
    mutable DirectX::XMFLOAT4X4 transform;
};

class Node {
    friend class Mesh;
    friend class Model;
    friend class ModelWindow;

  public:
    Node(const std::string& name, std::vector<Mesh*> meshPtrs,
         const DirectX::XMMATRIX& transform) noexcept(!IS_DEBUG);
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform,
              PassType passType,
              const std::vector<std::shared_ptr<Bindable>>& refShaders,
              const std::vector<std::shared_ptr<Bindable>>& normalShaders,
              const std::vector<std::shared_ptr<Bindable>>& animatedRefShaders,
              const std::vector<std::shared_ptr<Bindable>>&
                  animatedNormalShaders) const noexcept(!IS_DEBUG);
    void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;

  private:
    void AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG);
    void ShowTree(int& nodeIndex, std::optional<int>& selectedIndex,
                  Node*& pSelectedNode) const noexcept;

  private:
    std::string name;
    std::vector<std::unique_ptr<Node>> childPtrs;
    std::vector<Mesh*> meshPtrs;
    DirectX::XMFLOAT4X4 baseTransform;
    DirectX::XMFLOAT4X4 appliedTransform;
};

class Model {
  public:
    static std::shared_ptr<Model> create(Graphics& gfx,
                                         const std::string fileName,
                                         Renderer* renderer,
                                         Skybox* skybox = nullptr,
                                         float* animationTime = nullptr);
    Model(Graphics& gfx, const std::string fileName, Renderer* renderer,
          Skybox* skybox = nullptr, float* animationTime = nullptr);
    void Draw(Graphics& gfx, DirectX::XMMATRIX transform,
              PassType passType = PassType::normal) const noexcept(!IS_DEBUG);
    void ShowWindow(const char* windowName = nullptr) noexcept;
    ~Model() noexcept;
    void BoneTransform(float time,
                       std::vector<DirectX::XMFLOAT4X4>& transforms);
    void BlendBoneTransform(float time,
                            std::vector<DirectX::XMFLOAT4X4>& transforms);
    int getAnimNumber();

    std::vector<DirectX::XMFLOAT3> verticesForCollision;
    std::vector<std::shared_ptr<Texture>> textures;
    float parallaxHeight;
    Skybox* modelSkybox;

  private:
    std::shared_ptr<Mesh> ParseMesh(
        Graphics& gfx, aiMesh& mesh,
        std::vector<DirectX::XMFLOAT3>& verticesForColl);
    std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;
    void ReadNodeHierarchy(float animationTime, aiNode* pNode,
                           const DirectX::XMMATRIX& parentTransform);

    void ReadNodeHierarchyForBlend(float animationTime, float animationTime2,
                                   aiNode* currentNode,
                                   const DirectX::XMMATRIX& parentTransform,
                                   float factor);
    aiNodeAnim* FindNodeAnim(aiAnimation* pAnim,
                             std::string_view const& nodeName);
    UINT FindPosIndex(float animationTime, aiNodeAnim* pNodeAnim);
    UINT FindRotation(float animationTime, aiNodeAnim* pNodeAnim);
    UINT FindScaling(float animationTime, aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPos(aiVector3D& Out, float animationTime,
                             aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float animationTime,
                                  aiNodeAnim* pNodeAnim);
    void CalcInterpolatedScaling(aiVector3D& Out, float animationTime,
                                 aiNodeAnim* pNodeAnim);
    DirectX::XMMATRIX aiMatrixToXMMATRIX(aiMatrix4x4 aiM);
    std::vector<std::pair<std::string, Bone>> getBonesMap();

  private:
    float* animationTime;
    std::unique_ptr<Node> pRoot;
    aiNode* root;
    UINT numBones = 0;
    std::vector<std::pair<std::string, Bone>> bonesMap;
    std::vector<std::shared_ptr<Mesh>> meshPtrs;
    std::vector<aiAnimation*> animPtrs;
    std::unique_ptr<class ModelWindow> pWindow;
    std::unique_ptr<Assimp::Importer> importer;

    std::shared_ptr<VertexShader> refVert, pbrVert, animRefVert, animPbrVert;
    std::shared_ptr<GeometryShader> refGeo, pbrGeo;
    std::shared_ptr<PixelShader> refPixel, pbrPixel;

    std::vector<std::shared_ptr<Bindable>> refShaders, normalShaders,
        refShadersAnimated, normalShadersAnimated;
};

class ModelException : public ExceptionHandler {
  public:
    ModelException(int line, const char* file, std::string note) noexcept;
    const char* what() const noexcept override;
    const char* GetType() const noexcept override;
    const std::string& GetNote() const noexcept;

  private:
    std::string note;
};
