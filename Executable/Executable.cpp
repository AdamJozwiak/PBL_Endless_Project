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
    return loadEngineFromDll(L"PBL_Engine.dll")->run();
}

// ////////////////////////////////////////////////////////////////////////// //
