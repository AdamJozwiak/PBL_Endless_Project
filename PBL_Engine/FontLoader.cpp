#include "FontLoader.h"

IDWriteFontCollectionLoader* FontLoader::instance_(new (std::nothrow)
                                                       FontLoader());

HRESULT STDMETHODCALLTYPE FontLoader::QueryInterface(REFIID iid,
                                                     void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE FontLoader::AddRef() {
    return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE FontLoader::Release() {
    ULONG newCount = InterlockedDecrement(&refCount_);
    if (newCount == 0) delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE FontLoader::CreateEnumeratorFromKey(
    IDWriteFactory* factory,
    void const* collectionKey,  // [collectionKeySize] in bytes
    UINT32 collectionKeySize,
    OUT IDWriteFontFileEnumerator** fontFileEnumerator) {
    *fontFileEnumerator = NULL;
    HRESULT hr = S_OK;

    if (collectionKeySize % sizeof(UINT) != 0) return E_INVALIDARG;

    FontFileEnumerator* enumerator =
        new (std::nothrow) FontFileEnumerator(factory);
    if (enumerator == NULL) return E_OUTOFMEMORY;

    UINT const* mfCollectionKey = static_cast<UINT const*>(collectionKey);
    UINT32 const mfKeySize = collectionKeySize;

    hr = enumerator->Initialize(mfCollectionKey, mfKeySize);

    if (FAILED(hr)) {
        delete enumerator;
        return hr;
    }

    *fontFileEnumerator = SafeAcquire(enumerator);

    return hr;
}

// ------------------------------ MFFontFileEnumerator
// ----------------------------------------------------------

FontFileEnumerator::FontFileEnumerator(IDWriteFactory* factory)
    : refCount_(0),
      factory(SafeAcquire(factory)),
      currentFile(),
      nextIndex(0) {}

HRESULT FontFileEnumerator::Initialize(
    UINT const* collectionKey,  // [resourceCount]
    UINT32 keySize) {
    try {
        // dereference collectionKey in order to get index of collection in
        // MFFontGlobals::fontCollections vector
        UINT cPos = *collectionKey;
        for (collection::iterator it = FontGlobals::collections()[cPos].begin();
             it != FontGlobals::collections()[cPos].end(); ++it) {
            filePaths.push_back(it->c_str());
        }
    } catch (...) {
        return ExceptionToHResult();
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FontFileEnumerator::QueryInterface(REFIID iid,
                                                             void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE FontFileEnumerator::AddRef() {
    return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE FontFileEnumerator::Release() {
    ULONG newCount = InterlockedDecrement(&refCount_);
    if (newCount == 0) delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE
FontFileEnumerator::MoveNext(OUT BOOL* hasCurrentFile) {
    HRESULT hr = S_OK;

    *hasCurrentFile = FALSE;
    SafeRelease(&currentFile);

    if (nextIndex < filePaths.size()) {
        hr = factory->CreateFontFileReference(filePaths[nextIndex].c_str(),
                                              NULL, &currentFile);

        if (SUCCEEDED(hr)) {
            *hasCurrentFile = TRUE;

            ++nextIndex;
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE
FontFileEnumerator::GetCurrentFontFile(OUT IDWriteFontFile** fontFile) {
    *fontFile = SafeAcquire(currentFile);

    return (currentFile != NULL) ? S_OK : E_FAIL;
}

// ---------------------------------------- MFFontContext
// ---------------------------------------------------------

FontContext::FontContext(IDWriteFactory* pFactory)
    : hr_(S_FALSE), g_dwriteFactory(pFactory) {}

FontContext::~FontContext() {
    g_dwriteFactory->UnregisterFontCollectionLoader(FontLoader::GetLoader());
}

HRESULT FontContext::Initialize() {
    if (hr_ == S_FALSE) {
        hr_ = InitializeInternal();
    }
    return hr_;
}

HRESULT FontContext::InitializeInternal() {
    HRESULT hr = S_OK;

    if (!FontLoader::IsLoaderInitialized()) {
        return E_FAIL;
    }

    // Register our custom loader with the factory object.
    hr = g_dwriteFactory->RegisterFontCollectionLoader(FontLoader::GetLoader());

    return hr;
}

HRESULT FontContext::CreateFontCollection(collection& newCollection,
                                          OUT IDWriteFontCollection** result) {
    *result = NULL;

    HRESULT hr = S_OK;

    // save new collection in MFFontGlobals::fontCollections vector
    UINT collectionKey = FontGlobals::push(newCollection);
    cKeys.push_back(collectionKey);
    const void* fontCollectionKey = &cKeys.back();
    UINT32 keySize = sizeof(collectionKey);

    hr = Initialize();
    if (FAILED(hr)) return hr;

    hr = g_dwriteFactory->CreateCustomFontCollection(
        FontLoader::GetLoader(), fontCollectionKey, keySize, result);

    return hr;
}

std::vector<unsigned int> FontContext::cKeys = std::vector<unsigned int>(0);

// ----------------------------------- MFFontGlobals
// ---------------------------------------------------------

std::vector<collection> FontGlobals::fontCollections =
    std::vector<collection>(0);