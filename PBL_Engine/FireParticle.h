#pragma once
#include "BindableBase.h"
#include "Camera.h"
#include "EngineAPI.hpp"
#include "GraphicsThrowMacros.h"
#include "RenderableBase.h"
#include "Sampler.h"
#include "Surface.h"

enum EnemyType { none = 0, pawn, rook, bishop };

class ENGINE_API FireParticle : public RenderableBase<FireParticle> {
  public:
    FireParticle(Graphics& gfx, Camera* camera, EnemyType enemyType = none) {
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

            AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

            AddStaticBind(std::make_unique<Sampler>(gfx));

            auto pvs = std::make_unique<VertexShader>(gfx, L"FireEffectVS.cso");
            auto pvsbc = pvs->GetBytecode();
            AddStaticBind(std::move(pvs));

            AddStaticBind(
                std::make_unique<GeometryShader>(gfx, L"FireEffectGS.cso"));

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

            AddStaticBind(std::make_unique<Blender>(gfx, true));
        } else {
            SetIndexFromStatic();
        }

        if (enemyType == none) {
            AddBind(std::make_unique<PixelShader>(gfx, L"FireEffectPS.cso"));

            AddBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile(assetsPath + "fire-albedo.png")),
                0));
            AddBind(std::make_unique<Texture>(
                gfx, std::ref(Surface::FromFile(assetsPath + "fire-noise.png")),
                1));
            AddBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile(assetsPath + "fire-gradient.png")),
                2));
            AddBind(std::make_unique<Texture>(
                gfx, std::ref(Surface::FromFile(assetsPath + "fire-mask.png")),
                3));
        } else {
            scale = 0.08f;

            AddBind(std::make_unique<PixelShader>(gfx, L"EnemyPS.cso"));

            AddBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile(assetsPath + "enemy-noise.png")),
                1));
            AddBind(std::make_unique<Texture>(
                gfx,
                std::ref(Surface::FromFile(assetsPath + "fire-gradient.png")),
                2));
            AddBind(std::make_unique<Texture>(
                gfx, std::ref(Surface::FromFile(assetsPath + "enemy-mask.png")),
                3));

            switch (enemyType) {
                case pawn:
                    AddBind(std::make_unique<Texture>(
                        gfx,
                        std::ref(
                            Surface::FromFile(assetsPath + "enemy-albedo.png")),
                        0));
                    break;
                case rook:
                    AddBind(std::make_unique<Texture>(
                        gfx,
                        std::ref(Surface::FromFile(assetsPath +
                                                   "enemy-albedo1.png")),
                        0));
                    break;
                case bishop:
                    AddBind(std::make_unique<Texture>(
                        gfx,
                        std::ref(Surface::FromFile(assetsPath +
                                                   "enemy-albedo2.png")),
                        0));
                    break;
            }
        }

        auto texture =
            Texture(gfx, std::ref(Surface::FromFile(assetsPath + "red.png")));

        auto geoCbuf = GeometryCbuf(gfx, *this, camera);
        geoCbuf.SetTextureWidth(texture.GetTextureWidth());
        geoCbuf.SetTextureHeight(texture.GetTextureHeight());

        AddBind(std::make_unique<TransformCbuf>(gfx, *this));

        AddBind(std::make_unique<GeometryCbuf>(geoCbuf));

        AddBind(std::make_unique<TimerCbuf>(gfx, *this));
    }

    void Update(float dt) noexcept {}

    DirectX::XMMATRIX GetTransformXM() const noexcept {
        return DirectX::XMMatrixScaling(scale, scale, scale) *
               DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
    }

  public:
    float scale = 0.05f;
    DirectX::XMFLOAT4 pos = {0.0f, 0.0f, 0.0f, 0.0f};

  private:
    const std::string assetsPath = "Assets/Graphics/";
};
