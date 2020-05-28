#pragma once
#include "BindableBase.h"
#include "Camera.h"
#include "GraphicsThrowMacros.h"
#include "RenderableBase.h"
#include "Sampler.h"
#include "Surface.h"

class FireParticle : public RenderableBase<FireParticle> {
  public:
    FireParticle(Graphics& gfx, Camera* camera) {
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
                gfx,
                std::ref(Surface::FromFile("Assets/Graphics/fire-albedo.png")),
                0));
            AddStaticBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile("Assets/Graphics/fire-noise.png")),
                1));
            AddStaticBind(std::make_unique<Texture>(
                gfx,
                std::ref(
                    Surface::FromFile("Assets/Graphics/fire-gradient.png")),
                2));
            AddStaticBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile("Assets/Graphics/fire-mask.png")),
                3));

            AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

            AddStaticBind(std::make_unique<Sampler>(gfx));

            auto pvs = std::make_unique<VertexShader>(gfx, L"FireEffectVS.cso");
            auto pvsbc = pvs->GetBytecode();
            AddStaticBind(std::move(pvs));

            AddStaticBind(
                std::make_unique<GeometryShader>(gfx, L"FireEffectGS.cso"));

            AddStaticBind(
                std::make_unique<PixelShader>(gfx, L"FireEffectPS.cso"));

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

        auto texture = Texture(
            gfx, std::ref(Surface::FromFile("Assets/Graphics/red.png")));

        auto geoCbuf = GeometryCbuf(gfx, *this, camera);
        geoCbuf.SetTextureWidth(texture.GetTextureWidth());
        geoCbuf.SetTextureHeight(texture.GetTextureHeight());

        AddBind(std::make_unique<TransformCbuf>(gfx, *this));

        AddBind(std::make_unique<GeometryCbuf>(geoCbuf));

        AddBind(std::make_unique<TimerCbuf>(gfx, *this));

        AddBind(std::make_unique<Blender>(gfx, true));
    }

    void Update(float dt) noexcept {}

    DirectX::XMMATRIX GetTransformXM() const noexcept {
        return DirectX::XMMatrixScaling(scale, scale, scale) *
               DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
    }

  public:
    float scale = 0.1f;
    DirectX::XMFLOAT4 pos = {0.0f, 0.0f, 0.0f, 0.0f};
};
