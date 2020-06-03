#pragma once
#include <d2d1_2.h>
#include <dwrite_2.h>

#include <string>
#include <vector>

#include "FontUtils.h"

typedef std::vector<std::wstring> collection;

class FontLoader : public IDWriteFontCollectionLoader {
  public:
    FontLoader() : refCount_(0) {}

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                     void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontCollectionLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,  // [collectionKeySize] in bytes
        UINT32 collectionKeySize,
        OUT IDWriteFontFileEnumerator** fontFileEnumerator);
    // Gets the singleton loader instance.
    static IDWriteFontCollectionLoader* GetLoader() { return instance_; }

    static bool IsLoaderInitialized() { return instance_ != NULL; }

  private:
    ULONG refCount_;

    static IDWriteFontCollectionLoader* instance_;
};

class FontFileEnumerator : public IDWriteFontFileEnumerator {
  public:
    FontFileEnumerator(IDWriteFactory* factory);
    ~FontFileEnumerator() {
        SafeRelease(&currentFile);
        SafeRelease(&factory);
    }
    HRESULT Initialize(UINT const* collectionKey,  // [resourceCount]
                       UINT32 keySize);

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                     void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileEnumerator methods
    virtual HRESULT STDMETHODCALLTYPE MoveNext(OUT BOOL* hasCurrentFile);
    virtual HRESULT STDMETHODCALLTYPE
    GetCurrentFontFile(OUT IDWriteFontFile** fontFile);

  private:
    ULONG refCount_;

    IDWriteFactory* factory;
    IDWriteFontFile* currentFile;
    std::vector<std::wstring> filePaths;
    size_t nextIndex;
};

class FontContext {
  public:
    FontContext(IDWriteFactory* pFactory);
    ~FontContext();

    HRESULT Initialize();

    HRESULT CreateFontCollection(collection& newCollection,
                                 OUT IDWriteFontCollection** result);

  private:
    // Not copyable or assignable.
    FontContext(FontContext const&);
    void operator=(FontContext const&);

    HRESULT InitializeInternal();
    IDWriteFactory* g_dwriteFactory;
    static std::vector<unsigned int> cKeys;

    // Error code from Initialize().
    HRESULT hr_;
};

class FontGlobals {
  public:
    FontGlobals() {}
    static unsigned int push(collection& addCollection) {
        unsigned int ret = fontCollections.size();
        fontCollections.push_back(addCollection);
        return ret;
    }
    static std::vector<collection>& collections() { return fontCollections; }

  private:
    static std::vector<collection> fontCollections;
};