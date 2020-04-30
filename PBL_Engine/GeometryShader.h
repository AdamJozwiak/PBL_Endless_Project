#pragma once
#include "Bindable.h"

class GeometryShader : public Bindable {
  public:
    GeometryShader(Graphics& gfx, const std::wstring& path);
    void Bind(Graphics& gfx) noexcept override;
    ID3DBlob* GetBytecode() const noexcept;

  protected:
    Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> pGeometryShader;
};