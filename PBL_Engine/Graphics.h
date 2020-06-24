#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include <memory>
#include <random>
#include <sstream>
#include <vector>

#include "DxgiInfoManager.h"
#include "ExceptionHandler.h"
#include "IsDebug.hpp"
#include "WinHeader.h"

class Graphics {
    friend class Bindable;

  public:
    class Exception : public ExceptionHandler {
        using ExceptionHandler::ExceptionHandler;
    };
    class HrException : public Exception {
      public:
        HrException(int line, const char* file, HRESULT hr,
                    std::vector<std::string> infoMsgs = {}) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;
        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorString() const noexcept;
        std::string GetErrorDescription() const noexcept;
        std::string GetErrorInfo() const noexcept;

      private:
        HRESULT hr;
        std::string info;
    };
    class DeviceRemovedException : public HrException {
        using HrException::HrException;

      public:
        const char* GetType() const noexcept override;

      private:
        std::string reason;
    };
    class InfoException : public Exception {
      public:
        InfoException(int line, const char* file,
                      std::vector<std::string> infoMsgs) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;
        std::string GetErrorInfo() const noexcept;

      private:
        std::string info;
    };

  public:
    Graphics(HWND hWnd, int width, int height);
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    ~Graphics() = default;
    void SetViewport(int width, int height);
    void SetDefaultViewport();
    void SetDefaultRenderTarget(int width, int height);
    void EndFrame();
    void BeginFrame(float red, float green, float blue,
                    bool imguiRender = true) noexcept;
    void DrawIndexed(UINT count) noexcept(!IS_DEBUG);
    void SetProjection(DirectX::FXMMATRIX proj) noexcept;
    DirectX::XMMATRIX GetProjection() const noexcept;
    void SetCamera(DirectX::FXMMATRIX cam) noexcept;
    DirectX::XMMATRIX GetCamera() const noexcept;
    void EnableImgui() noexcept;
    void DisableImgui() noexcept;
    bool IsImguiEnabled() const noexcept;
    int GetWindowWidth() const noexcept;
    int GetWindowHeight() const noexcept;
    IDXGISwapChain* GetSwapChain();
    ID3D11Device* GetDevice();

  private:
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX camera;
    int windowWidth;
    int windowHeight;
    bool imguiEnabled = true;
#ifndef NDEBUG  // If in Debug mode this will be included
    DxgiInfoManager infoManager;
#endif  // !NDEBUG
    Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
};
