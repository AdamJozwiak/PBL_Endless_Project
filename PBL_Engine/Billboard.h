#pragma once
#include "BindableBase.h"
#include "GraphicsThrowMacros.h"
#include "Plane.h"
#include "RenderableBase.h"
#include "Sampler.h"
#include "Surface.h"
#include "Texture.h"

class Billboard : public RenderableBase<Billboard> {
  public:
    Billboard(Graphics& gfx, DirectX::XMVECTOR cameraPos) {
        namespace dx = DirectX;
        if (!IsStaticInitialized()) {
            struct Vertex {
                dx::XMFLOAT3 pos;
                struct {
                    float u;
                    float v;
                } tex;
            };
            auto model = Plane::Make<Vertex>();
            model.vertices[0].tex = {0.0f, 0.0f};
            model.vertices[1].tex = {1.0f, 0.0f};
            model.vertices[2].tex = {0.0f, 1.0f};
            model.vertices[3].tex = {1.0f, 1.0f};

            AddStaticBind(std::make_unique<Texture>(
                gfx, Surface::FromFile("../Executable/Graphics/kappa.png")));

            AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

            AddStaticBind(std::make_unique<Sampler>(gfx));

            auto pvs = std::make_unique<VertexShader>(gfx, L"TextureVS.cso");
            auto pvsbc = pvs->GetBytecode();
            AddStaticBind(std::move(pvs));

            AddStaticBind(
                std::make_unique<GeometryShader>(gfx, L"BillboardGS.cso"));

            AddStaticBind(std::make_unique<PixelShader>(gfx, L"TexturePS.cso"));

            AddStaticIndexBuffer(
                std::make_unique<IndexBuffer>(gfx, model.indices));

            const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
                {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

            AddStaticBind(std::make_unique<Topology>(
                gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
        } else {
            SetIndexFromStatic();
        }

        auto texture =
            Texture(gfx, Surface::FromFile("../Executable/Graphics/kappa.png"));

        auto geoCbuf = GeometryCbuf(gfx, *this, cameraPos, 13);
        geoCbuf.SetTextureWidth(texture.GetTextureWidth());
        geoCbuf.SetTextureHeight(texture.GetTextureHeight());

        AddBind(std::make_unique<GeometryCbuf>(geoCbuf));

        AddBind(std::make_unique<TransformCbuf>(gfx, *this));
    }

    void Update(float dt) noexcept {}

    DirectX::XMMATRIX GetTransformXM() const noexcept {
        return DirectX::XMMatrixScaling(scale, scale, scale) *
               DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
    }

  public:
    float scale = 1.0f;

  private:
    DirectX::XMFLOAT3 pos = {1.0f, 1.0f, 1.0f};
};