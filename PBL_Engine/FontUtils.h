#pragma once

#include <d2d1_2.h>
#include <dwrite_2.h>

#include <memory>

template <typename InterfaceType>
inline void SafeRelease(InterfaceType** currentObject) {
    if (*currentObject != NULL) {
        (*currentObject)->Release();
        *currentObject = NULL;
    }
}

// Acquires an additional reference, if non-null.
template <typename InterfaceType>
inline InterfaceType* SafeAcquire(InterfaceType* newObject) {
    if (newObject != NULL) newObject->AddRef();

    return newObject;
}

// Sets a new COM object, releasing the old one.
template <typename InterfaceType>
inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject) {
    SafeAcquire(newObject);
    SafeRelease(&currentObject);
    currentObject = newObject;
}

// Maps exceptions to equivalent HRESULTs,
inline HRESULT ExceptionToHResult() throw() {
    try {
        throw;  // Rethrow previous exception.
    } catch (std::bad_alloc&) {
        return E_OUTOFMEMORY;
    } catch (...) {
        return E_FAIL;
    }
}
