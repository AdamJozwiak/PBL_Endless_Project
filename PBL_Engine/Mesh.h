#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

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

  public:
    Node(const std::string& name, std::vector<Mesh*> meshPtrs,
         const DirectX::XMMATRIX& transform) noexcept(!IS_DEBUG);
    void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
        noexcept(!IS_DEBUG);
    void RenderTree() const noexcept;

  private:
    void AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG);

  private:
    std::string name;
    std::vector<std::unique_ptr<Node>> childPtrs;
    std::vector<Mesh*> meshPtrs;
    DirectX::XMFLOAT4X4 transform;
};

class Model {
  public:
    Model(Graphics& gfx, const std::string fileName);
    ~Model() noexcept;
    void Draw(Graphics& gfx) const;
    void ShowWindow(const char* windowName = nullptr) noexcept;

  private:
    static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);
    std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;

  private:
    std::unique_ptr<Node> pRoot;
    std::vector<std::unique_ptr<Mesh>> meshPtrs;
    struct {
        float roll = 0.0f;
        float pitch = 0.0f;
        float yaw = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    } pos;
};