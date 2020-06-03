#include "Text.h"

Text::Text(Graphics& gfx, const WCHAR* fontFamily, float fontSize) {
    CreateDevice(gfx);
    CreateBitmapRenderTarget(gfx);
    InitializeTextFormats(fontFamily, fontSize);
}

Text::Text(Graphics& gfx, const WCHAR* fontFamily, std::wstring path,
           float fontSize) {
    CreateDevice(gfx);
    CreateBitmapRenderTarget(gfx);
    InitCustomTextFormat(fontFamily, path, fontSize);
}

void Text::RenderText(Graphics& gfx, std::string text) {
    std::wstring tmp = std::wstring(text.begin(), text.end());
    writeFactory->CreateTextLayout(tmp.c_str(), (UINT32)tmp.size(),
                                   textFormatFPS.Get(), gfx.GetWindowWidth(),
                                   gfx.GetWindowHeight(), &textLayoutFPS);
    devCon->BeginDraw();

    devCon->DrawTextLayout(
        D2D1::Point2F(2.0f, 5.0f),  // origin / position of first letter
        textLayoutFPS.Get(),        // text layout
        yellowBrush.Get()           // brush
    );

    devCon->EndDraw();
}

void Text::CreateDevice(Graphics& gfx) {
    // Create DirectWrite factory
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                        &writeFactory);

    // Create Direct2D factory
    D2D1_FACTORY_OPTIONS options;
#ifndef NDEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                                 __uuidof(ID2D1Factory1), &options,
                                 &factory))) {
        throw TextException(
            __LINE__, __FILE__,
            "Critical error: Unable to create Direct2D Factory!");
    }

    // Get DXGI device
    wrl::ComPtr<IDXGIDevice> dxgiDevice;
    if (FAILED(gfx.GetDevice()->QueryInterface(__uuidof(IDXGIDevice),
                                               &dxgiDevice))) {
        throw TextException(__LINE__, __FILE__,
                            "Critical error: Unable to get the DXGI device!");
    }

    // Create Direct2D device
    if (FAILED(factory->CreateDevice(dxgiDevice.Get(), &dev))) {
        throw TextException(
            __LINE__, __FILE__,
            "Critical error: Unable to create the Direct2D device!");
    }

    // Create Direct2D device context
    if (FAILED(dev->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
            &devCon)))
        throw TextException(
            __LINE__, __FILE__,
            "Critical error: Unable to create the Direct2D device context!");
}

void Text::CreateBitmapRenderTarget(Graphics& gfx) {
    // Set bitmap properties
    D2D1_BITMAP_PROPERTIES1 bp;
    bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    bp.dpiX = 96.0f;
    bp.dpiY = 96.0f;
    bp.bitmapOptions =
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    bp.colorContext = nullptr;

    // Get Direct3D backbuffer
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiBuffer;
    gfx.GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBuffer);

    // Create the bitmap
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    devCon->CreateBitmapFromDxgiSurface(dxgiBuffer.Get(), &bp, &targetBitmap);

    // Set this bitmap as the render target
    devCon->SetTarget(targetBitmap.Get());
}

void Text::InitializeTextFormats(const WCHAR* fontFamily, float fontSize) {
    // Create brushes
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow),
                                  &yellowBrush);
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
                                  &blackBrush);
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                  &whiteBrush);

    // Set text parameters

    // Text format
    writeFactory.Get()->CreateTextFormat(
        fontFamily,                  // font family
        nullptr,                     // font collection
        DWRITE_FONT_WEIGHT_LIGHT,    // font weight
        DWRITE_FONT_STYLE_NORMAL,    // font style
        DWRITE_FONT_STRETCH_NORMAL,  // font stretch
        fontSize,                    // font size (in dp)
        L"en-GB",                    // locale
        &textFormatFPS               // [out] text format object
    );

    // Text alignment
    textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    // Paragraph alignment
    textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

void Text::InitCustomTextFormat(const WCHAR* fontFamily, std::wstring filePath,
                                float fontSize) {
    // Create brushes
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow),
                                  &yellowBrush);
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
                                  &blackBrush);
    devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                  &whiteBrush);

    FontContext fContext(writeFactory.Get());
    std::vector<std::wstring> filePaths;
    filePaths.push_back(filePath);
    // Set text parameters
    HRESULT hr = fContext.CreateFontCollection(filePaths, &fontCollection);

    // Text format
    writeFactory.Get()->CreateTextFormat(
        fontFamily,      // font family
        fontCollection.Get(),        // font collection
        DWRITE_FONT_WEIGHT_REGULAR,  // font weight
        DWRITE_FONT_STYLE_NORMAL,    // font style
        DWRITE_FONT_STRETCH_NORMAL,  // font stretch
        fontSize,                    // font size (in dp)
        L"en-GB",                    // locale
        &textFormatFPS               // [out] text format object
    );

    // Text alignment
    textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    // Paragraph alignment
    textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

TextException::TextException(int line, const char* file,
                             std::string note) noexcept
    : ExceptionHandler(line, file), note(std::move(note)) {}

const char* TextException::what() const noexcept {
    std::ostringstream oss;
    oss << ExceptionHandler::what() << std::endl << "[Note] " << GetNote();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* TextException::GetType() const noexcept {
    return "Model Exception";
}

const std::string& TextException::GetNote() const noexcept { return note; }
