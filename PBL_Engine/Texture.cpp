#include "Texture.h"

#include <map>

#include "GraphicsThrowMacros.h"
#include "Surface.h"

namespace wrl = Microsoft::WRL;

std::map<std::string, Texture> existingTextures;

Texture::Texture(Graphics& gfx, SurfaceReference s, int number)
    : number(number) {
    if (existingTextures.contains(s.get().filename)) {
        Texture texture = existingTextures.at(s.get().filename);
        *this = texture;
        return;
    }
    INFOMAN(gfx);
    textureWidth = s.get().GetWidth();
    textureHeight = s.get().GetHeight();

    // create texture resource
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = s.get().GetWidth();
    textureDesc.Height = s.get().GetHeight();
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format =
        DXGI_FORMAT_B8G8R8A8_UNORM;  // same format as the back buffer
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = s.get().GetBufferPtr();
    sd.SysMemPitch =
        s.get().GetWidth() *
        sizeof(Surface::Color);  // distance in bytes between adjacent pixels
    wrl::ComPtr<ID3D11Texture2D> pTexture;
    GFX_THROW_INFO(
        GetDevice(gfx)->CreateTexture2D(&textureDesc, &sd, &pTexture));

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
        pTexture.Get(), &srvDesc, &pTextureView));

    existingTextures.insert({s.get().filename, *this});
}

Texture::Texture(Graphics& gfx, std::vector<SurfaceReference*> s, int number)
    : number(number) {
    INFOMAN(gfx);

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = s.at(0)->get().GetWidth();
    texDesc.Height = s.at(0)->get().GetHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    // Array to fill which we will use to point D3D at our loaded images.
    D3D11_SUBRESOURCE_DATA pData[6];
    for (int cubeMapFaceIndex = 0; cubeMapFaceIndex < 6; cubeMapFaceIndex++) {
        // Pointer to the pixel data
        pData[cubeMapFaceIndex].pSysMem =
            s.at(cubeMapFaceIndex)->get().GetBufferPtr();
        // Line width in bytes
        pData[cubeMapFaceIndex].SysMemPitch =
            s.at(cubeMapFaceIndex)->get().GetWidth() *
            sizeof(
                Surface::Color);  // distance in bytes between adjacent pixels
        pData[cubeMapFaceIndex].SysMemSlicePitch = 0;
    }

    ID3D11Texture2D* cubeTex = 0;
    GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&texDesc, pData, &cubeTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = texDesc.MipLevels;

    GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
        cubeTex, &srvDesc, pTextureView.GetAddressOf()));
}

Texture::Texture(
    Graphics& gfx,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pOutputTexture,
    float width, float height, int number)
    : number(number) {
    pTextureView = pOutputTexture;
    textureWidth = width;
    textureHeight = height;
}

void Texture::Bind(Graphics& gfx) noexcept {
    GetContext(gfx)->PSSetShaderResources(number, 1u,
                                          pTextureView.GetAddressOf());
}

float Texture::GetTextureWidth() { return (float)textureWidth; }

float Texture::GetTextureHeight() { return (float)textureHeight; }
