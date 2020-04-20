#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <optional>

#include "BindableBase.h"
#include "RenderableBase.h"
#include "Vertex.h"

class Mesh : public RenderableBase<Mesh> {
  public:
    Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs);
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
        noexcept(!IS_DEBUG);
    DirectX::XMMATRIX GetTransformXM() const noexcept override;

  private:
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
    static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);
    std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;

  private:
    std::unique_ptr<Node> pRoot;
    std::vector<std::unique_ptr<Mesh>> meshPtrs;
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