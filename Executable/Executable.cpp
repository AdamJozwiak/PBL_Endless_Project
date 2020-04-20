// ///////////////////////////////////////////////////////////////// Includes //
#include <string>

#include "Engine.hpp"

// ////////////////////////////////////////////////////////////// DLL loading //
std::shared_ptr<Engine> loadEngineFromDll(std::wstring path) {
    HINSTANCE dll = LoadLibrary(path.c_str());
    assert(dll && "Engine's .dll must be loaded correctly!");

    using ScriptCreationPtr = void(__cdecl*)(std::shared_ptr<Engine>&);
    ScriptCreationPtr create =
        reinterpret_cast<ScriptCreationPtr>(GetProcAddress(dll, "create"));
    assert(create &&
           "Engine's .dll factory function must be defined correctly!");

    std::shared_ptr<Engine> engine;
    create(engine);
    return engine;
}

// ///////////////////////////////////////////////////////////////////// Main //
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    try {
        auto engine = loadEngineFromDll(L"PBL_Engine.dll");
        return engine->run();

    } catch (ExceptionHandler const& exception) {
        MessageBoxA(nullptr, exception.what(), exception.GetType(),
                    MB_OK | MB_ICONEXCLAMATION);
    } catch (std::exception const& exception) {
        MessageBoxA(nullptr, exception.what(), "Standard Exception",
                    MB_OK | MB_ICONEXCLAMATION);
    } catch (...) {
        MessageBoxA(nullptr, "No details available", "Unknown Exception",
                    MB_OK | MB_ICONEXCLAMATION);
    }

    return -1;
}

// ////////////////////////////////////////////////////////////////////////// //
