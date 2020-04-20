#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <map>
#include <optional>

#include "BindableBase.h"
#include "RenderableBase.h"
#include "Vertex.h"

class Mesh : public RenderableBase<Mesh> {
  public:
    struct VertexBoneData {
        UINT IDs[4];
        float weights[4];
        VertexBoneData() = default;
        void AddBoneData(UINT boneID, float boneWeight);
    };
    struct Bone {
        aiMatrix4x4 boneOffset;
        DirectX::XMMATRIX FinalTransform;
        Bone() = default;
    };
    Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs,
         std::vector<Mesh::VertexBoneData> Bones = {},
         std::vector<std::pair<std::string, Mesh::Bone>> bonesMap = {});
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
        noexcept(!IS_DEBUG);
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
    static void LoadBones(UINT meshIndex, aiMesh* pMesh,
                          std::vector<Mesh::VertexBoneData>& Bones,
                          std::vector<std::pair<std::string, Bone>>& bonesMap);

  private:
    std::vector<std::pair<std::string, Bone>> bonesMap;
    std::vector<VertexBoneData> Bones;

    mutable DirectX::XMFLOAT4X4 transform;
};

class Node {
    friend class Model;
    friend class ModelWindow;

  public:
    Node(const std::string& name, std::vector<Mesh*> meshPtrs,
         const DirectX::XMMATRIX& transform) noexcept(!IS_DEBUG);
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
        noexcept(!IS_DEBUG);
    void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnim);

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
    Model(Graphics& gfx, const std::string fileName);
    void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
    void ShowWindow(const char* windowName = nullptr) noexcept;
    ~Model() noexcept;

  private:
    static std::shared_ptr<Mesh> ParseMesh(Graphics& gfx, aiMesh& mesh);
    std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;

  private:
    std::unique_ptr<Node> pRoot;
    std::vector<std::shared_ptr<Mesh>> meshPtrs;
    std::unique_ptr<class ModelWindow> pWindow;
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