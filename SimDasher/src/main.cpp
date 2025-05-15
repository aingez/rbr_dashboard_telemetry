#include "udp_receiver.h"
#include "serial_sender.h"
#include "serial_port.h"
#include "com_ports.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

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
