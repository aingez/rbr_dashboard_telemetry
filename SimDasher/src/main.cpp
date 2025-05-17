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
    // init windsock
    if (!InitWinsock()) {
        std::cerr << "Failed to initialize Winsock.\n";
        return 1;
    }
    std::cout << "Initialized Winsock\n";

    // init UDP Socket
    SOCKET sock = CreateUdpSocket();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket.\n";
        WSACleanup();
        return 1;
    }
    std::cout << "Created UDP Socket\n";
    
    // get COM port
    HANDLE serial = SelectAndOpenSerialPort();
    if (serial == INVALID_HANDLE_VALUE) {
        Cleanup(sock, serial);
        return 1;
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
