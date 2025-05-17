#include "com_ports.h"
#include <iostream>

// Declare your InitSerialPort and Cleanup if they're in other files
extern HANDLE InitSerialPort(const std::string& port);
extern void Cleanup(SOCKET sock, HANDLE serial); // Or adjust if you're not using SOCKET

HANDLE SelectAndOpenSerialPort() {
    std::vector<std::string> availablePorts = getAvailableComPorts();
    if (availablePorts.empty()) {
        std::cout << "No COM ports found. Please connect a device" << std::endl;
        return INVALID_HANDLE_VALUE;
    }

    std::cout << "Available COM ports:\n";
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
        return INVALID_HANDLE_VALUE;
    }

    HANDLE serial = InitSerialPort(selectedPort);
    if (serial == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open serial port.\n";
    } else {
        std::cout << "Opened Serial Port " << selectedPort << " successfully\n";
    }

    return serial;
}