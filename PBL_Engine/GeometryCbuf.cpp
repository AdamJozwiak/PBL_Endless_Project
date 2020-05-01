#include "GeometryCbuf.h"

GeometryCbuf::GeometryCbuf(Graphics& gfx, const Renderable& parent,
                           DirectX::XMVECTOR cameraPos, UINT slot)
    : parent(parent) {
    if (!pGcbuf) {
        DirectX::XMStoreFloat4(&pCamPos, cameraPos);
        pGcbuf =
            std::make_unique<GeometryConstantBuffer<Transforms>>(gfx, slot);
    }
}

void GeometryCbuf::Bind(Graphics& gfx) noexcept {
    const Transforms tf = {
        DirectX::XMMatrixTranspose(parent.GetTransformXM()),
        DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection()),
        pCamPos, textureWidth, textureHeight};
    pGcbuf->Update(gfx, tf);
    pGcbuf->Bind(gfx);
}

void GeometryCbuf::SetTextureWidth(float textureW) { textureWidth = textureW; }

void GeometryCbuf::SetTextureHeight(float textureH) {
    textureHeight = textureH;
}

std::unique_ptr<GeometryConstantBuffer<GeometryCbuf::Transforms>>
    GeometryCbuf::pGcbuf;