#include "Engine.hpp"

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    try {
        return Engine{}.run();

    } catch (ExceptionHandler const& exception) {
        MessageBox(nullptr, exception.what(), exception.GetType(),
                   MB_OK | MB_ICONEXCLAMATION);
    } catch (std::exception const& exception) {
        MessageBox(nullptr, exception.what(), "Standard Exception",
                   MB_OK | MB_ICONEXCLAMATION);
    } catch (...) {
        MessageBox(nullptr, "No details available", "Unknown Exception",
                   MB_OK | MB_ICONEXCLAMATION);
    }

    return -1;
}