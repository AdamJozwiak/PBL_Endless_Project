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
    PostProcessing(Graphics& gfx, std::wstring shaderName,
                   size_t numberOfTargets);
    ~PostProcessing() = default;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OutputTexture() const;
    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>
    RenderTargetView() const;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView() const;
    void Begin();
    void End();
    void Bind(Graphics& gfx) noexcept override;
    void SetBlackProportion(float black);
    float GetBlackProportion();

  private:
    Graphics& pGfx;
    DxgiInfoManager infoManager;
    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> pRenderTargets;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pOutputTexture;
    std::shared_ptr<PostProcessCbuf> pCbuf;

    DirectX::XMFLOAT3 pos = {1.0f, 1.0f, 1.0f};
    float scale = 1.0f;

  private:
    void Update(float dt) noexcept override;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
};
