#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include <functional>

#include "Bindable.h"
#include "BindableBase.h"
#include "DxgiInfoManager.h"
#include "Graphics.h"
#include "GraphicsThrowMacros.h"
#include "RenderableBase.h"

class PostProcessing : public Bindable, public RenderableBase<PostProcessing> {
  public:
    PostProcessing(Graphics& gfx, std::wstring shaderName);
    ~PostProcessing() = default;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OutputTexture() const;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView() const;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView() const;
    void Begin();
    void End();
    void Bind(Graphics& gfx) noexcept override;

  private:
    Graphics& pGfx;
    DxgiInfoManager infoManager;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pOutputTexture;

    DirectX::XMFLOAT3 pos = {1.0f, 1.0f, 1.0f};
    float scale = 1.0f;

  private:
    void Update(float dt) noexcept override;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
};