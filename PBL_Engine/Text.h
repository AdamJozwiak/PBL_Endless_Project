#pragma once

#include <d2d1_2.h>
#include <dwrite_2.h>
#include <wrl/client.h>

#include "ExceptionHandler.h"
#include "Graphics.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace wrl = Microsoft::WRL;

class Text {
  public:
    friend class RenderSystem;
    friend class Graphics;
    Text(Graphics& gfx, const WCHAR* fontFamily, float fontSize);
    // Brushes
    inline static wrl::ComPtr<ID2D1SolidColorBrush> yellowBrush;
    inline static wrl::ComPtr<ID2D1SolidColorBrush> whiteBrush;
    inline static wrl::ComPtr<ID2D1SolidColorBrush> blackBrush;

    // Text formats
    wrl::ComPtr<IDWriteTextFormat> textFormatFPS;

    // Text layouts
    wrl::ComPtr<IDWriteTextLayout> textLayoutFPS;
    void RenderText(Graphics& gfx, std::string text);

  private:
    inline static bool init = false;
    static void CreateDevice(
        Graphics& gfx);  // creates the device and its context
    static void CreateBitmapRenderTarget(Graphics& gfx);
    void InitializeTextFormats(const WCHAR* fontFamily, float fontSize);

    inline static wrl::ComPtr<IDWriteFactory1>
        writeFactory;                                  // DirectWrite factory
    inline static wrl::ComPtr<ID2D1Factory1> factory;  // Direct2D factory
    inline static wrl::ComPtr<ID2D1Device> dev;        // Direct2D device
    inline static wrl::ComPtr<ID2D1DeviceContext> devCon;  // device context
};

class TextException : public ExceptionHandler {
  public:
    TextException(int line, const char* file, std::string note) noexcept;
    const char* what() const noexcept override;
    const char* GetType() const noexcept override;
    const std::string& GetNote() const noexcept;

  private:
    std::string note;
};
