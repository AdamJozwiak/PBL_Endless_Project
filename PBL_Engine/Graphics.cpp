#include "Graphics.h"

#include "GraphicsThrowMacros.h"
#include "dxerr.h"
#include "imgui/examples/imgui_impl_dx11.h"
#include "imgui/examples/imgui_impl_win32.h"

namespace WRL = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

Graphics::Graphics(HWND hWnd, int width, int height)
    : windowWidth(width), windowHeight(height) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;  // No Anti-Aliasing
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hWnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT swapCreateFlags = 0u;
#ifndef NDEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;  // D3D11_CREATE_DEVICE_DEBUG,
                                                   // to debug in console
#endif

    // for checking results
    HRESULT hr;

    // create device, front/back buffers, swap chain and rendering context
    GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, swapCreateFlags, nullptr, 0,
        D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, nullptr, &pContext));

    WRL::ComPtr<ID3D11Resource> pBackBuffer;
    GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
    GFX_THROW_INFO(
        pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));

    // create depth stensil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    WRL::ComPtr<ID3D11DepthStencilState> pDSState;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

    // bind depth state
    pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

    // create depth stencil texture
    WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;  // Same size as Swap Chain
    descDepth.Height = height;
    descDepth.MipLevels = 1u;
    descDepth.ArraySize = 1u;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;  // D32 - Depth
    descDepth.SampleDesc.Count = 1u;           // Anti-Aliasing
    descDepth.SampleDesc.Quality = 0u;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    GFX_THROW_INFO(
        pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

    // create view of depth stencil texture
    CD3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    GFX_THROW_INFO(
        pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV));

    SetDefaultRenderTarget(width, height);

    // configure viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    pContext->RSSetViewports(1u, &vp);

    ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
}

void Graphics::SetDefaultRenderTarget(int width, int height) {
    // bind depth stensil view to OM
    pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());
}

void Graphics::EndFrame() {
    // imgui frame end
    if (imguiEnabled) {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set();
#endif
    if (FAILED(hr = pSwap->Present(1u, 0u)))  // 60 fps, 30 would be 2u
    {
        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
        } else {
            throw GFX_EXCEPT(hr);
        }
    }
}

void Graphics::BeginFrame(float red, float green, float blue,
                          bool imguiRender) noexcept {
    // imgui begin frame
    if (imguiEnabled && imguiRender) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    const float color[] = {red, green, blue, 1.0f};
    pContext->ClearRenderTargetView(pTarget.Get(), color);
    pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawIndexed(UINT count) noexcept(!IS_DEBUG) {
    if (count > 1) {
        GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));
    } else {
        pContext->DrawIndexed(count, 0u, 0u);
    }
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept {
    projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept {
    return projection;
}

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept { camera = cam; }

DirectX::XMMATRIX Graphics::GetCamera() const noexcept { return camera; }

void Graphics::EnableImgui() noexcept { imguiEnabled = true; }

void Graphics::DisableImgui() noexcept { imguiEnabled = false; }

bool Graphics::IsImguiEnabled() const noexcept { return imguiEnabled; }

int Graphics::GetWindowWidth() const noexcept { return windowWidth; }

int Graphics::GetWindowHeight() const noexcept { return windowHeight; }

// ------------------------ Graphics exception ---------------------------- //

Graphics::HrException::HrException(int line, const char* file, HRESULT hr,
                                   std::vector<std::string> infoMsgs) noexcept
    : Exception(line, file), hr(hr) {
    // join all info messages with newlines into single string
    for (const auto& m : infoMsgs) {
        info += m;
        info.push_back('\n');
    }
    // remove final newline if exists
    if (!info.empty()) {
        info.pop_back();
    }
}

const char* Graphics::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
        << "[Error String] " << GetErrorString() << std::endl
        << "[Description] " << GetErrorDescription() << std::endl;
    if (!info.empty()) {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept {
    return "Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept { return hr; }

std::string Graphics::HrException::GetErrorString() const noexcept {
    return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept {
    char buf[512];
    DXGetErrorDescription(hr, buf, sizeof(buf));
    return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept {
    return info;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept {
    return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException(
    int line, const char* file, std::vector<std::string> infoMsgs) noexcept
    : Exception(line, file) {
    // join all info messages with newlines into single string
    for (const auto& m : infoMsgs) {
        info += m;
        info.push_back('\n');
    }
    // remove final newline if exists
    if (!info.empty()) {
        info.pop_back();
    }
}

const char* Graphics::InfoException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "\n[Error Info]\n"
        << GetErrorInfo() << std::endl
        << std::endl;
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept {
    return "Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept {
    return info;
}
