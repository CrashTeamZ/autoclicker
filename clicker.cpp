#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>


std::atomic<bool> autoclicker_running(true);
const int CPS = 11;
const int DELAY_MS = 1000 / CPS;

const int BIND_CLICK = 0x52; 

void SimulateClick() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}


void HoldKey(WORD vkCode, bool hold) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    input.ki.dwFlags = hold ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}


bool IsMinecraftOrVimeActive() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;

    char className[256];
    GetClassNameA(hwnd, className, sizeof(className));
    if (strcmp(className, "LWJGL") == 0 || strcmp(className, "GLFW30") == 0)
        return true;

    char windowTitle[256];
    GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
    std::string title(windowTitle);
    std::transform(title.begin(), title.end(), title.begin(), ::tolower);
    return (title.find("minecraft") != std::string::npos || title.find("vimeworld") != std::string::npos);
}


void AutoclickerThread() {
    bool spaceHeld = false;

    struct KeyState {
        WORD vk;
        bool held;
    };

    std::vector<KeyState> movementKeys = {
        {0x57, false}, // W
        {0x41, false}, // A
        {0x53, false}, // S
        {0x44, false}  // D
    };

    while (autoclicker_running) {
        bool mcActive = IsMinecraftOrVimeActive();

        if ((GetAsyncKeyState(BIND_CLICK) & 0x8000) && mcActive) {
            SimulateClick();
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

    }
}

void ConsoleThread() {
    AllocConsole();
    SetConsoleOutputCP(CP_UTF8);

    FILE* fin;
    freopen_s(&fin, "CONIN$", "r", stdin);
    FILE* fout;
    freopen_s(&fout, "CONOUT$", "w", stdout);

    std::cout << "AutoClicker (injectable)\n";
    std::cout << "by crashteam\n";
    std::cout << "R = AUTOCLICKER\n";
    std::cout << "Type 0 and press Enter to exit.\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;
        if (input == "0") {
            std::cout << "Exiting...\n";
            autoclicker_running = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            FreeConsole();
            break;
        }
    }
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        std::thread(AutoclickerThread).detach();
        std::thread(ConsoleThread).detach();
        break;
    case DLL_PROCESS_DETACH:
        autoclicker_running = false;
        break;
    }
    return TRUE;
}
