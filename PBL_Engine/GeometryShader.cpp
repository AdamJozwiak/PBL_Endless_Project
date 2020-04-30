#include "GraphicsThrowMacros.h"
#include "GeometryShader.h"

GeometryShader::GeometryShader(Graphics& gfx, const std::wstring& path) {
    INFOMAN(gfx);

    GFX_THROW_INFO(D3DReadFileToBlob(path.c_str(), &pBytecodeBlob));
    GFX_THROW_INFO(GetDevice(gfx)->CreateGeometryShader(
        pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(),
        nullptr, &pGeometryShader));
}

void GeometryShader::Bind(Graphics& gfx) noexcept {
    GetContext(gfx)->GSSetShader(pGeometryShader.Get(), nullptr, 0u);
}

ID3DBlob* GeometryShader::GetBytecode() const noexcept {
    return pBytecodeBlob.Get();
}