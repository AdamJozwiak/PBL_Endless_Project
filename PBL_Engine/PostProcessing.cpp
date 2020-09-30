#include "PostProcessing.h"

#include <dxgiformat.h>

#include <array>

#include "BindableBase.h"
#include "Sampler.h"
#include "Surface.h"

namespace WRL = Microsoft::WRL;

PostProcessing::PostProcessing(Graphics& gfx, std::wstring shaderName,
                               size_t numberOfTargets, bool shadowPass)
    : pGfx(gfx),
      pRenderTargets(numberOfTargets, nullptr),
      pDSV(nullptr),
      pOutputTexture(nullptr) {
    namespace dx = DirectX;
    HRESULT hr;
    int a = 0;

    std::vector<unsigned short> indices;

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    if (!shadowPass) {
        // Output Texture
        for (size_t i = 0; i < pRenderTargets.size(); i++) {
            D3D11_TEXTURE2D_DESC outputTextureDesc;
            ZeroMemory(&outputTextureDesc, sizeof(outputTextureDesc));

            outputTextureDesc.Width = gfx.GetWindowWidth();
            outputTextureDesc.Height = gfx.GetWindowHeight();
            outputTextureDesc.MipLevels = 1;
            outputTextureDesc.ArraySize = 1;
            outputTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
            outputTextureDesc.SampleDesc.Count = 1;
            outputTextureDesc.SampleDesc.Quality = 0;
            outputTextureDesc.BindFlags =
                D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            outputTextureDesc.Usage = D3D11_USAGE_DEFAULT;

            WRL::ComPtr<ID3D11Texture2D> screenTexture = nullptr;

            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
            shaderResourceViewDesc.Format = outputTextureDesc.Format;
            shaderResourceViewDesc.ViewDimension =
                D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
                &outputTextureDesc, nullptr, screenTexture.GetAddressOf()));
            GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
                screenTexture.Get(), &shaderResourceViewDesc,
                pOutputTexture.GetAddressOf()));

            // Depth Stencil
            D3D11_TEXTURE2D_DESC depthStencilDesc;
            ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
            depthStencilDesc.Width = gfx.GetWindowWidth();
            depthStencilDesc.Height = gfx.GetWindowHeight();
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

            WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = nullptr;
            GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
                &depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf()));
            GFX_THROW_INFO(GetDevice(gfx)->CreateDepthStencilView(
                depthStencilBuffer.Get(), nullptr, pDSV.GetAddressOf()));

            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
            ZeroMemory(&renderTargetViewDesc,
                       sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
            renderTargetViewDesc.Format = outputTextureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
                screenTexture.Get(), &renderTargetViewDesc,
                pRenderTargets[i].GetAddressOf()));

            AddBind(std::make_unique<Texture>(gfx, pOutputTexture,
                                              gfx.GetWindowWidth(),
                                              gfx.GetWindowHeight(), a++));

            AddBind(std::make_unique<Texture>(
                gfx, std::ref(Surface::FromFile("Assets/Graphics/map.png")),
                20));

            struct Vertex {
                dx::XMFLOAT3 pos;
                dx::XMFLOAT2 tex;
            };

            std::vector<Vertex> screenVertices = {
                {dx::XMFLOAT3(-1.0f, -1.0f, 0.0f), dx::XMFLOAT2(0.0f, 1.0f)},
                {dx::XMFLOAT3(-1.0f, 1.0f, 0.0f), dx::XMFLOAT2(0.0f, 0.0f)},
                {dx::XMFLOAT3(1.0f, 1.0f, 0.0f), dx::XMFLOAT2(1.0f, 0.0f)},
                {dx::XMFLOAT3(1.0f, -1.0f, 0.0f), dx::XMFLOAT2(1.0f, 1.0f)}};

            AddBind(std::make_unique<VertexBuffer>(pGfx, screenVertices));

            auto pvs =
                std::make_unique<VertexShader>(pGfx, shaderName + L"VS.cso");
            auto pvsbc = pvs->GetBytecode();
            AddBind(std::move(pvs));

            AddBind(
                std::make_unique<GeometryShader>(pGfx, shaderName + L"GS.cso"));

            AddBind(
                std::make_unique<PixelShader>(pGfx, shaderName + L"PS.cso"));

            const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
                {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
                 D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            AddBind(std::make_unique<InputLayout>(pGfx, ied, pvsbc));

            AddBind(std::make_unique<Topology>(
                pGfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

            AddBind(std::make_unique<Sampler>(pGfx));

            pCbuf = std::make_shared<PostProcessCbuf>(gfx, *this, 1.0f, 10u);

            AddBind(pCbuf);
        }

        AddIndexBuffer(std::make_unique<IndexBuffer>(pGfx, indices));

    } else {
        static const int CubeMapSize = 512;

        // ------------------- CubeMap Texture ----------------------- //
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = CubeMapSize;
        texDesc.Height = CubeMapSize;
        texDesc.MipLevels = 0;
        texDesc.ArraySize = 6;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format = DXGI_FORMAT_R32_FLOAT;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags =
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        ID3D11Texture2D* cubeTex = 0;
        GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&texDesc, 0, &cubeTex));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.ArraySize = 1;

        for (int i = 0; i < 6; ++i) {
            rtvDesc.Texture2DArray.FirstArraySlice = i;
            GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
                cubeTex, &rtvDesc, pRenderTargets[i].GetAddressOf()));
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = -1;

        GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
            cubeTex, &srvDesc, pOutputTexture.GetAddressOf()));

        // ----------------- CubeMap Depth Stencil ------------------- //
        D3D11_TEXTURE2D_DESC depthTexDesc;
        depthTexDesc.Width = CubeMapSize;
        depthTexDesc.Height = CubeMapSize;
        depthTexDesc.MipLevels = 1;
        depthTexDesc.ArraySize = 1;
        depthTexDesc.SampleDesc.Count = 1;
        depthTexDesc.SampleDesc.Quality = 0;
        depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
        depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthTexDesc.CPUAccessFlags = 0;
        depthTexDesc.MiscFlags = 0;

        ID3D11Texture2D* depthTex = 0;
        GFX_THROW_INFO(
            GetDevice(gfx)->CreateTexture2D(&depthTexDesc, 0, &depthTex));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = depthTexDesc.Format;
        dsvDesc.Flags = 0;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        GFX_THROW_INFO(GetDevice(gfx)->CreateDepthStencilView(
            depthTex, &dsvDesc, pDSV.GetAddressOf()));

        shadowMap =
            std::make_shared<Texture>(gfx, pOutputTexture, gfx.GetWindowWidth(),
                                      gfx.GetWindowHeight(), 11);
    }
}

std::vector<WRL::ComPtr<ID3D11RenderTargetView>>
PostProcessing::RenderTargetView() const {
    return pRenderTargets;
}

WRL::ComPtr<ID3D11DepthStencilView> PostProcessing::DepthStencilView() const {
    return pDSV;
}

WRL::ComPtr<ID3D11ShaderResourceView> PostProcessing::OutputTexture() const {
    return pOutputTexture;
}

void PostProcessing::Begin() {
    std::array<ID3D11RenderTargetView*, 2> addresses;
    for (size_t i = 0; i < pRenderTargets.size(); i++) {
        addresses[i] = pRenderTargets[i].Get();
    }
    GetContext(pGfx)->OMSetRenderTargets(pRenderTargets.size(),
                                         addresses.data(), pDSV.Get());

    const float color[] = {0.0f, 0.0f, 0.0f, 1.0f};
    for (auto const& target : pRenderTargets) {
        GetContext(pGfx)->ClearRenderTargetView(target.Get(), color);
    }

    GetContext(pGfx)->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f,
                                            0u);
}

void PostProcessing::ShadowBegin(int iterator) {
    GetContext(pGfx)->OMSetRenderTargets(
        1u, pRenderTargets[iterator].GetAddressOf(), pDSV.Get());

    const float color[] = {0.0f, 0.0f, 0.0f, 1.0f};

    GetContext(pGfx)->ClearRenderTargetView(pRenderTargets[iterator].Get(),
                                            color);
    GetContext(pGfx)->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f,
                                            0u);
}

void PostProcessing::End() {
    pGfx.SetDefaultRenderTarget(pGfx.GetWindowWidth(), pGfx.GetWindowHeight());
}

void PostProcessing::Bind(Graphics& gfx) noexcept {}

void PostProcessing::SetBlackProportion(float black) {
    pCbuf->SetBlackProportion(black);
}
float PostProcessing::GetBlackProportion() {
    return pCbuf->GetBlackProportion();
}

void PostProcessing::Update(float dt) noexcept {}

DirectX::XMMATRIX PostProcessing::GetTransformXM() const noexcept {
    return DirectX::XMMatrixScaling(scale, scale, scale) *
           DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}
