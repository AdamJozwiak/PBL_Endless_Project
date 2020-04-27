#include "BonesCbuf.h"
BonesCbuf::BonesCbuf(Graphics& gfx, Model& parent, float* animationTime,
                     UINT slot)
    : parent(parent), animationTime(animationTime) {
    if (!pVcbuf) {
        pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
    }
}

void BonesCbuf::Bind(Graphics& gfx) noexcept {
    std::vector<DirectX::XMFLOAT4X4> transforms;
    parent.BoneTransform(*animationTime, transforms);
    Transforms tf;
    for (UINT i = 0; i < 256; i++) {
        if (i < transforms.size()) {
            tf.BoneTransforms[i] = DirectX::XMLoadFloat4x4(&transforms[i]);
        }
    }
    pVcbuf->Update(gfx, tf);
    pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<BonesCbuf::Transforms>> BonesCbuf::pVcbuf;
