#include "PostProcessing.h"

#include "BindableBase.h"
#include "Sampler.h"
#include "Surface.h"

namespace WRL = Microsoft::WRL;

PostProcessing::PostProcessing(Graphics& gfx, std::wstring shaderName)
    : pGfx(gfx),
      pRenderTarget(nullptr),
      pDSV(nullptr),
      pOutputTexture(nullptr) {
    namespace dx = DirectX;
    // Output Texture
    D3D11_TEXTURE2D_DESC outputTextureDesc;
    ZeroMemory(&outputTextureDesc, sizeof(outputTextureDesc));
    outputTextureDesc.Width = gfx.GetWindowWidth();
    outputTextureDesc.Height = gfx.GetWindowHeight();
    outputTextureDesc.MipLevels = 1;
    outputTextureDesc.ArraySize = 1;
    outputTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    outputTextureDesc.SampleDesc.Count = 1;
    outputTextureDesc.SampleDesc.Quality = 0;
    outputTextureDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    outputTextureDesc.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr;
    WRL::ComPtr<ID3D11Texture2D> screenTexture = nullptr;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = outputTextureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
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
    ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    renderTargetViewDesc.Format = outputTextureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
        screenTexture.Get(), &renderTargetViewDesc,
        pRenderTarget.GetAddressOf()));

    struct Vertex {
        dx::XMFLOAT3 pos;
        dx::XMFLOAT2 tex;
    };

    std::vector<Vertex> screenVertices = {
        {dx::XMFLOAT3(-1.0f, -1.0f, 0.0f), dx::XMFLOAT2(0.0f, 1.0f)},
        {dx::XMFLOAT3(-1.0f, 1.0f, 0.0f), dx::XMFLOAT2(0.0f, 0.0f)},
        {dx::XMFLOAT3(1.0f, 1.0f, 0.0f), dx::XMFLOAT2(1.0f, 0.0f)},
        {dx::XMFLOAT3(1.0f, -1.0f, 0.0f), dx::XMFLOAT2(1.0f, 1.0f)}};

    std::vector<unsigned short> indices;

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    AddBind(std::make_unique<Texture>(
        gfx, pOutputTexture, gfx.GetWindowWidth(), gfx.GetWindowHeight()));

    AddBind(std::make_unique<VertexBuffer>(pGfx, screenVertices));

    AddBind(std::make_unique<Sampler>(pGfx));

    auto pvs = std::make_unique<VertexShader>(pGfx, shaderName + L"VS.cso");
    auto pvsbc = pvs->GetBytecode();
    AddBind(std::move(pvs));

    AddBind(
        std::make_unique<GeometryShader>(pGfx, shaderName + L"GS.cso"));

    AddBind(std::make_unique<PixelShader>(pGfx, shaderName + L"PS.cso"));

    AddIndexBuffer(std::make_unique<IndexBuffer>(pGfx, indices));

    const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    AddBind(std::make_unique<InputLayout>(pGfx, ied, pvsbc));

    AddBind(std::make_unique<Topology>(
        pGfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
}

WRL::ComPtr<ID3D11RenderTargetView> PostProcessing::RenderTargetView() const {
    return pRenderTarget;
}

WRL::ComPtr<ID3D11DepthStencilView> PostProcessing::DepthStencilView() const {
    return pDSV;
}

WRL::ComPtr<ID3D11ShaderResourceView> PostProcessing::OutputTexture() const {
    return pOutputTexture;
}

void PostProcessing::Begin() {
    GetContext(pGfx)->OMSetRenderTargets(1, pRenderTarget.GetAddressOf(),
                                         pDSV.Get());

    const float color[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GetContext(pGfx)->ClearRenderTargetView(pRenderTarget.Get(), color);
    GetContext(pGfx)->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f,
                                            0u);
}

void PostProcessing::End() {
    pGfx.SetDefaultRenderTarget(pGfx.GetWindowWidth(), pGfx.GetWindowHeight());
}

void PostProcessing::Bind(Graphics& gfx) noexcept {}

void PostProcessing::Update(float dt) noexcept {}

DirectX::XMMATRIX PostProcessing::GetTransformXM() const noexcept {
    return DirectX::XMMatrixScaling(scale, scale, scale) *
           DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}