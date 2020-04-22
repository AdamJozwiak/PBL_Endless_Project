#include "Mesh.h"

#include <sstream>
#include <unordered_map>

#include "imgui/imgui.h"

namespace dx = DirectX;

void Mesh::VertexBoneData::AddBoneData(UINT boneID, float boneWeight) {
    int size = sizeof(IDs) / sizeof(*IDs);
    for (UINT i = 0; i < size; i++) {
        if (weights[i] == 0.0) {
            IDs[i] = boneID;
            weights[i] = boneWeight;
            return;
        }
    }
}

// Mesh
Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs,
           std::vector<Mesh::VertexBoneData> Bones,
           std::vector<std::pair<std::string, Mesh::Bone>> bonesMap)
    : Bones(Bones), bonesMap(bonesMap) {
    if (!IsStaticInitialized()) {
        AddStaticBind(std::make_unique<Topology>(
            gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    }

    for (auto& pb : bindPtrs) {
        if (auto pi = dynamic_cast<IndexBuffer*>(pb.get())) {
            AddIndexBuffer(std::unique_ptr<IndexBuffer>{pi});
            pb.release();
        } else {
            AddBind(std::move(pb));
        }
    }

    AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}
void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
    noexcept(!IS_DEBUG) {
    DirectX::XMStoreFloat4x4(&transform, accumulatedTransform);
    Renderable::Draw(gfx);
}
DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept {
    return DirectX::XMLoadFloat4x4(&transform);
}

void Mesh::LoadBones(UINT meshIndex, aiMesh* pMesh,
                     std::vector<Mesh::VertexBoneData>& Bones,
                     std::vector<std::pair<std::string, Bone>>& bonesMap) {
    for (UINT i = 0; i < pMesh->mNumVertices; i++) {
        Bones.push_back(VertexBoneData());
    }
    for (UINT i = 0; i < pMesh->mNumBones; i++) {
        int boneIndex = -1;
        std::string BoneName(pMesh->mBones[i]->mName.data);
        int tmpIndex = 0;
        for (const auto& p : bonesMap) {
            if (p.first == BoneName) {
                boneIndex = tmpIndex;
                break;
            }
            tmpIndex++;
        }
        if (boneIndex < 0) {
            boneIndex = (int)bonesMap.size();
            Bone bone;
            bone.boneOffset = pMesh->mBones[boneIndex]->mOffsetMatrix;
            bonesMap.emplace_back(std::make_pair(BoneName, bone));
        }
        const aiBone* pBone = pMesh->mBones[boneIndex];
        for (UINT j = 0; j < pBone->mNumWeights; j++) {
            UINT vertexID = pBone->mWeights[j].mVertexId;
            float weight = pBone->mWeights[j].mWeight;
            Bones[vertexID].AddBoneData(boneIndex, weight);
        }
    }
}

// Node
Node::Node(const std::string& name, std::vector<Mesh*> meshPtrs,
           const DirectX::XMMATRIX& transform_in) noexcept(!IS_DEBUG)
    : meshPtrs(std::move(meshPtrs)), name(name) {
    dx::XMStoreFloat4x4(&baseTransform, transform_in);
    dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const
    noexcept(!IS_DEBUG) {
    const auto built = dx::XMLoadFloat4x4(&appliedTransform) *
                       dx::XMLoadFloat4x4(&baseTransform) *
                       accumulatedTransform;
    for (const auto pm : meshPtrs) {
        pm->Draw(gfx, built);
    }
    for (const auto& pc : childPtrs) {
        pc->Draw(gfx, built);
    }
}

void Node::AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG) {
    assert(pChild);
    childPtrs.push_back(std::move(pChild));
}

void Node::ShowTree(int& nodeIndexTracked, std::optional<int>& selectedIndex,
                    Node*& pSelectedNode) const noexcept {
    // nodeIndex serves as the uid for gui tree nodes, incremented throughout
    // recursion
    const int currentNodeIndex = nodeIndexTracked;
    nodeIndexTracked++;
    // build up flags for current node
    const auto node_flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ((currentNodeIndex == selectedIndex.value_or(-1))
             ? ImGuiTreeNodeFlags_Selected
             : 0) |
        ((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
    // render this node
    const auto expanded = ImGui::TreeNodeEx((void*)(intptr_t)currentNodeIndex,
                                            node_flags, name.c_str());
    // processing for selecting node
    if (ImGui::IsItemClicked()) {
        selectedIndex = currentNodeIndex;
        pSelectedNode = const_cast<Node*>(this);
    }
    // recursive rendering of open node's children
    if (expanded) {
        for (const auto& pChild : childPtrs) {
            pChild->ShowTree(nodeIndexTracked, selectedIndex, pSelectedNode);
        }
        ImGui::TreePop();
    }
}

void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept {
    dx::XMStoreFloat4x4(&appliedTransform, transform);
}

const aiNodeAnim* Node::FindNodeAnim(const aiAnimation* pAnim) {
    for (UINT i = 0; i < pAnim->mNumChannels; i++) {
        const aiNodeAnim* pNodeAnim = pAnim->mChannels[i];
        if (std::string(pNodeAnim->mNodeName.data) == name) {
            return pNodeAnim;
        }
    }
}

// Model
class ModelWindow  // pImpl idiom, only defined in this .cpp
{
  public:
    void Show(const char* windowName, const Node& root) noexcept {
        // window name defaults to "Model"
        windowName = windowName ? windowName : "Model";
        // need an ints to track node indices and selected node
        int nodeIndexTracker = 0;
        if (ImGui::Begin(windowName)) {
            ImGui::Columns(2, nullptr, true);
            root.ShowTree(nodeIndexTracker, selectedIndex, pSelectedNode);

            ImGui::NextColumn();
            if (pSelectedNode != nullptr) {
                auto& transform = transforms[*selectedIndex];
                ImGui::Text("Orientation");
                ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
                ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
                ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
                ImGui::Text("Position");
                ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
                ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
                ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
            }
        }
        ImGui::End();
    }
    dx::XMMATRIX GetTransform() const noexcept {
        const auto& transform = transforms.at(*selectedIndex);
        return dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch,
                                                transform.yaw) *
               dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
    }
    Node* GetSelectedNode() const noexcept { return pSelectedNode; }

  private:
    std::optional<int> selectedIndex;
    Node* pSelectedNode;
    struct TransformParameters {
        float roll = 0.0f;
        float pitch = 0.0f;
        float yaw = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };
    std::unordered_map<int, TransformParameters> transforms;
};

Model::Model(Graphics& gfx, const std::string fileName)
    : pWindow(std::make_unique<ModelWindow>()) {
    Assimp::Importer imp;
    const auto pScene =
        imp.ReadFile(fileName.c_str(),
                     aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                         aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);
    if (pScene == nullptr) {
        throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
    }
    for (size_t i = 0; i < pScene->mNumMeshes; i++) {
        auto pMesh = ParseMesh(gfx, *pScene->mMeshes[i]);
        meshPtrs.push_back(pMesh);
    }

    pRoot = ParseNode(*pScene->mRootNode);
}

void Model::Draw(Graphics& gfx, DirectX::XMMATRIX transform) const
    noexcept(!IS_DEBUG) {
    if (auto node = pWindow->GetSelectedNode()) {
        node->SetAppliedTransform(pWindow->GetTransform());
    }
    pRoot->Draw(gfx, transform);
}

void Model::ShowWindow(const char* windowName) noexcept {
    pWindow->Show(windowName, *pRoot);
}

Model::~Model() noexcept {}

std::shared_ptr<Mesh> Model::ParseMesh(Graphics& gfx, aiMesh& mesh) {
    namespace dx = DirectX;
    using pblexp::VertexLayout;
    std::vector<std::pair<std::string, Mesh::Bone>> bonesMap;
    std::vector<Mesh::VertexBoneData> Bones;
    std::vector<UINT> bonesID;
    std::vector<float> weights;
    VertexLayout vl;
    vl.Append(VertexLayout::Position3D).Append(VertexLayout::Normal);
    if (mesh.HasBones()) {
        Mesh::LoadBones(1, &mesh, Bones, bonesMap);
        for (auto it : Bones) {
            for (UINT i = 0; i < 4; i++) {
                bonesID.push_back(it.IDs[i]);
                weights.push_back(it.weights[i]);
            }
        }
        vl.Append(VertexLayout::BoneID).Append(VertexLayout::BoneWeight);
    }

    // bones need to be added into vbuffer
    pblexp::VertexBuffer vbuf(std::move(vl));

    for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
        vbuf.EmplaceBack(*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
                         *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]));
        if (mesh.HasBones()) {
            vbuf.EmplaceBack(*reinterpret_cast<dx::XMUINT4*>(bonesID.data()),
                             *reinterpret_cast<dx::XMFLOAT4*>(weights.data()));
        }
    }

    std::vector<unsigned short> indices;
    indices.reserve(mesh.mNumFaces * 3);
    for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
        const auto& face = mesh.mFaces[i];
        assert(face.mNumIndices == 3);
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    std::vector<std::unique_ptr<Bindable>> bindablePtrs;

    bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

    bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));

    auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
    auto pvsbc = pvs->GetBytecode();
    bindablePtrs.push_back(std::move(pvs));

    bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

    bindablePtrs.push_back(std::make_unique<InputLayout>(
        gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

    struct PSMaterialConstant {
        DirectX::XMFLOAT3 color = {0.6f, 0.6f, 0.8f};
        float specularIntensity = 0.6f;
        float specularPower = 30.0f;
        float padding[3];
    } pmc;
    bindablePtrs.push_back(
        std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc,
                                                                  1u));

    return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), Bones,
                                  bonesMap);
}
std::unique_ptr<Node> Model::ParseNode(const aiNode& node) noexcept {
    namespace dx = DirectX;
    const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
        reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)));

    std::vector<Mesh*> curMeshPtrs;
    curMeshPtrs.reserve(node.mNumMeshes);
    for (size_t i = 0; i < node.mNumMeshes; i++) {
        const auto meshIdx = node.mMeshes[i];
        curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
    }

    auto pNode = std::make_unique<Node>(node.mName.C_Str(),
                                        std::move(curMeshPtrs), transform);
    for (size_t i = 0; i < node.mNumChildren; i++) {
        pNode->AddChild(ParseNode(*node.mChildren[i]));
    }

    return pNode;
}

ModelException::ModelException(int line, const char* file,
                               std::string note) noexcept
    : ExceptionHandler(line, file), note(std::move(note)) {}

const char* ModelException::what() const noexcept {
    std::ostringstream oss;
    oss << ExceptionHandler::what() << std::endl << "[Note] " << GetNote();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* ModelException::GetType() const noexcept {
    return "Model Exception";
}

const std::string& ModelException::GetNote() const noexcept { return note; }
