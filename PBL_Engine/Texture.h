#pragma once
#include "Bindable.h"

class Texture : public Bindable {
  public:
    Texture(Graphics& gfx, const class Surface& s);
    void Bind(Graphics& gfx) noexcept override;
    void BindAdditional(Graphics& gfx, UINT texRegister);
    float GetTextureWidth();
    float GetTextureHeight();

  protected:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
    float textureWidth;
    float textureHeight;
};