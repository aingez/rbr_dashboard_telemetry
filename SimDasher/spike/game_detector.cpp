// g++ spike/game_detector.cpp -o bin/game_detector_test.exe -lws2_32
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <optional>

std::optional<std::wstring> GetRunningGameName(const std::vector<std::wstring>& gameNames) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return std::nullopt;

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &processEntry)) {
        do {
            for (const std::wstring& name : gameNames) {
                if (name == processEntry.szExeFile) {
                    CloseHandle(hSnapshot);
                    return processEntry.szExeFile;
                }
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return std::nullopt;
}

std::vector<std::wstring> gameList = {
    L"acs.exe",      // AC
    L"RichardBurnsRally_SSE.exe", // RBR
    L"dirtrally2.exe",   // DR 2.0
};

int main() {

    while (true) {
        auto result = GetRunningGameName (gameList);
        if (result) {
            std::wcout << L"Found running game: " << *result << L"\n";
        } else {
            std::wcout << L"No specified game is running.\n";
        }
        system("pause"); // Keep console open
    }
    return 0;
}
