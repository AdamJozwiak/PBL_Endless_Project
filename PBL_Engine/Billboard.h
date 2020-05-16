#pragma once
#include "BindableBase.h"
#include "Camera.h"
#include "GraphicsThrowMacros.h"
#include "Plane.h"
#include "RenderableBase.h"
#include "Sampler.h"
#include "Surface.h"
#include "Texture.h"

class Billboard : public RenderableBase<Billboard> {
  public:
    Billboard(Graphics& gfx, Camera* camera) {
        namespace dx = DirectX;
        if (!IsStaticInitialized()) {
            struct Vertex {
                dx::XMFLOAT3 pos;
                struct {
                    float u;
                    float v;
                } tex;
            };

            Vertex model;
            model.pos = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);

            std::vector<dx::XMFLOAT3> vertices;
            std::vector<unsigned short> indices;
            vertices.push_back(model.pos);
            indices.push_back(1);

            AddStaticBind(std::make_unique<Texture>(
                gfx, Surface::FromFile("Assets/Graphics/kappa.png")));

            AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

            AddStaticBind(std::make_unique<Sampler>(gfx));

            auto pvs = std::make_unique<VertexShader>(gfx, L"BillboardVS.cso");
            auto pvsbc = pvs->GetBytecode();
            AddStaticBind(std::move(pvs));

            AddStaticBind(
                std::make_unique<GeometryShader>(gfx, L"BillboardGS.cso"));

            AddStaticBind(
                std::make_unique<PixelShader>(gfx, L"BillboardPS.cso"));

            AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

            const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

            AddStaticBind(std::make_unique<Topology>(
                gfx, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST));
        } else {
            SetIndexFromStatic();
        }

        auto texture =
            Texture(gfx, Surface::FromFile("Assets/Graphics/kappa.png"));

        auto geoCbuf = GeometryCbuf(gfx, *this, camera);
        geoCbuf.SetTextureWidth(texture.GetTextureWidth());
        geoCbuf.SetTextureHeight(texture.GetTextureHeight());

        AddBind(std::make_unique<TransformCbuf>(gfx, *this));

        AddBind(std::make_unique<GeometryCbuf>(geoCbuf));
    }

    void Update(float dt) noexcept {}

    DirectX::XMMATRIX GetTransformXM() const noexcept {
        return DirectX::XMMatrixScaling(scale, scale, scale) *
               DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
    }

  public:
    float scale = 0.1f;

  private:
    DirectX::XMFLOAT3 pos = {1.0f, 1.0f, 10.0f};
};
