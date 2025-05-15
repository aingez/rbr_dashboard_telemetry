#include "udp_receiver.h"
#include "globals.h"
#include "config.h"

#include <iostream>
#include <cmath>      // for std::round
#include <cstring>    // for std::memcpy
#include <map>
#include <mutex>
#include <winsock2.h>

#define GEAR_OFFSET 44

// Local gear mapping table
static std::map<int, char> gear_map = {
    {-1, 'R'}, {0, 'N'}, {1, '1'}, {2, '2'}, {3, '3'},
    {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}
};

// Initialize Winsock
bool InitWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

// Create and bind UDP socket
SOCKET CreateUdpSocket() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(UDP_PORT);

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

// Extracts a float as RPM and converts it to int
int extractIntBuffer(const char* buffer, uint16_t offset) {
    float value;
    std::memcpy(&value, buffer + offset, sizeof(float));
    return static_cast<int>(std::round(value));
}

// Parses gear from specific packet offset
char ParseGearFromPacket(const char* buffer) {
    int gearRaw = *(int*)(buffer + GEAR_OFFSET);
    int gear = gearRaw - 1;
    return gear_map.count(gear) ? gear_map[gear] : 'N';
}

// Main UDP receiving loop
void receiveUdpPackets(SOCKET sock) {
    char buffer[PACKET_BUFFER_SIZE];
    sockaddr_in clientAddr{};
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        int bytesReceived = recvfrom(sock, buffer, sizeof(buffer), 0,
                                     (sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed.\n";
            continue;
        }

        // Parse gear and RPM
        char newGearChar = ParseGearFromPacket(buffer);
        int newEngineRPM = extractIntBuffer(buffer, 136);

        // Update shared global data
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            gearChar = newGearChar;
            engineRPM = newEngineRPM;
            dataReady = true;
        }
    }
}
