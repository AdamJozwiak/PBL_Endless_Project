#include "Mesh.h"

#include <Sampler.h>
#include <assimp/postprocess.h>
#include <array>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "BonesCbuf.h"
#include "Surface.h"
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
           Model& parent, float* animationTime,
           std::vector<Mesh::VertexBoneData> Bones)
    : Bones(Bones) {
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
    if (animationTime) {
        AddBind(std::make_unique<BonesCbuf>(gfx, parent, animationTime));
    }
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
            bone.boneOffset = pMesh->mBones[i]->mOffsetMatrix;
            bonesMap.emplace_back(std::make_pair(BoneName, bone));
        }
        const aiBone* pBone = pMesh->mBones[i];
        for (UINT j = 0; j < pBone->mNumWeights; j++) {
            UINT vertexID = pBone->mWeights[j].mVertexId;
            float weight = pBone->mWeights[j].mWeight;
            Bones[vertexID].AddBoneData(boneIndex, weight);
        }
    }
}
std::vector<Mesh::VertexBoneData> Mesh::getBones() { return Bones; }

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

// Model
class ModelWindow {
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

Model::Model(Graphics& gfx, const std::string fileName, float* animationTime)
    : pWindow(std::make_unique<ModelWindow>()), animationTime(animationTime) {
    importer = std::make_unique<Assimp::Importer>();
    const auto pScene = importer->ReadFile(
        fileName.c_str(),
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
            aiProcess_ConvertToLeftHanded | aiProcess_GenNormals |
            aiProcess_CalcTangentSpace | aiProcess_GenUVCoords |
            aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
    if (pScene == nullptr) {
        throw ModelException(__LINE__, __FILE__, importer->GetErrorString());
    }
    root = pScene->mRootNode;
    if (pScene->HasAnimations()) {
        for (size_t i = 0; i < pScene->mNumAnimations; i++) {
            animPtrs.push_back(pScene->mAnimations[i]);
        }
    }

    for (size_t i = 0; i < pScene->mNumMeshes; i++) {
        auto pMesh = ParseMesh(gfx, *pScene->mMeshes[i], verticesForCollision);
        numBones += pScene->mMeshes[i]->mNumBones;
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

std::shared_ptr<Mesh> Model::ParseMesh(
    Graphics& gfx, aiMesh& mesh,
    std::vector<DirectX::XMFLOAT3>& verticesForColl) {
    namespace dx = DirectX;
    using pblexp::VertexLayout;
    std::vector<Mesh::VertexBoneData> Bones;
    std::vector<std::array<UINT, 4>> bonesID;
    std::vector<std::array<float, 4>> weights;
    VertexLayout vl;
    vl.Append(VertexLayout::Position3D).Append(VertexLayout::Normal);
    if (mesh.HasBones()) {
        Mesh::LoadBones(1, &mesh, Bones, bonesMap);
        for (int x = 0; x < Bones.size(); ++x) {
            bonesID.push_back({});
            weights.push_back({});
            for (UINT i = 0; i < 4; i++) {
                bonesID[x][i] = Bones[x].IDs[i];
                weights[x][i] = Bones[x].weights[i];
            }
        }
        vl.Append(VertexLayout::BoneID).Append(VertexLayout::BoneWeight);
    }
    pblexp::VertexBuffer vbuf(std::move(vl));

    for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
        dx::XMFLOAT3 v;
        float s = 1.0f;
        v.x = s * mesh.mVertices[i].x;
        v.y = s * mesh.mVertices[i].y;
        v.z = s * mesh.mVertices[i].z;
        verticesForColl.emplace_back(v);

        if (mesh.HasBones()) {
            vbuf.EmplaceBack(
                *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
                *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
                *reinterpret_cast<dx::XMUINT4*>(bonesID[i].data()),
                *reinterpret_cast<dx::XMFLOAT4*>(weights[i].data()));
        } else
            vbuf.EmplaceBack(
                *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
                *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]));
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
    if (mesh.HasBones()) {
        pvs = std::make_unique<VertexShader>(gfx, L"AnimatedPhongVS.cso");
    }
    auto pvsbc = pvs->GetBytecode();
    bindablePtrs.push_back(std::move(pvs));

    bindablePtrs.push_back(std::make_unique<GeometryShader>(gfx, L"PhongGS.cso"));

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
                                                                  0u));

    return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), *this,
                                  animationTime, Bones);
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

void Model::ReadNodeHierarchy(float animationTime, aiNode* pNode,
                              const DirectX::XMMATRIX& parentTransform) {
    std::string nodeName(pNode->mName.data);
    aiAnimation* pAnim = animPtrs[0];
    DirectX::XMMATRIX nodeTransformation =
        DirectX::XMMATRIX(&pNode->mTransformation.a1);
    aiNodeAnim* pNodeAnim = FindNodeAnim(pAnim, nodeName);
    DirectX::XMMATRIX anim = DirectX::XMMatrixIdentity();

    if (pNodeAnim) {
        aiVector3D scale;
        CalcInterpolatedScaling(scale, animationTime, pNodeAnim);
        DirectX::XMMATRIX scalingM =
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        aiQuaternion rotation;
        CalcInterpolatedRotation(rotation, animationTime, pNodeAnim);
        DirectX::XMMATRIX rotationM =
            DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(
                rotation.x, rotation.y, rotation.z, rotation.w));

        aiVector3D position;
        CalcInterpolatedPos(position, animationTime, pNodeAnim);
        DirectX::XMMATRIX translationM =
            DirectX::XMMatrixTranslation(position.x, position.y, position.z);

        nodeTransformation = scalingM * rotationM * translationM;
        nodeTransformation = DirectX::XMMatrixTranspose(nodeTransformation);
    }
    DirectX::XMMATRIX globalTransformation =
        parentTransform * nodeTransformation;

    for (auto& it : bonesMap) {
        if (it.first == nodeName) {
            it.second.FinalTransform =
                // aiMatrixToXMMATRIX(root->mTransformation.Inverse()) *
                globalTransformation * aiMatrixToXMMATRIX(it.second.boneOffset);
            break;
        }
    }
    for (UINT i = 0; i < pNode->mNumChildren; ++i) {
        ReadNodeHierarchy(animationTime, pNode->mChildren[i],
                          globalTransformation);
    }
}
void Model::BoneTransform(float time,
                          std::vector<DirectX::XMFLOAT4X4>& transforms) {
    DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    if (animPtrs.empty()) {
        return;
    }
    float ticksPreSecond =
        (float)(animPtrs[0]->mTicksPerSecond != 0 ? animPtrs[0]->mTicksPerSecond
                                                  : 25.0f);
    float timeInTicks = time * ticksPreSecond;
    float animationTime = fmod(timeInTicks, (float)animPtrs[0]->mDuration);
    ReadNodeHierarchy(animationTime, root, identity);

    // transforms.resize(numBones);
    transforms.clear();
    for (auto& [name, bone] : bonesMap) {
        DirectX::XMFLOAT4X4 tmp;
        DirectX::XMStoreFloat4x4(&tmp, bone.FinalTransform);
        transforms.emplace_back(tmp);
    }
}
aiNodeAnim* Model::FindNodeAnim(aiAnimation* pAnim,
                                std::string_view const& nodeName) {
    for (UINT i = 0; i < pAnim->mNumChannels; i++) {
        aiNodeAnim* pNodeAnim = pAnim->mChannels[i];
        if (std::string_view(pNodeAnim->mNodeName.data) == nodeName) {
            return pNodeAnim;
        }
    }
    return NULL;
}

UINT Model::FindPos(float animationTime, aiNodeAnim* pNodeAnim) {
    for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);
    return 0;
}

UINT Model::FindRotation(float animationTime, aiNodeAnim* pNodeAnim) {
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}

UINT Model::FindScaling(float animationTime, aiNodeAnim* pNodeAnim) {
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}

void Model::CalcInterpolatedPos(aiVector3D& Out, float animationTime,
                                aiNodeAnim* pNodeAnim) {
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }
    UINT posIndex = FindPos(animationTime, pNodeAnim);
    UINT nextPosIndex = (posIndex + 1);
    assert(nextPosIndex < pNodeAnim->mNumPositionKeys);
    float deltaTime = (float)(pNodeAnim->mPositionKeys[nextPosIndex].mTime -
                              pNodeAnim->mPositionKeys[posIndex].mTime);
    float factor =
        (animationTime - (float)pNodeAnim->mPositionKeys[posIndex].mTime) /
        deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    const aiVector3D& start = pNodeAnim->mPositionKeys[posIndex].mValue;
    const aiVector3D& end = pNodeAnim->mPositionKeys[nextPosIndex].mValue;
    aiVector3D delta = end - start;
    Out = start + factor * delta;
}

void Model::CalcInterpolatedRotation(aiQuaternion& Out, float animationTime,
                                     aiNodeAnim* pNodeAnim) {
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    UINT RotationIndex = FindRotation(animationTime, pNodeAnim);
    UINT NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime =
        (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime -
                pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor =
        (animationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) /
        DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ =
        pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ =
        pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}

void Model::CalcInterpolatedScaling(aiVector3D& Out, float animationTime,
                                    aiNodeAnim* pNodeAnim) {
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    UINT ScalingIndex = FindScaling(animationTime, pNodeAnim);
    UINT NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime -
                              pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor =
        (animationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) /
        DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

DirectX::XMMATRIX Model::aiMatrixToXMMATRIX(aiMatrix4x4 aiM) {
    return DirectX::XMMATRIX(&aiM.a1);
    /*return DirectX::XMMATRIX(aiM[0][0], aiM[0][1], aiM[0][2], aiM[0][3],
                             aiM[1][0], aiM[1][1], aiM[1][2], aiM[1][3],
                             aiM[2][0], aiM[2][1], aiM[2][2], aiM[2][3],
                             aiM[3][0], aiM[3][1], aiM[3][2], aiM[3][3]);*/
}

std::vector<std::pair<std::string, Bone>> Model::getBonesMap() {
    return bonesMap;
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
