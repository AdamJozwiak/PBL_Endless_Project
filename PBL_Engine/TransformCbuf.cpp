#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Renderable& parent, UINT slot)
    : parent(parent) {
    if (!pVcbuf) {
        pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
    }
}

void TransformCbuf::Bind(Graphics& gfx) noexcept {
    Transforms tf = {
        .model = DirectX::XMMatrixTranspose(parent.GetTransformXM()),
        .viewProj =
            DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection())};
    tf.modelInverseTranspose =
        DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, tf.model));
    pVcbuf->Update(gfx, tf);
    pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>>
    TransformCbuf::pVcbuf;
