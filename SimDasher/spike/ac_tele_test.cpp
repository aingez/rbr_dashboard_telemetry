// g++ spike/ac_tele_test.cpp -o bin/ac_tele_test.exe -lws2_32

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <ws2tcpip.h>
#include <cstring>
#include <iomanip>
#include <cmath>

#pragma comment(lib, "ws2_32.lib")

#define AC_SERVER_PORT 9996
#define AC_SERVER_IP "127.0.0.1"

struct handshaker {
    int identifier;
    int version;
    int operationId;
};

struct handshackerResponse {
    char carName[50];
    char driverName[50];
    int identifier;
    int version;
    char trackName[50];
    char trackConfig[50];
};

void printFormattedLapTime(int ms) {
    int minutes = ms / 60000;
    int seconds = (ms % 60000) / 1000;
    int milliseconds = ms % 1000;

    std::cout << std::setfill('0')
              << minutes << ":"
              << std::setw(2) << seconds << "."
              << std::setw(3) << milliseconds;
}

struct RTCarInfo {
    char identifier;
    int size;
    float speed_Kmh;
    float speed_Mph;
    float speed_Ms;
    bool isAbsEnabled;
    bool isAbsInAction;
    bool isTcInAction;
    bool isTcEnabled;
    bool isInPit;
    bool isEngineLimiterOn;
    float accG_vertical;
    float accG_horizontal;
    float accG_frontal;
    int lapTime;
    int lastLap;
    int bestLap;
    int lapCount;
    float gas;
    float brake;
    float clutch;
    float engineRPM;
    float steer;
    int gear;
    float cgHeight;
    float wheelAngularSpeed[4];
    float slipAngle[4];
    float slipAngle_ContactPatch[4];
    float slipRatio[4];
    float tyreSlip[4];
    float ndSlip[4];
    float load[4];
    float Dy[4];
    float Mz[4];
    float tyreDirtyLevel[4];
    float camberRAD[4];
    float tyreRadius[4];
    float tyreLoadedRadius[4];
    float suspensionHeight[4];
    float carPositionNormalized;
    float carSlope;
    float carCoordinates[3];
};

int main() {
    WSADATA wsa;
    SOCKET s;
    sockaddr_in server;
    char buffer[2048];

    std::cout << "Initializing Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        std::cerr << "Failed. Error Code: " << WSAGetLastError() << "\n";
        return 1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(AC_SERVER_PORT);
    inet_pton(AF_INET, AC_SERVER_IP, &server.sin_addr);

    // Step 1: Send handshake
    handshaker hs;
    hs.identifier = 1;
    hs.version = 1;
    hs.operationId = 0; // HANDSHAKE

    sendto(s, reinterpret_cast<const char*>(&hs), sizeof(hs), 0,
           reinterpret_cast<sockaddr*>(&server), sizeof(server));
    std::cout << "Handshake sent.\n";

    // Step 2: Receive handshake response
    int server_len = sizeof(server);
    int recv_len = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&server), &server_len);
    if (recv_len >= sizeof(handshackerResponse)) {
        handshackerResponse* resp = reinterpret_cast<handshackerResponse*>(buffer);

        std::string driver(resp->driverName, 50);
        std::string car(resp->carName, 50);
        std::string track(resp->trackName, 50);
        std::string config(resp->trackConfig, 50);

        std::cout << "Connected to AC Server:\n";
        std::cout << "Driver: " << driver << "\n";
        std::cout << "Car: " << car << "\n";
        std::cout << "Track: " << track << " (" << config << ")\n";
    } else {
        std::cerr << "Failed to receive handshake response.\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // Step 3: Subscribe to updates
    hs.operationId = 1; // SUBSCRIBE_UPDATE
    sendto(s, reinterpret_cast<const char*>(&hs), sizeof(hs), 0,
           reinterpret_cast<sockaddr*>(&server), sizeof(server));
    std::cout << "Subscribed for telemetry updates.\n";

    // Step 4: Receive telemetry updates
    while (true) {
        recv_len = recvfrom(s, buffer, sizeof(buffer), 0,
                            reinterpret_cast<sockaddr*>(&server), &server_len);
        if (recv_len >= sizeof(RTCarInfo)) {
            RTCarInfo* telemetry = reinterpret_cast<RTCarInfo*>(buffer);

            std::cout 
            << telemetry->gear - 1
            << " | " << telemetry->engineRPM
            << " | " << static_cast<int>(round(telemetry->speed_Kmh))
            << " | ";
            
            printFormattedLapTime(telemetry->lapTime);

            std::cout << "\n";
        } else {
            std::cerr << "Incomplete telemetry packet received.\n";
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
