#pragma once
#include <Surface.h>

#include "Bindable.h"

class Texture : public Bindable {
  public:
    Texture(Graphics& gfx, SurfaceReference s, int number = 0);
    Texture(Graphics& gfx,
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pOutputTexture,
            float width, float height, int number = 0);
    void Bind(Graphics& gfx) noexcept override;
    float GetTextureWidth();
    float GetTextureHeight();

  protected:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
    float textureWidth;
    float textureHeight;
    int number;
    bool hasAlpha = false;
};
