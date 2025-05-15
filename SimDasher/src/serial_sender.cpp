#include "serial_sender.h"
#include "globals.h"
#include "config.h"

#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex>
#include <cstring>


void sendToArduino(HANDLE serial) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            if (dataReady) {
                // Prepare 5-byte buffer: [0] = gearChar, [1-4] = engineRPM
                char sendBuffer[1 + sizeof(int)];
                sendBuffer[0] = gearChar;
                std::memcpy(sendBuffer + 1, &engineRPM, sizeof(engineRPM));

                DWORD bytesWritten = 0;
                BOOL success = WriteFile(serial, sendBuffer, sizeof(sendBuffer), &bytesWritten, NULL);

                if (!success) {
                    std::cerr << "Failed to write to serial port." << std::endl;
                } else if (bytesWritten != sizeof(sendBuffer)) {
                    std::cerr << "Warning: incomplete write to serial port." << std::endl;
                }

                std::cout << "Gear :" << gearChar << "RPM :" << engineRPM << std::endl;

                dataReady = false;
            }
        }

        // Avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}