#include <vector>
#include <string>
#include <winsock2.h>
#include "config.h"

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