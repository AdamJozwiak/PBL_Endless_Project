#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Renderable& parent, UINT slot)
    : parent(parent) {
    if (!pVcbuf) {
        pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
    }
}

void TransformCbuf::Bind(Graphics& gfx) noexcept {
    const Transforms tf = {
        DirectX::XMMatrixTranspose(parent.GetTransformXM()),
        DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection())};
    pVcbuf->Update(gfx, tf);
    pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>>
    TransformCbuf::pVcbuf;
