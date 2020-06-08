#include "GeometryCbuf.h"

GeometryCbuf::GeometryCbuf(Graphics& gfx, const Renderable& parent,
                           Camera* camera, UINT slot)
    : parent(parent), pCamera(camera) {
    if (!pGcbuf) {
        pGcbuf =
            std::make_unique<GeometryConstantBuffer<Transforms>>(gfx, slot);
    }
}

void GeometryCbuf::Bind(Graphics& gfx) noexcept {
    DirectX::XMStoreFloat4(&camPos, pCamera->GetCameraPos());
    const Transforms tf = {
        DirectX::XMMatrixTranspose(parent.GetTransformXM()),
        DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection()),
        camPos, textureWidth, textureHeight};
    pGcbuf->Update(gfx, tf);
    pGcbuf->Bind(gfx);
}

void GeometryCbuf::SetTextureWidth(float textureW) { textureWidth = textureW; }

void GeometryCbuf::SetTextureHeight(float textureH) {
    textureHeight = textureH;
}

std::unique_ptr<GeometryConstantBuffer<GeometryCbuf::Transforms>>
    GeometryCbuf::pGcbuf;