#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <map>
#include <cmath>    // for std::round
#include <cstdint>  // for uint8_t
#include <cstring>  // for std::memcpy
#include <thread>       // For std::thread
#include <mutex>        // For std::mutex
#include <condition_variable>  // For thread synchronization (optional)

#pragma comment(lib, "ws2_32.lib")

#define UDP_PORT 6776
#define SERIAL_BAUDRATE 250000
#define PACKET_BUFFER_SIZE 1024
#define GEAR_OFFSET 44

std::map<int, char> gear_map = {
    {-1, 'R'}, {0, 'N'}, {1, '1'}, {2, '2'}, {3, '3'},
    {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}
};

std::mutex dataMutex;          // Mutex to protect shared data
char gearChar = 'N';           // Shared gear data
int engineRPM = 0;             // Shared RPM data
bool dataReady = false;        // Flag to indicate when new data is available

int extractIntBuffer(const char* buffer, uint16_t offset);
char ParseGearFromPacket(const char* buffer);
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

        // Parse data (gear and RPM)
        char newGearChar = ParseGearFromPacket(buffer);
        int newEngineRPM = extractIntBuffer(buffer, 136);

        // Lock the mutex and update the shared data
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            gearChar = newGearChar;
            engineRPM = newEngineRPM;
            dataReady = true;  // Data is ready to be sent
        }

        // Optional: Log data (for debugging)
        // std::cout << "Received Gear: " << gearChar << " RPM: " << engineRPM << std::endl;
    }
}

void sendToArduino(HANDLE serial) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (dataReady) {
                // Prepare data to send
                char sendBuffer[1 + sizeof(int)];
                sendBuffer[0] = gearChar;
                std::memcpy(sendBuffer + 1, &engineRPM, sizeof(engineRPM));

                // Write data to serial
                DWORD bytesWritten = 0;
                BOOL success = WriteFile(serial, sendBuffer, sizeof(sendBuffer), &bytesWritten, NULL);

                if (!success) {
                    std::cerr << "Failed to write to serial port.\n";
                } else if (bytesWritten != sizeof(sendBuffer)) {
                    std::cerr << "Warning: incomplete write to serial port.\n";
                }

                dataReady = false;  // Data has been sent, reset flag
            }
        }

        // Optional: Sleep to prevent busy-waiting (reduce CPU usage)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::vector<std::string> getAvailableComPorts() {
    std::vector<std::string> ports;
    char targetPath[1024];

    for (int i = 1; i <= 256; ++i) {
        std::string portName = "COM" + std::to_string(i);
        if (QueryDosDeviceA(portName.c_str(), targetPath, sizeof(targetPath))) {
            ports.push_back(portName);
        }
    }

    return ports;
}

bool InitWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

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

HANDLE InitSerialPort(const std::string& portName) {
    HANDLE hSerial = CreateFileA(portName.c_str(),
                                 GENERIC_WRITE,
                                 0, 0, OPEN_EXISTING,
                                 0, 0);

    if (hSerial == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) return INVALID_HANDLE_VALUE;

    dcbSerialParams.BaudRate = SERIAL_BAUDRATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) return INVALID_HANDLE_VALUE;

    return hSerial;
}

int extractIntBuffer (const char* buffer, uint16_t offset) {
    float value;
    std::memcpy(&value, buffer + offset, sizeof(float));
    return static_cast<int>(std::round(value));
}

char ParseGearFromPacket(const char* buffer) {
    int gearRaw = *(int*)(buffer + GEAR_OFFSET);
    int gear = gearRaw - 1;
    return gear_map.count(gear) ? gear_map[gear] : 'N';
}

void Cleanup(SOCKET sock, HANDLE serial) {
    if (serial != INVALID_HANDLE_VALUE) CloseHandle(serial);
    if (sock != INVALID_SOCKET) closesocket(sock);
    WSACleanup();
}

int main() {
    if (!InitWinsock()) {
        std::cerr << "Failed to initialize Winsock.\n";
        return 1;
    }
    std::cout << "Initialized Winsock\n";

    SOCKET sock = CreateUdpSocket();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket.\n";
        WSACleanup();
        return 1;
    }
    std::cout << "Created UDP Socket\n";

    std::vector<std::string> availablePorts = getAvailableComPorts();
    HANDLE serial = INVALID_HANDLE_VALUE;

    if (availablePorts.empty()) {
        std::cout << "No COM ports found. Please connect a device" << std::endl;
        return 0;
    } else {
        std::cout << "Available COM ports:" << std::endl;
        for (const std::string& port : availablePorts) {
            std::cout << port << std::endl;
        }

        std::cout << "\nPlease select a COM port from the list above: ";
        std::string selectedPort;
        std::cin >> selectedPort;

        bool valid = false;
        for (const std::string& port : availablePorts) {
            if (port == selectedPort) {
                valid = true;
                break;
            }
        }

        if (!valid) {
            std::cout << "Invalid selection. Exiting." << std::endl;
            return 0;
        }

        serial = InitSerialPort(selectedPort);
        if (serial == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open serial port.\n";
            Cleanup(sock, serial);
            return 1;
        }

        std::cout << "Opened Serial Port " << selectedPort << " successfully\n";
    }

    // Start the UDP and serial threads
    std::thread udpThread(receiveUdpPackets, sock);
    std::thread serialThread(sendToArduino, serial);

    // Join the threads to wait for them to finish
    udpThread.join();
    serialThread.join();

    Cleanup(sock, serial);
    return 0;
}
